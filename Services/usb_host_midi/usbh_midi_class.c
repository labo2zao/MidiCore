#include "Services/usb_host_midi/usbh_midi_class.h"

#if !defined(USBH_MAX_NUM_ENDPOINTS)
// If the USBH stack isn't present, don't compile this file.
#else

#include "usbh_ctlreq.h"
#include <string.h>

// ---- MIDI Streaming class: 0x01 / 0x03 (Audio / MIDI Streaming) ----

typedef struct {
  uint8_t  in_ep;
  uint8_t  out_ep;
  uint16_t in_ep_size;
  uint16_t out_ep_size;
  uint8_t  in_pipe;
  uint8_t  out_pipe;
  uint8_t  rx_buf[64];
  uint8_t  tx_buf[64];
  uint8_t  tx_len;
  uint8_t  tx_busy;
} USBH_MIDI_HandleTypeDef;

static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef USBH_MIDI_Class = {
  "MIDI",
  0x01, // class code is Audio
  USBH_MIDI_InterfaceInit,
  USBH_MIDI_InterfaceDeInit,
  USBH_MIDI_ClassRequest,
  USBH_MIDI_Process,
  USBH_MIDI_SOFProcess,
  NULL,
};

static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost){
  USBH_MIDI_HandleTypeDef *h = (USBH_MIDI_HandleTypeDef*)USBH_malloc(sizeof(USBH_MIDI_HandleTypeDef));
  if (!h) return USBH_FAIL;
  memset(h, 0, sizeof(*h));
  phost->pActiveClass->pData = h;

  // Find MIDI streaming interface (Audio class, subclass 0x03)
  USBH_InterfaceDescTypeDef *if_desc = NULL;
  for (uint8_t i=0;i<phost->device.CfgDesc.bNumInterfaces;i++){
    USBH_InterfaceDescTypeDef *it = &phost->device.CfgDesc.Itf_Desc[i];
    if (it->bInterfaceClass == 0x01 && it->bInterfaceSubClass == 0x03) {
      if_desc = it;
      break;
    }
  }
  if (!if_desc) return USBH_FAIL;

  // Select alternate 0
  phost->pActiveClass->pData = h;

  // Parse endpoints from interface
  for (uint8_t e=0; e<if_desc->bNumEndpoints; e++){
    USBH_EpDescTypeDef *ep = &if_desc->Ep_Desc[e];
    if ((ep->bEndpointAddress & 0x80) == 0x80){
      h->in_ep = ep->bEndpointAddress;
      h->in_ep_size = ep->wMaxPacketSize;
    } else {
      h->out_ep = ep->bEndpointAddress;
      h->out_ep_size = ep->wMaxPacketSize;
    }
  }

  if (!h->in_ep || !h->out_ep) return USBH_FAIL;

  h->in_pipe  = USBH_AllocPipe(phost, h->in_ep);
  h->out_pipe = USBH_AllocPipe(phost, h->out_ep);

  USBH_OpenPipe(phost, h->in_pipe,  h->in_ep,  phost->device.address, phost->device.speed, USB_EP_TYPE_BULK, h->in_ep_size);
  USBH_OpenPipe(phost, h->out_pipe, h->out_ep, phost->device.address, phost->device.speed, USB_EP_TYPE_BULK, h->out_ep_size);

  USBH_LL_SetToggle(phost, h->in_pipe, 0);
  USBH_LL_SetToggle(phost, h->out_pipe, 0);

  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost){
  USBH_MIDI_HandleTypeDef *h = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;
  if (h){
    if (h->in_pipe)  { USBH_ClosePipe(phost, h->in_pipe);  USBH_FreePipe(phost, h->in_pipe); }
    if (h->out_pipe) { USBH_ClosePipe(phost, h->out_pipe); USBH_FreePipe(phost, h->out_pipe); }
    USBH_free(h);
    phost->pActiveClass->pData = NULL;
  }
  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost){
  // No specific requests required
  (void)phost;
  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost){
  (void)phost;
  return USBH_OK;
}

static void decode_event_packet(const uint8_t* p){
  // USB-MIDI event packet: [CIN|Cable, MIDI0, MIDI1, MIDI2]
  uint8_t cin = p[0] & 0x0F;
  (void)cin;
  uint8_t b0 = p[1];
  uint8_t b1 = p[2];
  uint8_t b2 = p[3];

  uint8_t len = 3;
  if ((b0 & 0xF0) == 0xC0 || (b0 & 0xF0) == 0xD0) len = 2;
  if ((b0 & 0xF0) == 0xF0) {
    // Keep simple: ignore sysex streaming here
    if (b0 == 0xF6 || b0 == 0xF8 || b0 == 0xFA || b0 == 0xFB || b0 == 0xFC || b0 == 0xFE || b0 == 0xFF) len = 1;
  }
  USBH_MIDI_OnShortEvent(b0,b1,b2,len);
}

USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost){
  USBH_MIDI_HandleTypeDef *h = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;
  if (!h) return USBH_FAIL;

  // RX: submit bulk in if idle
  if (USBH_LL_GetURBState(phost, h->in_pipe) == USBH_URB_DONE) {
    uint32_t n = USBH_LL_GetLastXferSize(phost, h->in_pipe);
    for (uint32_t i=0; i+3 < n; i+=4) decode_event_packet(&h->rx_buf[i]);
  }
  // Always keep IN transfer running
  (void)USBH_BulkReceiveData(phost, h->rx_buf, sizeof(h->rx_buf), h->in_pipe);

  // TX: if busy, check URB
  if (h->tx_busy) {
    USBH_URBStateTypeDef st = USBH_LL_GetURBState(phost, h->out_pipe);
    if (st == USBH_URB_DONE || st == USBH_URB_NOTREADY) {
      h->tx_busy = 0;
      h->tx_len = 0;
    }
  }

  return USBH_OK;
}

static uint8_t cin_from_status(uint8_t b0, uint8_t len){
  uint8_t st = b0 & 0xF0;
  if (st >= 0x80 && st <= 0xE0) {
    switch (st) {
      case 0x80: return 0x08;
      case 0x90: return 0x09;
      case 0xA0: return 0x0A;
      case 0xB0: return 0x0B;
      case 0xC0: return 0x0C;
      case 0xD0: return 0x0D;
      case 0xE0: return 0x0E;
    }
  }
  if (b0 == 0xF1 || b0 == 0xF3) return 0x02;
  if (b0 == 0xF2) return 0x03;
  if (len == 1) return 0x0F;
  return 0x09;
}

int USBH_MIDI_SendShort(USBH_HandleTypeDef *phost, uint8_t b0,uint8_t b1,uint8_t b2,uint8_t len){
  USBH_MIDI_HandleTypeDef *h = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;
  if (!h) return -1;
  if (h->tx_busy) return -2;
  if (len < 1 || len > 3) return -3;

  uint8_t cin = cin_from_status(b0, len);
  h->tx_buf[0] = (uint8_t)(cin & 0x0F);
  h->tx_buf[1] = b0;
  h->tx_buf[2] = (len>1)?b1:0;
  h->tx_buf[3] = (len>2)?b2:0;
  h->tx_len = 4;
  h->tx_busy = 1;
  (void)USBH_BulkSendData(phost, h->tx_buf, h->tx_len, h->out_pipe, 1);
  return 0;
}

int USBH_MIDI_SendBytes(USBH_HandleTypeDef *phost, const uint8_t* data, uint16_t len){
  // Minimal: chunk as 3-byte messages not supported for Sysex here.
  // Provide a placeholder so upper layers can compile.
  (void)phost; (void)data; (void)len;
  return -4;
}

#endif // USBH_MAX_NUM_ENDPOINTS

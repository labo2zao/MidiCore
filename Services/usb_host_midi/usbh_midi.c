/**
 * usbh_midi.c
 * Minimal USB Host class for USB MIDI (Audio class, MIDIStreaming subclass)
 *
 * Works with STM32 USB Host Library (CubeMX) using USB_OTG_FS on STM32F407.
 * This is a "compile-safe" implementation: enumeration + endpoint parsing + bulk IN/OUT.
 *
 * NOTE: Real-world MIDI devices vary; some are composite and have multiple interfaces.
 */

#include "Services/usb_host_midi/usbh_midi.h"
#include "Services/usb_host_midi/usbh_midi_class.h"

#include "usbh_core.h"
#include "usbh_ctlreq.h"
#include "usbh_def.h"
#include "usbh_ioreq.h"

#include <string.h>
#include <stdint.h>

/* ---- USB Audio/MIDI constants (do NOT use an enum here) ---- */
#ifndef USB_AUDIO_CLASS
#define USB_AUDIO_CLASS 0x01u
#endif

#ifndef USB_AUDIO_MIDISTREAMING_SUBCLASS
#define USB_AUDIO_MIDISTREAMING_SUBCLASS 0x03u
#endif

/* Endpoint types */
#ifndef USB_EP_TYPE_BULK
#define USB_EP_TYPE_BULK 0x02u
#endif

/* ---- Tunables ---- */
#ifndef USBH_MIDI_RX_BUF_SZ
#define USBH_MIDI_RX_BUF_SZ 128u
#endif

#ifndef USBH_MIDI_TX_BUF_SZ
#define USBH_MIDI_TX_BUF_SZ 128u
#endif

/* ---- Internal handle ---- */
typedef struct {
  uint8_t  if_num;
  uint8_t  is_ready;

  uint8_t  in_ep;
  uint16_t in_ep_size;
  uint8_t  in_pipe;

  uint8_t  out_ep;
  uint16_t out_ep_size;
  uint8_t  out_pipe;

  uint8_t  rx_buf[USBH_MIDI_RX_BUF_SZ];
  uint8_t  tx_buf[USBH_MIDI_TX_BUF_SZ];

  uint32_t rx_len;
} USBH_MIDI_HandleTypeDef;

/* Forward declarations required by USBH_ClassTypeDef */
static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_Process_Internal(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost);

/* Exposed class instance */
USBH_ClassTypeDef  USBH_MIDI_Class = {
  "MIDI",
  USB_AUDIO_CLASS,
  USBH_MIDI_InterfaceInit,
  USBH_MIDI_InterfaceDeInit,
  USBH_MIDI_ClassRequest,
  USBH_MIDI_Process_Internal,
  USBH_MIDI_SOFProcess,
  NULL,
};

/* ---- Helper: parse endpoints in selected interface ---- */
static void parse_endpoints(USBH_HandleTypeDef *phost, USBH_MIDI_HandleTypeDef *hh)
{
  hh->in_ep = 0;
  hh->out_ep = 0;
  hh->in_ep_size = 0;
  hh->out_ep_size = 0;

  /* After USBH_SelectInterface(), the descriptor is available in phost->device.CfgDesc */
  USBH_InterfaceDescTypeDef *ifd = &phost->device.CfgDesc.Itf_Desc[hh->if_num];

  /* Cube host stack stores endpoints in ifd->Ep_Desc[] */
  for (uint8_t i = 0; i < ifd->bNumEndpoints; i++) {
    USBH_EpDescTypeDef *ep = &ifd->Ep_Desc[i];

    uint8_t ep_addr = ep->bEndpointAddress;
    uint8_t ep_attr = ep->bmAttributes & 0x03u;

    if (ep_attr != USB_EP_TYPE_BULK) continue;

    if ((ep_addr & 0x80u) != 0u) { /* IN */
      if (hh->in_ep == 0u) {
        hh->in_ep = ep_addr;
        hh->in_ep_size = ep->wMaxPacketSize;
      }
    } else { /* OUT */
      if (hh->out_ep == 0u) {
        hh->out_ep = ep_addr;
        hh->out_ep_size = ep->wMaxPacketSize;
      }
    }
  }
}

/* ---- Class callbacks ---- */

static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_OK;
  USBH_MIDI_HandleTypeDef *hh;

  /* Allocate class data */
  hh = (USBH_MIDI_HandleTypeDef*)USBH_malloc(sizeof(USBH_MIDI_HandleTypeDef));
  if (hh == NULL) {
    return USBH_FAIL;
  }
  memset(hh, 0, sizeof(*hh));
  phost->pActiveClass->pData = hh;

  /* Find an interface that matches Audio/MIDIStreaming */
  uint8_t if_num = USBH_FindInterface(phost,
                                      USB_AUDIO_CLASS,
                                      USB_AUDIO_MIDISTREAMING_SUBCLASS,
                                      0xFFu /* any protocol */);

  if (if_num == 0xFFu) {
    return USBH_FAIL;
  }

  hh->if_num = if_num;

  /* Select interface */
  status = USBH_SelectInterface(phost, hh->if_num);
  if (status != USBH_OK) {
    return status;
  }

  /* Parse endpoints */
  parse_endpoints(phost, hh);

  if (hh->in_ep == 0u || hh->out_ep == 0u) {
    /* Some devices may be OUT-only or IN-only; for our use we expect both */
    return USBH_FAIL;
  }

  /* Allocate pipes */
  hh->in_pipe  = USBH_AllocPipe(phost, hh->in_ep);
  hh->out_pipe = USBH_AllocPipe(phost, hh->out_ep);

  /* Open pipes */
  USBH_OpenPipe(phost, hh->in_pipe,  hh->in_ep,  phost->device.address,
                phost->device.speed, USB_EP_TYPE_BULK, hh->in_ep_size);

  USBH_OpenPipe(phost, hh->out_pipe, hh->out_ep, phost->device.address,
                phost->device.speed, USB_EP_TYPE_BULK, hh->out_ep_size);

  USBH_LL_SetToggle(phost, hh->in_pipe,  0u);
  USBH_LL_SetToggle(phost, hh->out_pipe, 0u);

  /* Start first RX */
  hh->rx_len = 0;
  (void)USBH_BulkReceiveData(phost, hh->rx_buf, USBH_MIDI_RX_BUF_SZ, hh->in_pipe);

  hh->is_ready = 1u;
  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  USBH_MIDI_HandleTypeDef *hh = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;
  if (hh != NULL) {
    if (hh->in_pipe != 0u) {
      USBH_ClosePipe(phost, hh->in_pipe);
      USBH_FreePipe(phost, hh->in_pipe);
      hh->in_pipe = 0u;
    }
    if (hh->out_pipe != 0u) {
      USBH_ClosePipe(phost, hh->out_pipe);
      USBH_FreePipe(phost, hh->out_pipe);
      hh->out_pipe = 0u;
    }
    USBH_free(hh);
    phost->pActiveClass->pData = NULL;
  }
  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost)
{
  /* MIDIStreaming generally needs no class-specific control request for basic operation */
  (void)phost;
  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MIDI_Process_Internal(USBH_HandleTypeDef *phost)
{
  USBH_MIDI_HandleTypeDef *hh = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;
  if (hh == NULL || hh->is_ready == 0u) return USBH_FAIL;

  /* Keep RX running; user can read frames via USBH_MIDI_Read() */
  USBH_URBStateTypeDef urb = USBH_LL_GetURBState(phost, hh->in_pipe);

  if (urb == USBH_URB_DONE) {
    /* how many bytes received */
    hh->rx_len = USBH_LL_GetLastXferSize(phost, hh->in_pipe);

    /* immediately restart RX */
    (void)USBH_BulkReceiveData(phost, hh->rx_buf, USBH_MIDI_RX_BUF_SZ, hh->in_pipe);
  } else if (urb == USBH_URB_ERROR || urb == USBH_URB_STALL) {
    /* try to recover */
    (void)USBH_BulkReceiveData(phost, hh->rx_buf, USBH_MIDI_RX_BUF_SZ, hh->in_pipe);
  }

  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost)
{
  (void)phost;
  return USBH_OK;
}

/* ---- Public API (thin wrappers) ---- */

USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost)
{
  return USBH_MIDI_Process_Internal(phost);
}

int USBH_MIDI_IsReady(USBH_HandleTypeDef *phost)
{
  if (phost == NULL || phost->pActiveClass == NULL) return 0;
  USBH_MIDI_HandleTypeDef *hh = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;
  return (hh != NULL && hh->is_ready) ? 1 : 0;
}

/**
 * Read received raw USB-MIDI event packets (4-byte USB MIDI packets typically).
 * Returns number of bytes copied into out (0 if none).
 */
int USBH_MIDI_Read(USBH_HandleTypeDef *phost, uint8_t *out, uint16_t max_len)
{
  if (!USBH_MIDI_IsReady(phost) || out == NULL || max_len == 0) return 0;

  USBH_MIDI_HandleTypeDef *hh = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;
  uint32_t n = hh->rx_len;
  if (n == 0u) return 0;

  if (n > max_len) n = max_len;
  memcpy(out, hh->rx_buf, n);

  /* mark consumed */
  hh->rx_len = 0;
  return (int)n;
}

/**
 * Send bytes via bulk OUT. The data is expected to be already formatted as USB-MIDI packets
 * (usually 4 bytes per event). For simple "3-byte MIDI", use USBH_MIDI_SendShort().
 */
int USBH_MIDI_SendBytes(USBH_HandleTypeDef *phost, const uint8_t *data, uint16_t len)
{
  if (!USBH_MIDI_IsReady(phost) || data == NULL || len == 0) return -1;

  USBH_MIDI_HandleTypeDef *hh = (USBH_MIDI_HandleTypeDef*)phost->pActiveClass->pData;

  /* ST signature is: USBH_BulkSendData(phost, buff, length, pipe, do_ping) */
  if (USBH_BulkSendData(phost, (uint8_t*)data, len, hh->out_pipe, 0u) != USBH_OK) {
    return -2;
  }
  return (int)len;
}

/**
 * USBH_MIDI_Recv - Wrapper for USBH_MIDI_Read to match expected API signature
 * Returns 0 on success, writes number of bytes received to *out_used
 */
int USBH_MIDI_Recv(USBH_HandleTypeDef *phost, uint8_t *out, uint16_t out_len, uint16_t *out_used)
{
  if (out_used == NULL) return -1;
  
  int bytes = USBH_MIDI_Read(phost, out, out_len);
  if (bytes < 0) {
    *out_used = 0;
    return -1;
  }
  
  *out_used = (uint16_t)bytes;
  return 0;  // 0 = success
}

/**
 * USBH_MIDI_Send - Wrapper for USBH_MIDI_SendBytes to match expected API signature
 * Returns 0 on success
 */
int USBH_MIDI_Send(USBH_HandleTypeDef *phost, const uint8_t *data, uint16_t len)
{
  int result = USBH_MIDI_SendBytes(phost, data, len);
  return (result > 0) ? 0 : result;  // 0 = success, negative = error
}

/**
 * Convenience for short MIDI messages (len=1..3).
 * This wraps into 4-byte USB MIDI event packet.
 *
 * WARNING: This is a minimal packer. For SysEx, use USBH_MIDI_SendBytes() with proper packets.
 */
int USBH_MIDI_SendShort(USBH_HandleTypeDef *phost, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t len)
{
  if (len < 1u) return -1;
  if (len > 3u) len = 3u;

  uint8_t cin = 0x0u;
  uint8_t st  = b0 & 0xF0u;

  /* Basic CIN mapping for Channel Voice */
  if (st == 0x80u) cin = 0x8u;      /* Note Off */
  else if (st == 0x90u) cin = 0x9u; /* Note On */
  else if (st == 0xA0u) cin = 0xAu; /* Poly Pressure */
  else if (st == 0xB0u) cin = 0xBu; /* CC */
  else if (st == 0xC0u) cin = 0xCu; /* Program Change (2 bytes) */
  else if (st == 0xD0u) cin = 0xDu; /* Channel Pressure (2 bytes) */
  else if (st == 0xE0u) cin = 0xEu; /* Pitch Bend */
  else cin = 0x0u;                 /* fallback */

  uint8_t pkt[4];
  pkt[0] = (uint8_t)((0u << 4) | (cin & 0x0Fu)); /* cable=0 */
  pkt[1] = b0;
  pkt[2] = (len >= 2u) ? b1 : 0u;
  pkt[3] = (len >= 3u) ? b2 : 0u;

  return USBH_MIDI_SendBytes(phost, pkt, 4u);
}

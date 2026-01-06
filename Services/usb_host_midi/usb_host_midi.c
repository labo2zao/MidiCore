#include "usb_host_midi.h"
#include <string.h>

#include "Config/project_config.h"
#include "Services/router/router.h"

#if defined(__has_include)
#  if __has_include("usbh_core.h")
#    define USBH_PRESENT 1
#  else
#    define USBH_PRESENT 0
#  endif
#else
#  define USBH_PRESENT 0
#endif

#if USBH_PRESENT && defined(ENABLE_USBH_MIDI)

#include "usbh_core.h"
#include "usbh_midi.h"

// CubeMX will generate USBH_HandleTypeDef hUsbHostFS in usb_host.c.
// If you name it differently, adjust here.
extern USBH_HandleTypeDef hUsbHostFS;

#define RXQ_SZ 128
typedef struct { uint8_t s,d1,d2; } midi3_t;
static midi3_t rxq[RXQ_SZ];
static volatile uint16_t rx_head=0, rx_tail=0;

static void rx_push(uint8_t s,uint8_t d1,uint8_t d2){
  uint16_t nh=(uint16_t)((rx_head+1u)%RXQ_SZ);
  if (nh==rx_tail) return; // drop on overflow
  rxq[rx_head]=(midi3_t){s,d1,d2};
  rx_head=nh;
}

static int rx_pop(uint8_t* s,uint8_t* d1,uint8_t* d2){
  if (rx_tail==rx_head) return 1;
  midi3_t m=rxq[rx_tail];
  rx_tail=(uint16_t)((rx_tail+1u)%RXQ_SZ);
  if(s)*s=m.s; if(d1)*d1=m.d1; if(d2)*d2=m.d2;
  return 0;
}

static uint8_t cin_from_status(uint8_t st){
  uint8_t t=st & 0xF0;
  switch(t){
    case 0x80: return 0x8; // note off
    case 0x90: return 0x9; // note on
    case 0xA0: return 0xA; // poly aftertouch
    case 0xB0: return 0xB; // cc
    case 0xC0: return 0xC; // prog
    case 0xD0: return 0xD; // ch pressure
    case 0xE0: return 0xE; // pitch bend
    default: return 0x0;
  }
}

void usb_host_midi_init(void){
  // Nothing here: USBH init/start is expected to be done by CubeMX-generated usb_host.c
  // We only rely on USBH_MIDI_Class registration which is also done there (see README).
}

void usb_host_midi_task(void){
  // Let USB Host core run
  USBH_Process(&hUsbHostFS);

  // Pull raw USB-MIDI packets and decode to MIDI3
  uint8_t buf[64];
  uint16_t used=0;
  if (USBH_MIDI_Recv(&hUsbHostFS, buf, sizeof(buf), &used)==0 && used>=4){
    for (uint16_t i=0;i+3<used;i+=4){
      uint8_t cin = buf[i] & 0x0F;
      uint8_t b1=buf[i+1], b2=buf[i+2], b3=buf[i+3];
      // Handle common 3-byte messages
      if (cin==0x8 || cin==0x9 || cin==0xA || cin==0xB || cin==0xE){
        rx_push(b1,b2,b3);
        router_msg_t m={.type=ROUTER_MSG_3B,.b0=b1,.b1=b2,.b2=b3};
        router_process(ROUTER_NODE_USBH_IN,&m);
      } else if (cin==0xC || cin==0xD){
        rx_push(b1,b2,0);
        router_msg_t m={.type=ROUTER_MSG_2B,.b0=b1,.b1=b2,.b2=0};
        router_process(ROUTER_NODE_USBH_IN,&m);
      }
    }
  }
}

int usb_host_midi_send3(uint8_t status, uint8_t d1, uint8_t d2){
  uint8_t cin = cin_from_status(status);
  if (cin==0) return -1;
  uint8_t p[4] = { (uint8_t)(0x00 | (cin & 0x0F)), status, d1, d2 };
  // For 2-byte messages, d2 is ignored by receiver anyway
  return USBH_MIDI_Send(&hUsbHostFS, p, 4);
}

int usb_host_midi_recv3(uint8_t *status, uint8_t *d1, uint8_t *d2){
  return rx_pop(status,d1,d2);
}

#else

void usb_host_midi_init(void){ }
void usb_host_midi_task(void){ }
int usb_host_midi_send3(uint8_t status, uint8_t d1, uint8_t d2){ (void)status;(void)d1;(void)d2; return -1; }
int usb_host_midi_recv3(uint8_t *status, uint8_t *d1, uint8_t *d2){ (void)status;(void)d1;(void)d2; return 1; }

#endif

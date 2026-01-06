#pragma once
#include <stdint.h>

// -----------------------------------------------------------------------------
// Compile-safe include strategy
//
// When CubeMX USB Host middleware is enabled, the ST headers define:
//   - USBH_HandleTypeDef
//   - USBH_ClassTypeDef
//   - USBH_StatusTypeDef
// and expose them through "usbh_core.h" / "usbh_def.h".
//
// During early bring-up (middleware not yet enabled), those types don't exist
// and this header must still compile so the rest of the project builds.
// In that case, we provide tiny stub typedefs guarded by the ST header guards.
// -----------------------------------------------------------------------------

#if defined(__has_include)
  #if __has_include("usbh_core.h")
    #include "usbh_core.h"
  #elif __has_include("usbh_def.h")
    #include "usbh_def.h"
  #endif
#endif

// If the ST USB Host headers weren't included (middleware disabled), create
// minimal stand-ins so this module can be compiled out cleanly.
#if !defined(__USBH_CORE_H) && !defined(__USBH_DEF_H)
  typedef struct _USBH_HandleTypeDef USBH_HandleTypeDef;
  // Must be a COMPLETE type because we declare an extern instance below.
  typedef struct _USBH_ClassTypeDef {
    uint32_t _dummy;
  } USBH_ClassTypeDef;
  typedef enum {
    USBH_OK = 0,
    USBH_FAIL,
    USBH_BUSY,
    USBH_NOT_SUPPORTED,
    USBH_UNRECOVERED_ERROR
  } USBH_StatusTypeDef;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Minimal USBH MIDI class.
// Note: requires STM32 USB Host Library to be enabled in CubeMX (USB_OTG_FS Host + USB Host middleware).

// Exposed to core
extern USBH_ClassTypeDef  USBH_MIDI_Class;
#define USBH_MIDI_CLASS   &USBH_MIDI_Class

// Called periodically when class active
USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost);

// Send helpers
int USBH_MIDI_SendShort(USBH_HandleTypeDef *phost, uint8_t b0,uint8_t b1,uint8_t b2,uint8_t len);
int USBH_MIDI_SendBytes(USBH_HandleTypeDef *phost, const uint8_t* data, uint16_t len);

// Weak callback implemented in usb_host_midi.c
void USBH_MIDI_OnShortEvent(uint8_t b0,uint8_t b1,uint8_t b2,uint8_t len);

#ifdef __cplusplus
}
#endif

/**
  ******************************************************************************
  * @file    usbd_conf.h
  * @brief   USB Device configuration header
  * @author  MidiCore (MIOS32-inspired)
  ******************************************************************************
  */

#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

/*---------- -----------*/
#define USBD_MAX_NUM_INTERFACES     1
/*---------- -----------*/
#define USBD_MAX_NUM_CONFIGURATION     1
/*---------- -----------*/
#define USBD_MAX_STR_DESC_SIZ     512
/*---------- -----------*/
#define USBD_DEBUG_LEVEL     0
/*---------- -----------*/
#define USBD_LPM_ENABLED     0
/*---------- -----------*/
#define USBD_SELF_POWERED     1
/*---------- -----------*/
#define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE     2
/*---------- -----------*/
#define USBD_CUSTOM_HID_REPORT_DESC_SIZE     163
/*---------- -----------*/
#define CUSTOM_HID_FS_BINTERVAL     5

/* MIDI specific defines */
#define USBD_MIDI_DATA_IN_PACKET_SIZE     64
#define USBD_MIDI_DATA_OUT_PACKET_SIZE     64
#define MIDI_NUM_PORTS                     4    /* 4 virtual ports like MIOS32 */

/****************************************/
/* #define for FS and HS identification */
#define DEVICE_FS     0

/** @defgroup USBD_Exported_Macros
  * @{
  */

/* Memory management macros */
#define USBD_malloc         malloc
#define USBD_free           free
#define USBD_memset         memset
#define USBD_memcpy         memcpy
#define USBD_Delay          HAL_Delay
    
/* For footprint reasons and since only one allocation is handled in the MSC class 
   driver, the malloc/free is changed into a static allocation method */
void *USBD_static_malloc(uint32_t size);
void USBD_static_free(void *p);

#define MAX_STATIC_ALLOC_SIZE     300

/* DEBUG macros */
#if (USBD_DEBUG_LEVEL > 0)
#define  USBD_UsrLog(...)   printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define  USBD_ErrLog(...)   printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define  USBD_DbgLog(...)   printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)
#endif

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF__H__ */

/**
 * @file i2c_scan.c
 * @brief MIOS32-style I2C scan utility
 * 
 * MIOS32 ARCHITECTURE:
 * - NO printf/snprintf/vsnprintf (causes stack overflow)
 * - Results available via global variables for debugger
 * - Debug via MIOS Studio terminal if needed
 */

#include "App/i2c_scan.h"
#include "Hal/i2c_hal.h"

/* Maximum devices to track in results array */
#define I2C_SCAN_MAX_DEVICES 16

/* Global results for debugger visibility (no printf!) */
volatile uint8_t g_i2c_scan_bus = 0;
volatile uint8_t g_i2c_scan_found = 0;
volatile uint8_t g_i2c_scan_addrs[I2C_SCAN_MAX_DEVICES] = {0};

void app_i2c_scan_and_log(uint8_t bus){
  g_i2c_scan_bus = bus;
  g_i2c_scan_found = 0;
  
  for(uint8_t a=0x03; a<=0x77; a++){
    if(i2c_hal_probe(bus, a, 10) == 0){
      if(g_i2c_scan_found < I2C_SCAN_MAX_DEVICES){
        g_i2c_scan_addrs[g_i2c_scan_found] = a;
      }
      g_i2c_scan_found++;
    }
  }
  /* Results available in g_i2c_scan_* variables for debugger */
}

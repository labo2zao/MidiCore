#include "App/i2c_scan.h"
#include "Hal/i2c_hal.h"
#include "App/tests/test_debug.h"

void app_i2c_scan_and_log(uint8_t bus){
  dbg_printf("I2C: Scan start bus=%u\r\n", (unsigned)bus);
  int found = 0;
  for(uint8_t a=0x03; a<=0x77; a++){
    if(i2c_hal_probe(bus, a, 10) == 0){
      dbg_printf("I2C: Found addr 0x%02X\r\n", a);
      found++;
    }
  }
  if(!found){
    dbg_printf("I2C: No devices found on bus=%u\r\n", (unsigned)bus);
  } else {
    dbg_printf("I2C: Scan done bus=%u found=%d\r\n", (unsigned)bus, found);
  }
}

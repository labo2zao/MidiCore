#include "App/i2c_scan.h"
#include "Hal/i2c_hal.h"
#include "Services/log/log.h"

void app_i2c_scan_and_log(uint8_t bus){
  log_printf("I2C", "Scan start bus=%u", (unsigned)bus);
  int found = 0;
  for(uint8_t a=0x03; a<=0x77; a++){
    if(i2c_hal_probe(bus, a, 10) == 0){
      log_printf("I2C", "Found addr 0x%02X", a);
      found++;
    }
  }
  if(!found){
    log_printf("I2C", "No devices found on bus=%u", (unsigned)bus);
  } else {
    log_printf("I2C", "Scan done bus=%u found=%d", (unsigned)bus, found);
  }
}

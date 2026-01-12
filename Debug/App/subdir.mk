################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/ain_midi_task.c \
../App/ain_raw_debug_task.c \
../App/app_entry.c \
../App/app_init.c \
../App/calibration_task.c \
../App/din_selftest.c \
../App/freertos_hooks.c \
../App/i2c_scan.c \
../App/input_task.c \
../App/looper_selftest.c \
../App/midi_din_debug_task.c \
../App/midi_io_task.c \
../App/pressure_task.c 

OBJS += \
./App/ain_midi_task.o \
./App/ain_raw_debug_task.o \
./App/app_entry.o \
./App/app_init.o \
./App/calibration_task.o \
./App/din_selftest.o \
./App/freertos_hooks.o \
./App/i2c_scan.o \
./App/input_task.o \
./App/looper_selftest.o \
./App/midi_din_debug_task.o \
./App/midi_io_task.o \
./App/pressure_task.o 

C_DEPS += \
./App/ain_midi_task.d \
./App/ain_raw_debug_task.d \
./App/app_entry.d \
./App/app_init.d \
./App/calibration_task.d \
./App/din_selftest.d \
./App/freertos_hooks.d \
./App/i2c_scan.d \
./App/input_task.d \
./App/looper_selftest.d \
./App/midi_din_debug_task.d \
./App/midi_io_task.d \
./App/pressure_task.d 


# Each subdirectory must supply rules for building sources it contributes
App/%.o App/%.su App/%.cyclo: ../App/%.c App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSRIO_ENABLE -DDIN_SELFTEST -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App

clean-App:
	-$(RM) ./App/ain_midi_task.cyclo ./App/ain_midi_task.d ./App/ain_midi_task.o ./App/ain_midi_task.su ./App/ain_raw_debug_task.cyclo ./App/ain_raw_debug_task.d ./App/ain_raw_debug_task.o ./App/ain_raw_debug_task.su ./App/app_entry.cyclo ./App/app_entry.d ./App/app_entry.o ./App/app_entry.su ./App/app_init.cyclo ./App/app_init.d ./App/app_init.o ./App/app_init.su ./App/calibration_task.cyclo ./App/calibration_task.d ./App/calibration_task.o ./App/calibration_task.su ./App/din_selftest.cyclo ./App/din_selftest.d ./App/din_selftest.o ./App/din_selftest.su ./App/freertos_hooks.cyclo ./App/freertos_hooks.d ./App/freertos_hooks.o ./App/freertos_hooks.su ./App/i2c_scan.cyclo ./App/i2c_scan.d ./App/i2c_scan.o ./App/i2c_scan.su ./App/input_task.cyclo ./App/input_task.d ./App/input_task.o ./App/input_task.su ./App/looper_selftest.cyclo ./App/looper_selftest.d ./App/looper_selftest.o ./App/looper_selftest.su ./App/midi_din_debug_task.cyclo ./App/midi_din_debug_task.d ./App/midi_din_debug_task.o ./App/midi_din_debug_task.su ./App/midi_io_task.cyclo ./App/midi_io_task.d ./App/midi_io_task.o ./App/midi_io_task.su ./App/pressure_task.cyclo ./App/pressure_task.d ./App/pressure_task.o ./App/pressure_task.su

.PHONY: clean-App


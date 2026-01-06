################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hal/uart_midi/hal_uart_midi.c 

OBJS += \
./Hal/uart_midi/hal_uart_midi.o 

C_DEPS += \
./Hal/uart_midi/hal_uart_midi.d 


# Each subdirectory must supply rules for building sources it contributes
Hal/uart_midi/%.o Hal/uart_midi/%.su Hal/uart_midi/%.cyclo: ../Hal/uart_midi/%.c Hal/uart_midi/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hal-2f-uart_midi

clean-Hal-2f-uart_midi:
	-$(RM) ./Hal/uart_midi/hal_uart_midi.cyclo ./Hal/uart_midi/hal_uart_midi.d ./Hal/uart_midi/hal_uart_midi.o ./Hal/uart_midi/hal_uart_midi.su

.PHONY: clean-Hal-2f-uart_midi


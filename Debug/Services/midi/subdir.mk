################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Services/midi/midi_delayq.c \
../Services/midi/midi_din.c 

OBJS += \
./Services/midi/midi_delayq.o \
./Services/midi/midi_din.o 

C_DEPS += \
./Services/midi/midi_delayq.d \
./Services/midi/midi_din.d 


# Each subdirectory must supply rules for building sources it contributes
Services/midi/%.o Services/midi/%.su Services/midi/%.cyclo: ../Services/midi/%.c Services/midi/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Services-2f-midi

clean-Services-2f-midi:
	-$(RM) ./Services/midi/midi_delayq.cyclo ./Services/midi/midi_delayq.d ./Services/midi/midi_delayq.o ./Services/midi/midi_delayq.su ./Services/midi/midi_din.cyclo ./Services/midi/midi_din.d ./Services/midi/midi_din.o ./Services/midi/midi_din.su

.PHONY: clean-Services-2f-midi


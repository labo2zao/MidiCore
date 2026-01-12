################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Services/system/boot_reason.c \
../Services/system/panic.c \
../Services/system/safe_mode.c \
../Services/system/system_status.c 

OBJS += \
./Services/system/boot_reason.o \
./Services/system/panic.o \
./Services/system/safe_mode.o \
./Services/system/system_status.o 

C_DEPS += \
./Services/system/boot_reason.d \
./Services/system/panic.d \
./Services/system/safe_mode.d \
./Services/system/system_status.d 


# Each subdirectory must supply rules for building sources it contributes
Services/system/%.o Services/system/%.su Services/system/%.cyclo: ../Services/system/%.c Services/system/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSRIO_ENABLE -DDIN_SELFTEST -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Services-2f-system

clean-Services-2f-system:
	-$(RM) ./Services/system/boot_reason.cyclo ./Services/system/boot_reason.d ./Services/system/boot_reason.o ./Services/system/boot_reason.su ./Services/system/panic.cyclo ./Services/system/panic.d ./Services/system/panic.o ./Services/system/panic.su ./Services/system/safe_mode.cyclo ./Services/system/safe_mode.d ./Services/system/safe_mode.o ./Services/system/safe_mode.su ./Services/system/system_status.cyclo ./Services/system/system_status.d ./Services/system/system_status.o ./Services/system/system_status.su

.PHONY: clean-Services-2f-system


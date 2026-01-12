################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hal/ainser64_hw/hal_ainser64_hw_step.c 

OBJS += \
./Hal/ainser64_hw/hal_ainser64_hw_step.o 

C_DEPS += \
./Hal/ainser64_hw/hal_ainser64_hw_step.d 


# Each subdirectory must supply rules for building sources it contributes
Hal/ainser64_hw/%.o Hal/ainser64_hw/%.su Hal/ainser64_hw/%.cyclo: ../Hal/ainser64_hw/%.c Hal/ainser64_hw/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSRIO_ENABLE -DDIN_SELFTEST -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hal-2f-ainser64_hw

clean-Hal-2f-ainser64_hw:
	-$(RM) ./Hal/ainser64_hw/hal_ainser64_hw_step.cyclo ./Hal/ainser64_hw/hal_ainser64_hw_step.d ./Hal/ainser64_hw/hal_ainser64_hw_step.o ./Hal/ainser64_hw/hal_ainser64_hw_step.su

.PHONY: clean-Hal-2f-ainser64_hw


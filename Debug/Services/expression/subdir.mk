################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Services/expression/expression.c \
../Services/expression/expression_cfg.c 

OBJS += \
./Services/expression/expression.o \
./Services/expression/expression_cfg.o 

C_DEPS += \
./Services/expression/expression.d \
./Services/expression/expression_cfg.d 


# Each subdirectory must supply rules for building sources it contributes
Services/expression/%.o Services/expression/%.su Services/expression/%.cyclo: ../Services/expression/%.c Services/expression/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Services-2f-expression

clean-Services-2f-expression:
	-$(RM) ./Services/expression/expression.cyclo ./Services/expression/expression.d ./Services/expression/expression.o ./Services/expression/expression.su ./Services/expression/expression_cfg.cyclo ./Services/expression/expression_cfg.d ./Services/expression/expression_cfg.o ./Services/expression/expression_cfg.su

.PHONY: clean-Services-2f-expression


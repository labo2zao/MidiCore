################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Services/patch/patch.c \
../Services/patch/patch_adv.c \
../Services/patch/patch_bank.c \
../Services/patch/patch_manager.c \
../Services/patch/patch_router.c \
../Services/patch/patch_sd_mount.c \
../Services/patch/patch_state.c \
../Services/patch/patch_system.c 

OBJS += \
./Services/patch/patch.o \
./Services/patch/patch_adv.o \
./Services/patch/patch_bank.o \
./Services/patch/patch_manager.o \
./Services/patch/patch_router.o \
./Services/patch/patch_sd_mount.o \
./Services/patch/patch_state.o \
./Services/patch/patch_system.o 

C_DEPS += \
./Services/patch/patch.d \
./Services/patch/patch_adv.d \
./Services/patch/patch_bank.d \
./Services/patch/patch_manager.d \
./Services/patch/patch_router.d \
./Services/patch/patch_sd_mount.d \
./Services/patch/patch_state.d \
./Services/patch/patch_system.d 


# Each subdirectory must supply rules for building sources it contributes
Services/patch/%.o Services/patch/%.su Services/patch/%.cyclo: ../Services/patch/%.c Services/patch/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Services-2f-patch

clean-Services-2f-patch:
	-$(RM) ./Services/patch/patch.cyclo ./Services/patch/patch.d ./Services/patch/patch.o ./Services/patch/patch.su ./Services/patch/patch_adv.cyclo ./Services/patch/patch_adv.d ./Services/patch/patch_adv.o ./Services/patch/patch_adv.su ./Services/patch/patch_bank.cyclo ./Services/patch/patch_bank.d ./Services/patch/patch_bank.o ./Services/patch/patch_bank.su ./Services/patch/patch_manager.cyclo ./Services/patch/patch_manager.d ./Services/patch/patch_manager.o ./Services/patch/patch_manager.su ./Services/patch/patch_router.cyclo ./Services/patch/patch_router.d ./Services/patch/patch_router.o ./Services/patch/patch_router.su ./Services/patch/patch_sd_mount.cyclo ./Services/patch/patch_sd_mount.d ./Services/patch/patch_sd_mount.o ./Services/patch/patch_sd_mount.su ./Services/patch/patch_state.cyclo ./Services/patch/patch_state.d ./Services/patch/patch_state.o ./Services/patch/patch_state.su ./Services/patch/patch_system.cyclo ./Services/patch/patch_system.d ./Services/patch/patch_system.o ./Services/patch/patch_system.su

.PHONY: clean-Services-2f-patch


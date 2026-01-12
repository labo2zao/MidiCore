################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Services/ui/chord_cfg.c \
../Services/ui/ui.c \
../Services/ui/ui_actions.c \
../Services/ui/ui_actions_impl.c \
../Services/ui/ui_actions_stubs.c \
../Services/ui/ui_bindings.c \
../Services/ui/ui_encoders.c \
../Services/ui/ui_gfx.c \
../Services/ui/ui_page_looper.c \
../Services/ui/ui_page_looper_pianoroll.c \
../Services/ui/ui_page_looper_timeline.c \
../Services/ui/ui_page_song.c \
../Services/ui/ui_page_midi_monitor.c \
../Services/ui/ui_page_sysex.c \
../Services/ui/ui_page_config.c \
../Services/ui/ui_page_livefx.c \
../Services/ui/ui_state.c \
../Services/ui/ui_status.c 

OBJS += \
./Services/ui/chord_cfg.o \
./Services/ui/ui.o \
./Services/ui/ui_actions.o \
./Services/ui/ui_actions_impl.o \
./Services/ui/ui_actions_stubs.o \
./Services/ui/ui_bindings.o \
./Services/ui/ui_encoders.o \
./Services/ui/ui_gfx.o \
./Services/ui/ui_page_looper.o \
./Services/ui/ui_page_looper_pianoroll.o \
./Services/ui/ui_page_looper_timeline.o \
./Services/ui/ui_page_song.o \
./Services/ui/ui_page_midi_monitor.o \
./Services/ui/ui_page_sysex.o \
./Services/ui/ui_page_config.o \
./Services/ui/ui_page_livefx.o \
./Services/ui/ui_state.o \
./Services/ui/ui_status.o 

C_DEPS += \
./Services/ui/chord_cfg.d \
./Services/ui/ui.d \
./Services/ui/ui_actions.d \
./Services/ui/ui_actions_impl.d \
./Services/ui/ui_actions_stubs.d \
./Services/ui/ui_bindings.d \
./Services/ui/ui_encoders.d \
./Services/ui/ui_gfx.d \
./Services/ui/ui_page_looper.d \
./Services/ui/ui_page_looper_pianoroll.d \
./Services/ui/ui_page_looper_timeline.d \
./Services/ui/ui_page_song.d \
./Services/ui/ui_page_midi_monitor.d \
./Services/ui/ui_page_sysex.d \
./Services/ui/ui_page_config.d \
./Services/ui/ui_page_livefx.d \
./Services/ui/ui_state.d \
./Services/ui/ui_status.d 


# Each subdirectory must supply rules for building sources it contributes
Services/ui/%.o Services/ui/%.su Services/ui/%.cyclo: ../Services/ui/%.c Services/ui/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSRIO_ENABLE -DDIN_SELFTEST -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I"J:/workspace_2.0.0/MidiCore_MERGED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Services-2f-ui

clean-Services-2f-ui:
	-$(RM) ./Services/ui/chord_cfg.cyclo ./Services/ui/chord_cfg.d ./Services/ui/chord_cfg.o ./Services/ui/chord_cfg.su ./Services/ui/ui.cyclo ./Services/ui/ui.d ./Services/ui/ui.o ./Services/ui/ui.su ./Services/ui/ui_actions.cyclo ./Services/ui/ui_actions.d ./Services/ui/ui_actions.o ./Services/ui/ui_actions.su ./Services/ui/ui_actions_impl.cyclo ./Services/ui/ui_actions_impl.d ./Services/ui/ui_actions_impl.o ./Services/ui/ui_actions_impl.su ./Services/ui/ui_actions_stubs.cyclo ./Services/ui/ui_actions_stubs.d ./Services/ui/ui_actions_stubs.o ./Services/ui/ui_actions_stubs.su ./Services/ui/ui_bindings.cyclo ./Services/ui/ui_bindings.d ./Services/ui/ui_bindings.o ./Services/ui/ui_bindings.su ./Services/ui/ui_encoders.cyclo ./Services/ui/ui_encoders.d ./Services/ui/ui_encoders.o ./Services/ui/ui_encoders.su ./Services/ui/ui_gfx.cyclo ./Services/ui/ui_gfx.d ./Services/ui/ui_gfx.o ./Services/ui/ui_gfx.su ./Services/ui/ui_page_looper.cyclo ./Services/ui/ui_page_looper.d ./Services/ui/ui_page_looper.o ./Services/ui/ui_page_looper.su ./Services/ui/ui_page_looper_pianoroll.cyclo ./Services/ui/ui_page_looper_pianoroll.d ./Services/ui/ui_page_looper_pianoroll.o ./Services/ui/ui_page_looper_pianoroll.su ./Services/ui/ui_page_looper_timeline.cyclo ./Services/ui/ui_page_looper_timeline.d ./Services/ui/ui_page_looper_timeline.o ./Services/ui/ui_page_looper_timeline.su ./Services/ui/ui_page_song.cyclo ./Services/ui/ui_page_song.d ./Services/ui/ui_page_song.o ./Services/ui/ui_page_song.su ./Services/ui/ui_page_midi_monitor.cyclo ./Services/ui/ui_page_midi_monitor.d ./Services/ui/ui_page_midi_monitor.o ./Services/ui/ui_page_midi_monitor.su ./Services/ui/ui_page_sysex.cyclo ./Services/ui/ui_page_sysex.d ./Services/ui/ui_page_sysex.o ./Services/ui/ui_page_sysex.su ./Services/ui/ui_page_config.cyclo ./Services/ui/ui_page_config.d ./Services/ui/ui_page_config.o ./Services/ui/ui_page_config.su ./Services/ui/ui_state.cyclo ./Services/ui/ui_state.d ./Services/ui/ui_state.o ./Services/ui/ui_state.su ./Services/ui/ui_status.cyclo ./Services/ui/ui_status.d ./Services/ui/ui_status.o ./Services/ui/ui_status.su

.PHONY: clean-Services-2f-ui


################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include USB_HOST/Target/subdir.mk
-include USB_HOST/App/subdir.mk
-include Services/zones/subdir.mk
-include Services/watchdog/subdir.mk
-include Services/velocity/subdir.mk
-include Services/usb_midi/subdir.mk
-include Services/usb_host_midi/subdir.mk
-include Services/ui/subdir.mk
-include Services/system/subdir.mk
-include Services/srio/subdir.mk
-include Services/safe/subdir.mk
-include Services/router/subdir.mk
-include Services/pressure/subdir.mk
-include Services/patch/subdir.mk
-include Services/midi/subdir.mk
-include Services/looper/subdir.mk
-include Services/log/subdir.mk
-include Services/instrument/subdir.mk
-include Services/input/subdir.mk
-include Services/humanize/subdir.mk
-include Services/fs/subdir.mk
-include Services/expression/subdir.mk
-include Services/dream/subdir.mk
-include Services/dout/subdir.mk
-include Services/config/subdir.mk
-include Services/ain/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Middlewares/Third_Party/FatFs/src/option/subdir.mk
-include Middlewares/Third_Party/FatFs/src/subdir.mk
-include Middlewares/ST/STM32_USB_Host_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/subdir.mk
-include Hal/uart_midi/subdir.mk
-include Hal/oled_ssd1322/subdir.mk
-include Hal/ainser64_hw/subdir.mk
-include Hal/subdir.mk
-include FATFS/Target/subdir.mk
-include FATFS/App/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include App/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := MidiCore
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
MidiCore.elf \

MAP_FILES += \
MidiCore.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
MidiCore.list \

OBJCOPY_HEX += \
MidiCore.hex \

OBJCOPY_BIN += \
MidiCore.bin \


# All Target
all: main-build

# Main-build Target
main-build: MidiCore.elf secondary-outputs

# Tool invocations
MidiCore.elf MidiCore.map: $(OBJS) $(USER_OBJS) J:\workspace_2.0.0\MidiCore_MERGED\STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "MidiCore.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"J:\workspace_2.0.0\MidiCore_MERGED\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="MidiCore.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

MidiCore.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "MidiCore.list"
	@echo 'Finished building: $@'
	@echo ' '

MidiCore.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "MidiCore.hex"
	@echo 'Finished building: $@'
	@echo ' '

MidiCore.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "MidiCore.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) MidiCore.bin MidiCore.elf MidiCore.hex MidiCore.list MidiCore.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets

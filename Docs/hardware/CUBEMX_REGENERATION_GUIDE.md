# CubeMX Code Regeneration Guide for OLED Software SPI

## Issue
When trying to regenerate code with STM32CubeMX, you may encounter overwrite warnings because the generated GPIO initialization code differs from the current `main.c`.

## Solution

### Option 1: Let CubeMX Regenerate (Recommended)
Since the `.ioc` file now has the correct pin labels, you can safely let CubeMX regenerate the code:

1. **Backup your changes** (optional, but recommended):
   ```bash
   git stash
   ```

2. **Open MidiCore.ioc in STM32CubeMX**

3. **Verify pin labels** in the Pinout view:
   - **PA8** should be labeled `OLED_RST`
   - **PC8** should be labeled `OLED_SCL`
   - **PC11** should be labeled `OLED_SDA`
   - **PC4** should have NO label (just GPIO_Output)
   - **PC5** should have NO label (just GPIO_Output)

4. **Generate Code**:
   - Click "Project" → "Generate Code"
   - When prompted about file conflicts, choose **"Yes to All"** to overwrite

5. **The regenerated code will now be correct** because:
   - CubeMX will generate `OLED_RST_Pin` on PA8 (GPIOA)
   - CubeMX will generate `OLED_SCL_Pin` on PC8 (GPIOC)
   - CubeMX will generate `OLED_SDA_Pin` on PC11 (GPIOC)
   - No `OLED_DC_Pin` will be generated (PC4 has no label)

### Option 2: Manual Fix (If Option 1 Fails)

If you cannot use CubeMX to regenerate, manually ensure `Core/Inc/main.h` has these definitions:

```c
/* OLED Software SPI Pins */
#define OLED_RST_Pin GPIO_PIN_8
#define OLED_RST_GPIO_Port GPIOA
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOC
#define OLED_SDA_Pin GPIO_PIN_11
#define OLED_SDA_GPIO_Port GPIOC
```

And `Core/Src/main.c` MX_GPIO_Init() function should have:

```c
/* Configure GPIO pin Output Level for OLED pins on GPIOC */
HAL_GPIO_WritePin(GPIOC, ...|OLED_SCL_Pin|...|OLED_SDA_Pin, GPIO_PIN_RESET);

/* Configure GPIO pin Output Level for OLED RST on GPIOA */
HAL_GPIO_WritePin(GPIOA, OLED_RST_Pin|..., GPIO_PIN_RESET);

/* Configure GPIO pins : OLED_SCL_Pin OLED_SDA_Pin (on GPIOC) */
GPIO_InitStruct.Pin = ...|OLED_SCL_Pin|...|OLED_SDA_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* Configure GPIO pins : OLED_RST_Pin (on GPIOA) */
GPIO_InitStruct.Pin = OLED_RST_Pin|...;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

### Option 3: Force Clean Regeneration

If CubeMX still has issues:

1. **Delete the generated files** (CubeMX will regenerate them):
   ```bash
   rm Core/Src/main.c
   rm Core/Inc/main.h
   ```

2. **Open STM32CubeMX** and generate code

3. **Re-apply any USER CODE sections** if you had custom modifications

## Current Pin Configuration

**Software SPI (bit-bang) for OLED SSD1322**:
- **RST** = PA8 (Reset control, active low)
- **SCL** = PC8 (Clock signal, bit-banged)
- **SDA** = PC11 (Data signal, bit-banged)
- **CS** = Hardwired to GND on display (not connected to STM32)

**Note**: This configuration matches MIOS32 LoopA conventions for maximum compatibility.

## Verification

After regeneration, verify the build compiles without errors:
```bash
make clean
make -j16 all
```

You should see no errors about `OLED_DC_Pin` or undefined pins.

## Troubleshooting

### Error: `'OLED_DC_Pin' undeclared`
- **Cause**: Old pin labels still in `.ioc` file or cached build
- **Fix**: Delete `build/` directory and regenerate with CubeMX

### Error: `'OLED_RST_Pin' undeclared`
- **Cause**: Pin label not set in `.ioc` file
- **Fix**: Open `.ioc`, set PA8 label to `OLED_RST`, regenerate

### CubeMX says "Files will be overwritten"
- **This is normal** - the `.ioc` file configuration has changed
- **Safe to proceed** if you've committed your custom code

## Related Files
- `.ioc` file: `MidiCore.ioc` (STM32CubeMX project)
- Pin definitions: `Config/oled_pins.h` (software SPI pins)
- OLED config: `Config/oled_config.h` (display parameters)
- Driver: `Hal/oled_ssd1322/oled_ssd1322.c` (software SPI implementation)

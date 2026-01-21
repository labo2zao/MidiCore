# SPI Configuration Parameters - Current Code Reference

This document lists all SPI parameters currently configured in `Core/Src/main.c` so you can manually replicate them in STM32CubeMX.

## SPI1 Configuration

**Instance**: SPI1

**Parameters**:
- Mode: `SPI_MODE_MASTER` (Master mode)
- Direction: `SPI_DIRECTION_2LINES` (Full duplex)
- Data Size: `SPI_DATASIZE_8BIT` (8-bit data frame)
- Clock Polarity: `SPI_POLARITY_LOW` (Clock idle state is low)
- Clock Phase: `SPI_PHASE_1EDGE` (Data captured on first edge)
- NSS: `SPI_NSS_SOFT` (Software slave select management)
- Baud Rate Prescaler: `SPI_BAUDRATEPRESCALER_8` (Prescaler = 8)
- First Bit: `SPI_FIRSTBIT_MSB` (MSB transmitted first)
- TI Mode: `SPI_TIMODE_DISABLE` (TI mode disabled)
- CRC Calculation: `SPI_CRCCALCULATION_DISABLE` (CRC disabled)
- CRC Polynomial: `10`

### CubeMX Settings for SPI1:
```
Mode: Master
Hardware NSS Signal: Disable
Frame Format: Motorola
Data Size: 8 Bits
First Bit: MSB First
Prescaler (for Baud Rate): 8
Clock Polarity (CPOL): Low
Clock Phase (CPHA): 1 Edge
CRC Calculation: Disabled
NSS Signal Type: Software
```

---

## SPI2 Configuration

**Instance**: SPI2

**Parameters**:
- Mode: `SPI_MODE_MASTER` (Master mode)
- Direction: `SPI_DIRECTION_2LINES` (Full duplex)
- Data Size: `SPI_DATASIZE_8BIT` (8-bit data frame)
- Clock Polarity: `SPI_POLARITY_LOW` (Clock idle state is low)
- Clock Phase: `SPI_PHASE_1EDGE` (Data captured on first edge)
- NSS: `SPI_NSS_SOFT` (Software slave select management)
- **Baud Rate Prescaler**: `SPI_BAUDRATEPRESCALER_2` (Prescaler = 2) ⚠️
- First Bit: `SPI_FIRSTBIT_MSB` (MSB transmitted first)
- TI Mode: `SPI_TIMODE_DISABLE` (TI mode disabled)
- CRC Calculation: `SPI_CRCCALCULATION_DISABLE` (CRC disabled)
- CRC Polynomial: `10`

### CubeMX Settings for SPI2:
```
Mode: Master
Hardware NSS Signal: Disable
Frame Format: Motorola
Data Size: 8 Bits
First Bit: MSB First
Prescaler (for Baud Rate): 2  ⚠️ Different from SPI1!
Clock Polarity (CPOL): Low
Clock Phase (CPHA): 1 Edge
CRC Calculation: Disabled
NSS Signal Type: Software
```

---

## SPI3 Configuration

**Instance**: SPI3

**Parameters**:
- Mode: `SPI_MODE_MASTER` (Master mode)
- Direction: `SPI_DIRECTION_2LINES` (Full duplex)
- Data Size: `SPI_DATASIZE_8BIT` (8-bit data frame)
- Clock Polarity: `SPI_POLARITY_LOW` (Clock idle state is low)
- Clock Phase: `SPI_PHASE_1EDGE` (Data captured on first edge)
- NSS: `SPI_NSS_SOFT` (Software slave select management)
- **Baud Rate Prescaler**: `SPI_BAUDRATEPRESCALER_2` (Prescaler = 2) ⚠️
- First Bit: `SPI_FIRSTBIT_MSB` (MSB transmitted first)
- TI Mode: `SPI_TIMODE_DISABLE` (TI mode disabled)
- CRC Calculation: `SPI_CRCCALCULATION_DISABLE` (CRC disabled)
- CRC Polynomial: `10`

### CubeMX Settings for SPI3:
```
Mode: Master
Hardware NSS Signal: Disable
Frame Format: Motorola
Data Size: 8 Bits
First Bit: MSB First
Prescaler (for Baud Rate): 2  ⚠️ Different from SPI1!
Clock Polarity (CPOL): Low
Clock Phase (CPHA): 1 Edge
CRC Calculation: Disabled
NSS Signal Type: Software
```

---

## Key Differences Between SPI Instances

| Parameter | SPI1 | SPI2 | SPI3 |
|-----------|------|------|------|
| Baud Rate Prescaler | **8** | **2** | **2** |
| All other parameters | Identical | Identical | Identical |

**Important**: SPI1 runs 4x slower than SPI2 and SPI3 due to the different prescaler.

---

## How to Configure in STM32CubeMX

1. **Open** `MidiCore.ioc` in STM32CubeMX

2. **For each SPI (SPI1, SPI2, SPI3)**:
   - Navigate to: Connectivity → SPIx
   - Mode: Select "Full-Duplex Master"
   
3. **In Configuration tab → Parameter Settings**:
   - Frame Format: **Motorola**
   - Data Size: **8 Bits**
   - First Bit: **MSB First**
   - Prescaler: 
     - **SPI1**: Set to **8**
     - **SPI2**: Set to **2**
     - **SPI3**: Set to **2**
   - Clock Polarity (CPOL): **Low**
   - Clock Phase (CPHA): **1 Edge**
   - CRC Calculation: **Disabled**
   - NSS Signal Type: **Software**

4. **Verify pin assignments** match your hardware

5. **Generate Code**

---

## Notes

- All SPI peripherals are configured as Master
- All use software NSS management
- SPI1 has different prescaler (8 vs 2) - this affects baud rate
- Make sure to preserve your user code sections (see FreeRTOS guide)

---

## Quick Reference Table

| Setting | SPI1 | SPI2 | SPI3 |
|---------|------|------|------|
| Mode | Master | Master | Master |
| Direction | Full Duplex | Full Duplex | Full Duplex |
| Data Size | 8 Bits | 8 Bits | 8 Bits |
| CPOL | Low | Low | Low |
| CPHA | 1 Edge | 1 Edge | 1 Edge |
| NSS | Software | Software | Software |
| **Prescaler** | **8** | **2** | **2** |
| First Bit | MSB | MSB | MSB |
| TI Mode | Disabled | Disabled | Disabled |
| CRC | Disabled | Disabled | Disabled |

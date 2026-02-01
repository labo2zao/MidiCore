# 🔧 MidiCore Hardware Setup

**Wiring diagrams and pin tables**

---

## 📦 What You Need

### Minimum
| Part | Notes |
|------|-------|
| STM32F407VGT6 board | Main controller |
| USB cable | For MIDI and power |

### Optional
| Part | Notes |
|------|-------|
| OLED display (SSD1322) | 256×64 screen |
| SD card | Save settings |
| Rotary encoders | Menu navigation |
| Pressure sensor (XGZP6847D) | Bellows expression |
| AINSER64 board | 64 analog inputs |
| SRIO board | Button/LED I/O |
| MIDI DIN connectors | 5-pin MIDI |

---

## 🖥️ OLED Display Wiring

### Display: SSD1322 256×64

| Display Pin | STM32 Pin | Function |
|-------------|-----------|----------|
| GND | GND | Ground |
| VCC | 3.3V | Power |
| SCL (Clock) | PC8 | SPI clock |
| SDA (Data) | PC11 | SPI data |
| D/C | PA8 | Data/Command |
| CS | GND | Always selected |
| Reset | - | Use on-board reset |

### Wiring Diagram
```
OLED Display           STM32F407
┌──────────┐           ┌──────────┐
│ GND      │───────────│ GND      │
│ VCC      │───────────│ 3.3V     │
│ SCL      │───────────│ PC8      │
│ SDA      │───────────│ PC11     │
│ D/C      │───────────│ PA8      │
│ CS       │───────────│ GND      │
└──────────┘           └──────────┘
```

---

## 🎚️ MIDI DIN Wiring

### Standard 5-Pin DIN

| Port | UART | TX Pin | RX Pin |
|------|------|--------|--------|
| MIDI 1 | UART1 | PA9 | PA10 |
| MIDI 2 | UART2 | PD5 | PD6 |
| MIDI 3 | UART3 | PB10 | PB11 |
| MIDI 4 | UART6 | PC6 | PC7 |

### DIN Connector Wiring
```
DIN OUT (to other device):
  Pin 2 → GND (shield)
  Pin 4 → +5V through 220Ω
  Pin 5 → UART TX through 220Ω

DIN IN (from other device):
  Pin 2 → GND
  Pin 4 → Optocoupler anode
  Pin 5 → Optocoupler cathode
```

---

## 📊 AINSER64 (Analog Inputs)

### SPI Connection

| AINSER64 | STM32 Pin | Function |
|----------|-----------|----------|
| MOSI | PB15 | SPI data out |
| MISO | PB14 | SPI data in |
| SCK | PB13 | SPI clock |
| CS | PB12 | Chip select |
| GND | GND | Ground |
| VCC | 3.3V | Power |

### Multiple MCP3208 Chips
```
STM32 SPI                  MCP3208 #1      MCP3208 #2
┌─────────┐               ┌─────────┐     ┌─────────┐
│ MOSI    │──────────────▶│ DIN     │ ───▶│ DIN     │
│ MISO    │◀──────────────│ DOUT    │◀─── │ DOUT    │
│ SCK     │──────────────▶│ CLK     │ ───▶│ CLK     │
│ CS0     │──────────────▶│ /CS     │     │         │
│ CS1     │─────────────────────────────▶│ /CS     │
└─────────┘               └─────────┘     └─────────┘
```

---

## 🔘 SRIO (Shift Registers)

### DIN (Digital Input) - 74HC165

| 74HC165 | STM32 Pin | Function |
|---------|-----------|----------|
| QH (Pin 9) | PB14 | Data out |
| CLK (Pin 2) | PB13 | Clock |
| /PL (Pin 1) | PD10 | Load |
| GND | GND | Ground |
| VCC | 3.3V | Power |

### DOUT (Digital Output) - 74HC595

| 74HC595 | STM32 Pin | Function |
|---------|-----------|----------|
| SER (Pin 14) | PB15 | Data in |
| SRCLK (Pin 11) | PB13 | Clock |
| RCLK (Pin 12) | PB12 | Latch |
| /OE (Pin 13) | GND | Always enabled |
| GND | GND | Ground |
| VCC | 3.3V | Power |

### Daisy Chain
```
STM32        74HC165 #1      74HC165 #2
┌─────┐      ┌─────────┐     ┌─────────┐
│ SPI │◀─────│ QH      │◀────│ QH      │
│     │──────│ CLK     │─────│ CLK     │
│     │──────│ /PL     │─────│ /PL     │
└─────┘      └─────────┘     └─────────┘
```

---

## 🌬️ Pressure Sensor (Bellows)

### XGZP6847D I2C Connection

| Sensor | STM32 Pin | Function |
|--------|-----------|----------|
| SDA | PB7 | I2C data |
| SCL | PB6 | I2C clock |
| GND | GND | Ground |
| VCC | 3.3V | Power |

### Wiring
```
Pressure Sensor          STM32F407
┌──────────────┐         ┌──────────┐
│ VCC          │─────────│ 3.3V     │
│ GND          │─────────│ GND      │
│ SDA          │─────────│ PB7      │
│ SCL          │─────────│ PB6      │
└──────────────┘         └──────────┘

Add 4.7kΩ pull-ups on SDA and SCL to 3.3V
```

---

## 💾 SD Card (SDIO)

| SD Card | STM32 Pin | Function |
|---------|-----------|----------|
| CLK | PC12 | Clock |
| CMD | PD2 | Command |
| DAT0 | PC8 | Data 0 |
| DAT1 | PC9 | Data 1 |
| DAT2 | PC10 | Data 2 |
| DAT3 | PC11 | Data 3 |
| GND | GND | Ground |
| VCC | 3.3V | Power |

---

## 🔌 USB Connections

### USB Device (to computer)

| USB | STM32 Pin | Function |
|-----|-----------|----------|
| D+ | PA12 | USB Data+ |
| D- | PA11 | USB Data- |
| VBUS | PA9 | 5V sense |
| GND | GND | Ground |

### USB Host (for controllers)

| USB | STM32 Pin | Function |
|-----|-----------|----------|
| D+ | PB15 | USB Data+ |
| D- | PB14 | USB Data- |
| VBUS | - | 5V output |
| GND | GND | Ground |

---

## 🎛️ Rotary Encoders

| Encoder | STM32 Pins | Function |
|---------|------------|----------|
| Encoder 1 | PA0, PA1 | Navigation |
| Encoder 2 | PA2, PA3 | Value |
| Button 1 | PA4 | Select |
| Button 2 | PA5 | Back |

### Wiring
```
Encoder                  STM32
┌──────────────┐         ┌──────────┐
│ A            │─────────│ PA0      │
│ B            │─────────│ PA1      │
│ COM (common) │─────────│ GND      │
│ Button       │─────────│ PA4      │
└──────────────┘         └──────────┘
```

---

## ⚡ Power

### Requirements

| Rail | Current | Notes |
|------|---------|-------|
| 5V | 500mA | From USB or external |
| 3.3V | 300mA | Generated from 5V |

### USB Power
- USB provides 5V/500mA
- STM32 board has 3.3V regulator
- OK for most configurations

### External Power
- If using many LEDs: Add external 5V supply
- Connect external 5V GND to board GND

---

## 📋 Pin Summary Table

| Function | Pin(s) | Notes |
|----------|--------|-------|
| **USB Device** | PA11, PA12 | To computer |
| **USB Host** | PB14, PB15 | Conflicts with SPI2! |
| **MIDI 1** | PA9, PA10 | UART1 |
| **MIDI 2** | PD5, PD6 | UART2 |
| **SPI (AINSER)** | PB13-15 | SPI2 (conflicts with USB Host) |
| **I2C (Pressure)** | PB6, PB7 | I2C1 |
| **OLED** | PC8, PC11, PA8 | Software SPI |
| **SD Card** | PC8-12, PD2 | SDIO |
| **Encoders** | PA0-PA5 | GPIO |
| **SRIO Control** | PD10, PB12 | Load/Latch |

⚠️ **Note:** USB Host and SPI2 share pins PB14/PB15. 
Choose ONE: either AINSER64 (SPI) or USB Host (keyboards).

---

## ✅ Checklist Before Power On

- [ ] GND connected between all boards
- [ ] 3.3V not exceeding 3.6V anywhere
- [ ] No shorts between pins
- [ ] SPI/I2C pull-ups in place if needed
- [ ] USB cable tested

---

**Questions?** Check `5_TROUBLESHOOTING.md`

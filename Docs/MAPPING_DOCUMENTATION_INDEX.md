# MidiCore Mapping & Parsing Documentation Index

## 📚 Documentation Overview

Complete bilingual documentation for all MidiCore SD card mapping and text parsing modules.

## 🗂️ Module Documentation

### AINSER Module (Analog Inputs)
Maps analog sensor readings to MIDI Control Changes.

- 🇬🇧 [English Documentation](../Services/ainser/README.md)
- 🇫🇷 [Documentation Française](../Services/ainser/README_FR.md)

**Features:** 64 channels, CC mapping, response curves, threshold filtering, SD configuration

### DIN Module (Digital Inputs)
Maps button/switch states to MIDI Notes or Control Changes.

- 🇬🇧 [English Documentation](../Services/din/README.md)
- 🇫🇷 [Documentation Française](../Services/din/README_FR.md)

**Features:** 64 channels, NOTE/CC modes, velocity control, polarity inversion, SD configuration

### DOUT Module (Digital Outputs)
Maps logical bits to physical hardware outputs with RGB LED support.

- 🇬🇧 [English Documentation](../Services/dout/README.md)
- 🇫🇷 [Documentation Française](../Services/dout/README_FR.md)

**Features:** 64-bit mapping, per-bit inversion, RGB LED control (16 LEDs), common anode/cathode

## 📖 Reference Guides

### Mapping Modules Reference
Consolidated quick reference with function tables for all modules.

- 🇬🇧 [Mapping Modules Reference](MAPPING_MODULES_REFERENCE.md)

**Contents:** Function tables, integration examples, configuration patterns, troubleshooting

### NGC Text Format Parsing
Complete specification of the NGC text configuration file format.

- 🇬🇧 [English Reference](NGC_TEXT_PARSING_REFERENCE.md)
- 🇫🇷 [Référence Française](NGC_TEXT_PARSING_REFERENCE_FR.md)

**Contents:** Syntax rules, parsing commands, numeric formats, parser behavior, error handling

### Multi-Channel Selection
Specification for multi-channel configuration syntax (comma and dash notation).

- 🇬🇧 [English Specification](NGC_MULTI_CHANNEL_SPEC.md)
- 🇫🇷 [Spécification Française](NGC_MULTI_CHANNEL_SPEC_FR.md)

**Contents:** Range syntax (`CH0-5`), comma syntax (`CH0,1,2`), mixed notation, parser algorithms

## 🔍 Quick Navigation

### By Use Case

| Use Case | Module | Documentation |
|----------|--------|---------------|
| Analog sensors (pots, sliders) | AINSER | [EN](../Services/ainser/README.md) • [FR](../Services/ainser/README_FR.md) |
| Buttons, switches | DIN | [EN](../Services/din/README.md) • [FR](../Services/din/README_FR.md) |
| LEDs, indicators | DOUT | [EN](../Services/dout/README.md) • [FR](../Services/dout/README_FR.md) |
| Configuration files | NGC Format | [EN](NGC_TEXT_PARSING_REFERENCE.md) • [FR](NGC_TEXT_PARSING_REFERENCE_FR.md) |
| Bulk channel setup | Multi-Channel | [EN](NGC_MULTI_CHANNEL_SPEC.md) • [FR](NGC_MULTI_CHANNEL_SPEC_FR.md) |

### By Topic

| Topic | Document |
|-------|----------|
| Function reference | [Mapping Modules Reference](MAPPING_MODULES_REFERENCE.md) |
| File format syntax | [NGC Parsing Reference EN](NGC_TEXT_PARSING_REFERENCE.md) / [FR](NGC_TEXT_PARSING_REFERENCE_FR.md) |
| Configuration examples | All module READMEs |
| Integration code | [Mapping Reference](MAPPING_MODULES_REFERENCE.md), Module READMEs |
| Troubleshooting | All module READMEs |
| Parser implementation | [NGC Parsing Reference](NGC_TEXT_PARSING_REFERENCE.md) |
| Multi-channel syntax | [Multi-Channel Spec EN](NGC_MULTI_CHANNEL_SPEC.md) / [FR](NGC_MULTI_CHANNEL_SPEC_FR.md) |

## 📝 Configuration File Examples

### AINSER Configuration (`0:/cfg/ainser_map.ngc`)
```ini
# Bellows sensor with logarithmic response
[CH16]
CC=36
CURVE=LOG
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

# Multiple channels with range notation
[CH0-7]
CC=16
CURVE=LINEAR
ENABLED=1
```

### DIN Configuration (`0:/cfg/din_map.ngc`)
```ini
# Piano keyboard (12 keys)
[CH0-11]
TYPE=NOTE
NUMBER=48
VEL_ON=100
ENABLED=1

# Sustain pedal
[CH48]
TYPE=CC
NUMBER=64
INVERT=1
ENABLED=1
```

## 🔧 API Quick Reference

### AINSER Functions
| Function | Purpose |
|----------|---------|
| `ainser_map_init_defaults()` | Initialize with defaults |
| `ainser_map_load_sd()` | Load config from SD |
| `ainser_map_process_channel()` | Process ADC reading |
| `ainser_map_set_output_cb()` | Set MIDI output callback |
| `ainser_map_get_table()` | Get config table |

### DIN Functions
| Function | Purpose |
|----------|---------|
| `din_map_init_defaults()` | Initialize with defaults |
| `din_map_load_sd()` | Load config from SD |
| `din_map_process_event()` | Process button event |
| `din_map_set_output_cb()` | Set MIDI output callback |
| `din_map_get_table()` | Get config table |

### DOUT Functions
| Function | Purpose |
|----------|---------|
| `dout_map_init()` | Initialize with config |
| `dout_map_apply()` | Convert logical to physical |
| `dout_set_rgb()` | Set RGB LED state |

## 🌍 Language Support

All documentation is available in:
- 🇬🇧 **English** - Primary language
- 🇫🇷 **Français** - Complete translations

## 📊 Documentation Statistics

- **Total Files:** 11
- **Total Lines:** 4,071
- **Languages:** 2 (English, French)
- **Modules Covered:** 3 (AINSER, DIN, DOUT)
- **Reference Guides:** 3 (Mapping, NGC Parsing, Multi-Channel)

## 🔗 Related Documentation

- [SD Card File Structure](../Assets/sd_cfg/README_SD_TREE.txt)
- [MIDI Router Documentation](../Services/router/README.md) *(if exists)*
- [Config Service Documentation](../Services/config/README.md) *(if exists)*

## 🆕 Recent Updates

- **2026-01-25:** Initial complete documentation release
  - All 3 mapping modules documented (AINSER, DIN, DOUT)
  - NGC text format parsing specification
  - Multi-channel selection specification
  - Full bilingual support (EN/FR)

---

**Documentation Version:** 1.0  
**Last Updated:** 2026-01-25  
**Maintained By:** MidiCore Development Team

# 💻 MidiCore Commands

**All commands you need, one page**

---

## 🔵 System Commands

| Command | What it does |
|---------|--------------|
| `help` | Show all commands |
| `help module` | Show module help |
| `version` | Show firmware version |
| `uptime` | Show how long running |
| `status` | Show system health |
| `reboot` | Restart system |
| `clear` | Clear screen |

---

## 🟢 Module Commands

### See What's Available

| Command | What it does |
|---------|--------------|
| `module list` | List ALL modules |
| `module info arpeggiator` | Info about one module |
| `module params looper` | List parameters |

---

### Turn Modules On/Off

| Command | What it does |
|---------|--------------|
| `module enable arpeggiator` | Turn ON |
| `module disable arpeggiator` | Turn OFF |
| `module status arpeggiator` | Check if ON/OFF |

---

### Change Settings

| Command | What it does |
|---------|--------------|
| `module get looper bpm` | See current value |
| `module set looper bpm 120` | Change value |
| `module set arpeggiator pattern UP` | Set to UP |

---

## 🟡 Track Commands

Some modules have 4 tracks (0, 1, 2, 3).

Add track number at the end:

| Command | What it does |
|---------|--------------|
| `module enable midi_filter 0` | Enable on track 0 |
| `module set looper mute true 1` | Mute track 1 |
| `module get harmonizer interval 2` | Get track 2 setting |

---

## 🟠 Config Commands

| Command | What it does |
|---------|--------------|
| `config save 0:/mysettings.ini` | Save to SD card |
| `config load 0:/mysettings.ini` | Load from SD card |
| `config list` | Show all saved settings |

---

## 📋 Most Common Examples

### Looper Setup
```
module set looper bpm 120
module set looper time_sig_num 4
module enable looper 0
module set looper state REC 0
```

### Arpeggiator
```
module enable arpeggiator
module set arpeggiator pattern UP_DOWN
```

### MIDI Filter
```
module enable midi_filter 0
module set midi_filter min_note 36 0
module set midi_filter max_note 96 0
```

### Harmonizer
```
module enable harmonizer 0
module set harmonizer voice1_interval THIRD_UP 0
module set harmonizer voice1_enabled true 0
```

### Save Everything
```
config save 0:/my_accordion.ini
```

---

## 🔢 Parameter Types

### Boolean (true/false)
```
module set looper mute true 0
module set looper mute false 0
```

Also works: `1`, `0`, `on`, `off`, `yes`, `no`

### Numbers
```
module set looper bpm 120
module set midi_filter min_note 36 0
```

### Choices (enum)
```
module set arpeggiator pattern UP
module set arpeggiator pattern DOWN
module set arpeggiator pattern UP_DOWN
module set arpeggiator pattern RANDOM
```

---

## ⚠️ Error Messages

| Error | Meaning | Fix |
|-------|---------|-----|
| "Module not found" | Typo in name | Check spelling |
| "Parameter not found" | Wrong param name | Use `module params` |
| "Value out of range" | Number too big/small | Check valid range |
| "Invalid track index" | Track 4+ doesn't exist | Use 0, 1, 2, or 3 |

---

## 📖 Quick Reference Card

```
┌────────────────────────────────────────────┐
│  MOST USED COMMANDS                        │
├────────────────────────────────────────────┤
│  help              → See all commands      │
│  module list       → See all modules       │
│  module enable X   → Turn on module X      │
│  module disable X  → Turn off module X     │
│  module set X Y Z  → Set X.Y = Z           │
│  module get X Y    → Get value of X.Y      │
│  config save F     → Save to file F        │
│  config load F     → Load from file F      │
│  status            → System health         │
│  reboot            → Restart               │
└────────────────────────────────────────────┘
```

---

**Need more details?** Type `help module` in the terminal.

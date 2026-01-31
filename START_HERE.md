# MidiCore - Start Here! 🎵

---

## 📚 NEW: Simple Docs (Dyslexia-Friendly)

**5 easy documents, designed for accessibility:**

| Doc | What's Inside | Time |
|-----|---------------|------|
| [1_QUICK_START](Docs/SIMPLE/1_QUICK_START.md) | Connect and start | 3 min |
| [2_COMMANDS](Docs/SIMPLE/2_COMMANDS.md) | All CLI commands | 5 min |
| [3_MODULES](Docs/SIMPLE/3_MODULES.md) | All 60+ modules | 10 min |
| [4_HARDWARE](Docs/SIMPLE/4_HARDWARE.md) | Wiring diagrams | 5 min |
| [5_TROUBLESHOOTING](Docs/SIMPLE/5_TROUBLESHOOTING.md) | Fix problems | As needed |

**→ [Go to Simple Docs](Docs/SIMPLE/README.md)**

---

## What is MidiCore?

A complete system to control all features of your MIDI accordion through **TWO ways**:

### 1. 💻 CLI (Computer Terminal) - For Setup
Type text commands on a computer connected via USB cable

### 2. 🎛️ UI (Buttons & Screen) - For Playing
Use physical rotary encoders and buttons on the accordion

**BOTH control the exact same things!**

---

## What is CLI? (Simple Explanation)

**CLI = Command Line Interface**

It's like sending text messages to your accordion to tell it what to do.

### Example:
Instead of pressing 10 buttons to enable the arpeggiator...
Just type: `module enable arpeggiator`

**That's it!**

### What You Need:
1. A cheap USB cable adapter ($5)
2. Free software called PuTTY (on your computer)
3. 5 minutes to connect and learn

### Why It's Useful:
- ⚡ **Fast** - One command instead of navigating menus
- 📝 **Easy to share** - Copy/paste settings to friends
- 🔧 **Perfect for setup** - Configure everything quickly
- 💾 **Save/load** - Store different configurations

**Read `WHAT_IS_CLI.md` for complete beginner guide**

---

## The Complete System

```
┌─────────────────────────────────────────────────────┐
│           YOUR ACCORDION (MidiCore)                  │
│                                                      │
│  60+ Music Modules Inside:                          │
│   • Arpeggiator    • Looper      • Harmonizer      │
│   • Metronome      • Quantizer   • Delay           │
│   • Filter         • Transpose   • And 50+ more... │
│                                                      │
└─────────────────────────────────────────────────────┘
              ▲                           ▲
              │                           │
              │                           │
   ┌──────────┴────────┐      ┌──────────┴─────────┐
   │  Physical         │      │  Computer          │
   │  Buttons &        │      │  Terminal          │
   │  Rotary Encoders  │      │  (Text Commands)   │
   │                   │      │                    │
   │  For PLAYING →    │      │  For SETUP →       │
   └───────────────────┘      └────────────────────┘
```

---

## What Each Part Does

### 📋 Module Registry
**Think of it as:** A phone directory of all your accordion features

**What it stores:**
- All 60+ modules (arpeggiator, looper, effects, etc.)
- Their settings (enabled/disabled, tempo, pattern, etc.)
- How to control them

**You benefit:** Everything is organized and easy to find

---

### 💻 CLI System  
**Think of it as:** Text messages to your accordion

**What you can do:**
```
module list                    ← See all features
module enable arpeggiator      ← Turn something on
module set looper bpm 120      ← Change a setting
config save 0:/mysettings.ini  ← Save your setup
```

**You benefit:** Super fast configuration

**Read:** `WHAT_IS_CLI.md` and `CLI_VS_UI_VISUAL.md`

---

### 🎛️ UI Navigation System
**Think of it as:** Menus on your TV, controlled with a remote

**How it works:**
1. Turn encoder → Move through menu
2. Press button → Select item
3. Turn encoder → Change value
4. Press button → Confirm

**Menu structure:**
```
Main Menu
 ├─ Effects Category
 │   ├─ Arpeggiator [ON/OFF]
 │   │   ├─ Pattern: UP
 │   │   ├─ Rate: 8
 │   │   └─ Octaves: 2
 │   └─ Harmonizer [ON/OFF]
 └─ Looper Category
     └─ Tempo: 120 BPM
```

**You benefit:** Easy to use while playing

---

### 🔄 Rotary Encoder Handler
**Think of it as:** Smart knob controller

**What it does:**
- Makes encoders feel smooth
- Speed up when you turn fast
- Slow down when you turn slow
- No accidental changes

**You benefit:** Natural, responsive feel

---

## Quick Start Guide

### For Setup (Using Computer):

1. **Connect Hardware:**
   - USB-to-Serial adapter: $5 on Amazon
   - Connect to accordion's UART2 pins
   
2. **Open Terminal:**
   - Windows: Download PuTTY (free)
   - Settings: 115200 baud, COM port
   
3. **Try Commands:**
   ```
   help                          ← See what you can do
   module list                   ← See all features
   module enable arpeggiator     ← Try enabling something
   ```

4. **Save Your Settings:**
   ```
   config save 0:/mysetup.ini
   ```

**Time needed: 30 minutes first time**

---

### For Playing (Using Buttons):

1. **Turn encoder** → Navigate menu
2. **Press button** → Select category (Effects, Looper, etc.)
3. **Turn encoder** → Find module (Arpeggiator)
4. **Press button** → View/Edit
5. **Turn encoder** → Change settings
6. **Press button** → Confirm

**Works immediately, no computer needed**

---

## Real Example: Enable Arpeggiator

### Method 1: CLI (5 seconds)
```
midicore> module enable arpeggiator
midicore> module set arpeggiator pattern UP
```
**Done!**

### Method 2: Physical UI (30 seconds)
1. Turn encoder to "Effects" → Press button
2. Turn encoder to "Arpeggiator" → Press button  
3. Turn encoder to "Enabled" → Press button
4. Turn encoder to "ON" → Press button
5. Press back button
6. Turn encoder to "Pattern" → Press button
7. Turn encoder to "UP" → Press button

**Both do the same thing!**

---

## All The Documents

### Start Here (Easy):
1. **WHAT_IS_CLI.md** - What is CLI? Simple explanation
2. **CLI_VS_UI_VISUAL.md** - Pictures and comparisons
3. **README_CLI_MODULE_SYSTEM.md** - Complete overview

### For Understanding:
4. **MODULE_INVENTORY.md** - All 60+ modules explained
5. **Services/cli/README.md** - How CLI works

### For Developers:
6. **MODULE_CLI_INTEGRATION.md** - How to add new modules

---

## Common Questions

### "Do I NEED a computer?"

**For initial setup:** YES (recommended)
**For playing:** NO (works standalone)

Think of it like a smartphone - you set it up with a computer once, then use it anywhere.

---

### "Is this complicated?"

**No!** You only need to learn ~5 commands:
- `help` - Get help
- `module list` - See features
- `module enable <name>` - Turn on
- `module set <name> <setting> <value>` - Change
- `config save` - Save

That's it! Everything else is bonus.

---

### "What if I break something?"

**You can't!** Everything can be:
- Undone
- Reset
- Reloaded from backup

Just save a good configuration first: `config save 0:/backup.ini`

---

### "CLI or UI - which is better?"

**Neither!** They're tools for different jobs:

**CLI (Text):**
- ✅ Setup and testing
- ✅ Changing many things
- ✅ Troubleshooting

**UI (Buttons):**
- ✅ Playing live
- ✅ Quick adjustments  
- ✅ No computer needed

**Use both!** CLI for setup, UI for performance.

---

## What You Get

### Working Features:
✅ 60+ modules documented and controllable
✅ Text commands (CLI) for fast setup
✅ Physical buttons/screen for playing
✅ Save/load different configurations
✅ Enable/disable any feature
✅ Change all settings
✅ Help system built-in

### Documentation:
✅ Beginner guides with pictures
✅ Technical references
✅ Step-by-step tutorials
✅ Copy-paste examples

### Developer Tools:
✅ Easy templates to add modules
✅ Helper macros (copy-paste)
✅ Working examples

---

## Next Steps

### 1. Understand the Basics
- Read `WHAT_IS_CLI.md`
- Read `CLI_VS_UI_VISUAL.md`
- Understand you have TWO ways to control your accordion

### 2. Try CLI (Optional but Recommended)
- Get USB-to-Serial adapter ($5)
- Connect to computer
- Type `help` and explore
- Try a few commands

### 3. Use Physical UI
- Turn encoders
- Press buttons
- Navigate menus
- See how it matches what CLI does

### 4. Save Your Settings
- Use CLI: `config save 0:/mysetup.ini`
- Or use UI: Button → Save Config

### 5. Play Your Accordion!
- All modules are accessible
- All settings can be changed
- Both CLI and UI control the same system

---

## Summary in One Sentence

**You can now control every feature of your MIDI accordion either by typing commands on a computer (fast for setup) or using physical buttons and encoders (good for playing), and both methods work together perfectly.**

---

## Need Help?

1. **Type `help` in CLI** - See all commands
2. **Read `WHAT_IS_CLI.md`** - Understand the basics
3. **Read `MODULE_INVENTORY.md`** - See all features
4. **Experiment!** - You can't break anything

---

## Files to Read (In Order)

1. **Docs/WHAT_IS_CLI.md** ← Start here!
2. **Docs/CLI_VS_UI_VISUAL.md** ← Pictures!
3. **Docs/MODULE_INVENTORY.md** ← All features explained

**Total reading time: 30 minutes to understand everything**

---

**Remember: CLI and UI are friends, not enemies. They both control your accordion. Use CLI to set up, UI to play!** 🎵

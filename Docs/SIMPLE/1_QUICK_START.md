# 🚀 MidiCore Quick Start

**Read time: 3 minutes**

---

## What is MidiCore?

A MIDI system for your accordion/instrument.

**It does:**
- ✅ Read your keys/buttons
- ✅ Send MIDI to your computer/synth
- ✅ Add effects (arpeggiator, looper, etc.)
- ✅ Display info on a small screen

---

## 📦 What You Need

| Item | Why |
|------|-----|
| STM32F407 board | The brain |
| OLED display (optional) | To see menus |
| USB cable | Connect to computer |

---

## 🔌 How to Connect

### USB Cable
```
Your accordion  ───USB───▶  Computer
```

### What happens:
1. Plug in USB
2. Computer sees "MidiCore MIDI"
3. Open your music software
4. Play!

---

## 💻 First Commands

Connect via USB terminal (MIOS Studio or serial):

```
help              ← See all commands
module list       ← See all features
status            ← Check everything works
```

---

## ✅ Test It Works

### Step 1: Send a note
Press a key on your accordion.

### Step 2: Check computer
Your music software should receive the note.

### Step 3: Celebrate! 🎉
You're connected!

---

## 🎛️ Enable a Feature

Example: Turn on the arpeggiator

```
module enable arpeggiator
module set arpeggiator pattern UP
```

Now your notes will arpeggiate!

---

## 💾 Save Your Settings

```
config save 0:/my_setup.ini
```

Your settings are now on the SD card.

---

## ❓ Something Wrong?

| Problem | Solution |
|---------|----------|
| No USB | Check cable, replug |
| No sound | Check MIDI channel |
| Screen blank | Check wiring |

**Full troubleshooting:** See `5_TROUBLESHOOTING.md`

---

## 📚 Next Steps

1. **Learn commands** → `2_COMMANDS.md`
2. **Explore modules** → `3_MODULES.md`
3. **Setup hardware** → `4_HARDWARE.md`

---

**You're ready to go!** 🎵

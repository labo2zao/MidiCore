# ❓ MidiCore Troubleshooting

**Problem → Solution format**

---

## 🔴 USB Problems

### Problem: Computer doesn't see MidiCore

**Check:**
1. ✅ USB cable is data cable (not charging-only)
2. ✅ Cable plugged in firmly
3. ✅ Try different USB port
4. ✅ Try different cable

**Solution:**
```
# Unplug, wait 5 seconds, replug
# Check Device Manager (Windows) or System Info (Mac)
```

---

### Problem: USB disconnects randomly

**Check:**
1. ✅ Cable quality (try another)
2. ✅ USB port power (try powered hub)
3. ✅ Not using USB 3.0 port (try USB 2.0)

**Solution:**
- Use shorter cable (under 2m)
- Use USB 2.0 port
- Add powered USB hub

---

### Problem: MIDI notes don't arrive

**Check:**
1. ✅ MIDI software configured correctly
2. ✅ Correct MIDI port selected
3. ✅ Channel matches

**Solution:**
```
# In terminal, check router:
router show

# Test with status:
status
```

---

## 🖥️ OLED Display Problems

### Problem: Display is blank

**Check:**
1. ✅ Power connected (3.3V + GND)
2. ✅ All wires connected
3. ✅ Module enabled in config

**Solution:**
```
# Check if OLED module enabled:
# In Config/module_config.h:
#define MODULE_ENABLE_OLED 1
```

---

### Problem: Display shows garbage

**Check:**
1. ✅ Clock (SCL) and Data (SDA) not swapped
2. ✅ D/C pin connected correctly
3. ✅ 3.3V not 5V

**Solution:**
```
Swap SCL and SDA wires, try again
```

---

### Problem: Display flickers

**Check:**
1. ✅ Power supply stable
2. ✅ Connections not loose
3. ✅ GND connected

**Solution:**
- Add 100nF capacitor near display power pins
- Secure all connections

---

## 🎹 No Sound / No MIDI

### Problem: Keys don't produce MIDI

**Check:**
1. ✅ AIN module enabled
2. ✅ Calibration done
3. ✅ Router configured

**Solution:**
```
# Enable AIN:
module set ain enable true
module set ain velocity_enable true

# Check router:
router show
```

---

### Problem: Notes stuck on

**Check:**
1. ✅ Note off messages being sent
2. ✅ No MIDI loops
3. ✅ Router not doubling messages

**Solution:**
```
# Panic - all notes off:
reboot

# Or disconnect USB briefly
```

---

### Problem: Wrong MIDI channel

**Check:**
1. ✅ Module channel setting
2. ✅ Channelizer settings
3. ✅ Zone configuration

**Solution:**
```
# Check current channel:
module get looper midi_channel 0

# Set channel:
module set looper midi_channel 0 0
```

---

## 💾 SD Card Problems

### Problem: SD card not detected

**Check:**
1. ✅ Card formatted FAT32
2. ✅ Card fully inserted
3. ✅ Card not locked (switch)
4. ✅ Card under 32GB

**Solution:**
- Format card as FAT32
- Use 8GB or 16GB card
- Check card lock switch

---

### Problem: Can't save config

**Check:**
1. ✅ SD card inserted
2. ✅ Card not full
3. ✅ File path correct

**Solution:**
```
# Use correct path:
config save 0:/settings.ini

# Not:
config save settings.ini  ← Wrong!
```

---

### Problem: Settings lost after power off

**Check:**
1. ✅ Settings saved to SD
2. ✅ Auto-load configured

**Solution:**
```
# Save your settings:
config save 0:/default.ini

# Load on startup (automatic if named default.ini)
```

---

## 🔌 Hardware Problems

### Problem: No power

**Check:**
1. ✅ USB cable connected
2. ✅ Board LED on
3. ✅ 3.3V rail present

**Solution:**
- Try different USB port
- Try different cable
- Check board fuse/regulator

---

### Problem: Board resets randomly

**Check:**
1. ✅ Power stable
2. ✅ No shorts
3. ✅ Stack overflow (debug)

**Solution:**
```
# Check system status:
status

# Check memory:
# If low memory, disable unused modules
```

---

### Problem: Analog inputs jumping

**Check:**
1. ✅ Good connections
2. ✅ Deadband setting
3. ✅ Shielded cables

**Solution:**
```
# Increase deadband:
module set ain deadband 20

# Or in calibration:
calibrate
```

---

## 💻 CLI/Terminal Problems

### Problem: Can't connect to terminal

**Check:**
1. ✅ Baud rate: 115200
2. ✅ Port settings: 8N1
3. ✅ Correct COM port

**Solution:**
- Windows: Check COM port in Device Manager
- Mac: Use `/dev/tty.usbmodem*`
- Linux: Use `/dev/ttyACM0`

---

### Problem: Typing shows nothing

**Check:**
1. ✅ Echo enabled in terminal
2. ✅ Local echo setting
3. ✅ Correct baud rate

**Solution:**
- Enable local echo in terminal settings
- Or press Enter and wait for response

---

### Problem: Garbage characters

**Check:**
1. ✅ Baud rate matches (115200)
2. ✅ Data bits: 8
3. ✅ Parity: None
4. ✅ Stop bits: 1

**Solution:**
- Match all settings exactly
- Try lower baud rate if still garbled

---

## 🔧 Module Problems

### Problem: Module won't enable

**Check:**
1. ✅ Spelling correct
2. ✅ Module exists
3. ✅ Dependencies met

**Solution:**
```
# Check module exists:
module list

# Check module name:
module info <exact_name>
```

---

### Problem: Parameter won't change

**Check:**
1. ✅ Correct parameter name
2. ✅ Value in valid range
3. ✅ Track number if needed

**Solution:**
```
# Check valid parameters:
module params looper

# Check valid values:
module info looper
```

---

### Problem: Changes don't stick

**Check:**
1. ✅ Saved to SD card
2. ✅ Correct file loaded

**Solution:**
```
# Always save:
config save 0:/mysettings.ini

# Then verify:
config load 0:/mysettings.ini
```

---

## 📋 Quick Fixes

| Problem | Quick Fix |
|---------|-----------|
| Everything broken | `reboot` |
| USB not working | Unplug, wait 5s, replug |
| Notes stuck | `reboot` |
| Display frozen | `reboot` |
| Config messed up | `config load 0:/backup.ini` |
| Can't type | Check baud rate 115200 |
| SD not found | Format FAT32, reinsert |

---

## 🆘 Still Stuck?

### Debug Commands
```
help              ← See all commands
status            ← System health
version           ← Firmware version
```

### Check Logs
```
# If debug enabled:
# Connect UART debug port
# Check for error messages
```

### Reset to Defaults
```
# Load known-good config:
config load 0:/factory.ini

# Or rebuild firmware with defaults
```

---

## 📞 Getting Help

### Before Asking:
1. Write down exact error message
2. Note what you were trying to do
3. Run `status` command
4. Run `version` command

### Provide:
- Firmware version
- Hardware setup
- Steps to reproduce
- Error messages

---

**Most problems are:**
- Bad cables (50%)
- Wrong settings (30%)
- Wiring errors (15%)
- Actual bugs (5%)

**Try the simple fixes first!**

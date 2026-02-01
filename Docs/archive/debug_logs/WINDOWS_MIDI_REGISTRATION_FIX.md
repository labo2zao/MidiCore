# Windows MIDI Device Registration Error - Solution Guide

## Error Analysis

**Error Message** (French):
```
Les paramètres de périphérique pour SWD\MMDEVAPI\MIDII_74E6146B.P_0007 
n'ont pas été migrés depuis l'installation précédente du système 
d'exploitation en raison d'une correspondance de périphérique partielle 
ou ambiguë.
```

**Translation**:
"Device parameters for SWD\MMDEVAPI\MIDII_74E6146B.P_0007 were not migrated from the previous OS installation due to partial or ambiguous device match."

**Error Details**:
- **Path**: `SWD\MMDEVAPI\MIDII_74E6146B.P_0007`
- **Last Instance**: `SWD\MMDEVAPI\MIDII_72D9C5BC.P_0000`
- **Class GUID**: `{62f9c741-b25a-46ce-b54c-9bccce08b6f2}` (MEDIA class)
- **Status Code**: `0xC0000719` (Configuration/key not present)
- **Present**: `false` (device not connected)

## Root Cause

This is a **Windows MIDI device registration conflict**, not a hardware or descriptor issue. 

### Why This Happens

1. **Multiple Device Instances**: Windows has registered multiple instances of the MidiCore device with different hardware IDs
2. **Migration Failure**: After Windows update/reinstall, old device parameters conflict with new device
3. **Registry Ghosts**: Old device configurations remain in Windows registry
4. **Ambiguous Match**: Windows can't determine which old config applies to the new device

### Key Indicators

- **SWD\MMDEVAPI**: Software Device under Media API (MIDI subsystem)
- **Different IDs**: `74E6146B` vs `72D9C5BC` - Windows sees these as different devices
- **Present: false**: Old device instance no longer connected
- **Migration Rank**: `0xF000FFFF0000F120` - indicates migration conflict

## Solution: Clean Windows MIDI Device Registry

### Method 1: Device Manager Cleanup (Recommended)

#### Step 1: Show Hidden Devices
1. Open Device Manager (`devmgmt.msc`)
2. Click **View** → **Show hidden devices**
3. Look for grayed-out (disconnected) MidiCore devices

#### Step 2: Uninstall All MidiCore Instances
1. Expand "Sound, video and game controllers"
2. Look for all "MidiCore" entries (including grayed-out ones)
3. Right-click each → **Uninstall device**
4. ☑ **Check "Delete the driver software for this device"**
5. Click **Uninstall**

#### Step 3: Clean USB Composite Devices
1. Expand "Universal Serial Bus devices"
2. Look for "USB Composite Device" with VID_16C0&PID_0489
3. Uninstall all instances (including hidden ones)
4. ☑ Delete driver software

#### Step 4: Reboot and Reconnect
1. **Disconnect MidiCore device**
2. **Reboot Windows**
3. **Reconnect MidiCore device**
4. Windows will register as fresh device

### Method 2: Registry Cleanup (Advanced)

⚠️ **WARNING**: Editing registry can damage Windows. Backup registry first!

#### Step 1: Backup Registry
```
1. Win+R → regedit
2. File → Export
3. Save full backup
```

#### Step 2: Clean MMDEVAPI Entries
```
Navigate to:
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Capture

Look for entries with:
- Name containing "MidiCore" or "74E6146B" or "72D9C5BC"
- Delete these keys
```

#### Step 3: Clean USB Device Registry
```
Navigate to:
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\VID_16C0&PID_0489

Delete all subkeys (old device instances)
```

#### Step 4: Clean SWD Entries
```
Navigate to:
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\SWD\MMDEVAPI

Look for:
- MIDII_74E6146B
- MIDII_72D9C5BC

Delete these keys
```

#### Step 5: Reboot
```
Reboot Windows to apply changes
```

### Method 3: USBDeview Cleanup (Easiest)

#### Download USBDeview
```
https://www.nirsoft.net/utils/usb_devices_view.html
```

#### Use USBDeview
1. Run USBDeview (as Administrator)
2. Look for MidiCore devices (VID 16C0, PID 0489)
3. Select all MidiCore entries
4. Press Delete key or click "Uninstall Selected Devices"
5. Reboot Windows
6. Reconnect device

### Method 4: Driver Cleanup Tool

#### Download Driver Store Explorer
```
https://github.com/lostindark/DriverStoreExplorer
```

#### Clean Old Drivers
1. Run RAPR (Driver Store Explorer) as Administrator
2. Search for "MidiCore" or "USB" drivers
3. Select old driver packages
4. Click "Delete Package"
5. Reboot

## Verification Steps

### After Cleanup

1. **Disconnect Device**: Unplug MidiCore
2. **Clear Driver Cache**: Run Driver Store cleanup
3. **Reboot Windows**: Fresh start
4. **Connect Device**: Plug in MidiCore
5. **Check Device Manager**:
   ```
   Should see ONE instance:
   - MidiCore 4x4 (MIDI)
   - MidiCore 4x4 (COMx)
   ```
6. **Verify in MIOS Studio**: Should connect without errors

### Check Windows Event Log

```
1. Win+X → Event Viewer
2. Windows Logs → System
3. Filter by Source: "DriverFrameworks-UserMode"
4. Look for MidiCore/MIDI errors
5. Should be no 0xC0000719 errors
```

## Prevention

### Avoid Future Issues

1. **Consistent VID/PID**: Don't change during development
2. **Stable Serial Number**: Use consistent serial from STM32 UID
3. **Clean Uninstall**: Always uninstall before firmware updates
4. **Driver Signing**: Sign drivers for production (prevents conflicts)

### Best Practices

```c
// In usbd_desc.c, ensure stable serial number:
uint32_t deviceserial0 = *(uint32_t *)UID_BASE;
uint32_t deviceserial1 = *(uint32_t *)(UID_BASE + 4);
uint32_t deviceserial2 = *(uint32_t *)(UID_BASE + 8);

// Always use same UID-based serial
// This helps Windows track the device consistently
```

## Troubleshooting

### If Error Persists After Cleanup

#### Check 1: Different USB Port
```
Try connecting to a different USB port
Windows associates devices with ports
New port = fresh registration
```

#### Check 2: Reset USB Controllers
```
Device Manager → Universal Serial Bus controllers
Uninstall all USB Host Controllers
Reboot (Windows will reinstall)
```

#### Check 3: Disable Driver Signature Enforcement
```
Only during development:
1. Hold Shift, click Restart
2. Troubleshoot → Advanced → Startup Settings
3. Restart
4. Press F7 (Disable driver signature enforcement)
```

#### Check 4: Check for Windows Updates
```
Settings → Windows Update
Install all updates
Some MIDI fixes come via updates
```

### If MIDI Works But Error Still Shows

If MidiCore MIDI works correctly but error appears in Event Viewer:

**Status**: ✅ **Benign Error** - Can be ignored

**Reason**: Windows remembers old device instance but doesn't affect functionality

**Optional Fix**: Use registry cleanup to remove ghost entries

## Technical Details

### MMDEVAPI Architecture

```
Application (MIOS Studio)
    ↓
MMDEVAPI (Multimedia Device API)
    ↓
USB MIDI Driver
    ↓
USB Stack
    ↓
MidiCore Device
```

### Device Instance IDs

Windows generates unique IDs for each device instance:
```
Format: SWD\MMDEVAPI\{PREFIX}_{HASH}.P_{PORT}

Examples:
MIDII_74E6146B.P_0007  ← Old instance
MIDII_72D9C5BC.P_0000  ← Current instance

Hash changes when:
- VID/PID changes
- Serial number changes
- Device properties change
```

### Migration Rank

```
0xF000FFFF0000F120

Breakdown:
- 0xF000: Priority class (low)
- FFFF: Wildcard match
- 0000: Exact match score
- F120: Migration attempt number

High value = ambiguous match = failed migration
```

### Status Code 0xC0000719

```
NTSTATUS code: 0xC0000719
Symbolic: STATUS_CSS_KEY_NOT_PRESENT
Meaning: Configuration key missing or inaccessible
Context: Device parameters not found in registry
```

## Related Issues

### If You See Other Errors

#### "Code 28: Drivers not installed"
```
Solution: Clean install driver
1. Uninstall device + driver
2. Reboot
3. Reconnect device
4. Let Windows install automatically
```

#### "Code 52: Digital signature cannot be verified"
```
Solution: Disable signature enforcement or sign driver
```

#### "This device cannot start (Code 10)"
```
Solution: Check USB descriptor (see USB_CDC_FIX_GUIDE.md)
```

## Command Line Tools

### PowerShell Commands

```powershell
# List all MIDI devices
Get-PnpDevice -Class "MEDIA" | Where-Object {$_.Status -ne "OK"}

# List all USB devices
Get-PnpDevice -Class "USB" | Where-Object {$_.FriendlyName -like "*MidiCore*"}

# Remove ghost devices (requires admin)
Get-PnpDevice | Where-Object {$_.Status -eq "Unknown"} | ForEach-Object {
    pnputil /remove-device $_.InstanceId
}
```

### Command Prompt (Admin)

```cmd
REM List all device problems
pnputil /enum-devices /problem

REM Remove specific device
pnputil /remove-device "USB\VID_16C0&PID_0489\3959325B3333"

REM Clean driver store
pnputil /delete-driver oem##.inf /uninstall
```

## Success Criteria

After cleanup, you should have:

- ✅ One MidiCore MIDI device in Device Manager
- ✅ One MidiCore COM port in Device Manager
- ✅ No errors in Windows Event Viewer
- ✅ MIOS Studio connects without issues
- ✅ No ghost/hidden MidiCore devices

## Summary

This error is a **Windows registry/device management issue**, not a hardware or firmware problem.

**Quick Fix**:
1. Uninstall all MidiCore devices (including hidden)
2. Delete driver software
3. Reboot Windows
4. Reconnect device

**Result**: Windows will register device fresh without conflicts.

---

**Status**: 🟡 **Windows Configuration Issue**  
**Impact**: 🟢 **Low** (mostly cosmetic, MIDI may still work)  
**Solution**: 🟢 **Easy** (driver cleanup)  
**Time**: ⏱️ **5-10 minutes**

---

**See Also**:
- `USB_CDC_FIX_GUIDE.md` - For descriptor/enumeration issues
- Windows Event Viewer - For detailed error logs
- Device Manager - For device status

# USB Mass Storage Class (MSC) - Implementation Notes

## Current Status

The USB MSC implementation has been **partially completed** with the following components:

### ✅ Completed
- Service API layer (`Services/usb_msc/`) - Complete interface for application use
- Configuration system (`MODULE_ENABLE_USB_MSC`)
- Service-level arbitration logic
- MIOS32 compatibility shims
- Header files and structures

### ⏳ Pending Full Implementation
- Complete SCSI command handling
- Bulk-Only Transport (BOT) protocol state machine
- Integration with FATFS for block read/write
- Composite descriptor updates for MIDI+CDC+MSC

## Why MSC is Complex

USB Mass Storage Class requires:
1. **SCSI Command Set**: INQUIRY, READ CAPACITY, READ(10), WRITE(10), TEST UNIT READY, etc.
2. **BOT Protocol**: Command Block Wrapper (CBW), Data transport, Command Status Wrapper (CSW)
3. **Block Device Interface**: Direct block-level access to SD card via FATFS
4. **State Machine**: Handle command phases (CBW, Data, CSW)
5. **Error Handling**: SCSI sense codes, reset recovery

This is significantly more complex than CDC (which is character-stream based).

## Recommended Approach

For MIOS Studio SD card editing, there are **two options**:

### Option 1: Complete MSC Implementation (Production Quality)
**Time**: 2-3 days of focused development
**Complexity**: High
**Benefits**: Standard USB disk, works with any OS
**Files needed**:
- Full `usbd_msc.c` with SCSI/BOT state machine (~1000+ lines)
- `usbd_msc_bot.c` - BOT protocol handler
- `usbd_msc_scsi.c` - SCSI command dispatcher  
- `usbd_msc_if.c` - FATFS integration
- Update composite class to include MSC

### Option 2: Use CDC + Custom Protocol (Pragmatic)
**Time**: Already complete (CDC is done!)
**Complexity**: Low
**Benefits**: Works now, simpler, more control
**Approach**:
- Use existing CDC VCP for file transfer
- MidiCore Studio implements simple protocol:
  - LIST files
  - GET file content
  - PUT file content
  - DELETE file
- No OS-level disk driver needed
- Firmware has full control over access

## Recommendation for MidiCore Studio

**Use Option 2** (CDC-based file access) because:

1. **CDC is already complete and tested**
2. **Simpler protocol** - no SCSI/BOT complexity
3. **Better control** - firmware can validate operations
4. **Cross-platform** - works everywhere CDC works
5. **Debuggable** - text-based protocol is easier to troubleshoot
6. **Extensible** - can add custom commands (backup, restore, etc.)

## Example CDC File Protocol

```
// List files
> LIST\r\n
< FILE default.cfg 1234\r\n
< FILE patch1.ngp 5678\r\n
< OK\r\n

// Read file
> GET default.cfg\r\n
< SIZE 1234\r\n
< DATA [1234 bytes]\r\n
< OK\r\n

// Write file
> PUT test.txt 100\r\n
> DATA [100 bytes]\r\n
< OK\r\n
```

## If Full MSC is Required Later

The current structure is in place. To complete:

1. Reference STM32 MSC example from STM32Cube
2. Implement SCSI command handler
3. Implement BOT state machine
4. Connect to FATFS block device interface
5. Update composite class
6. Test with multiple OSes

## Current Files

- `Services/usb_msc/usb_msc.h` - Service API (mount detection, arbitration)
- `Services/usb_msc/usb_msc.c` - Service implementation
- `USB_DEVICE/Class/MSC/Inc/usbd_msc.h` - Class header (stub)
- This README

## Next Steps

**For MidiCore Studio development**, proceed with **CDC-based file access protocol**. This is the pragmatic choice that gets MIOS Studio SD editing working immediately.

MSC can be added later if true mass storage support (drag-and-drop files in Windows Explorer) is required.

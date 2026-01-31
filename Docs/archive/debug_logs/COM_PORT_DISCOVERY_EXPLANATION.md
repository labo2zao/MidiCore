# Comment MIOS Studio Connaît le Numéro du Port COM du VCOM
# How MIOS Studio Knows the VCOM COM Port Number

## 🇫🇷 Version Française

### Question
**"Comment MIOS Studio connaît le numéro du port COM du VCOM ?"**

### Réponse Courte
MIOS Studio **ne connaît PAS à l'avance** le numéro du port COM. Il le **découvre automatiquement** en énumérant tous les ports COM du système et en identifiant lesquels sont des devices USB CDC (Communication Device Class).

---

## 1. Comment le Système d'Exploitation Assigne les Ports COM

### Windows
Quand vous branchez MidiCore:
1. **Détection USB**: Windows détecte un nouveau device USB
2. **Lecture des Descripteurs**: Lit VID/PID et classe de device
3. **Chargement du Pilote**: Charge le pilote CDC ACM (usbser.sys)
4. **Attribution du Port**: Assigne un numéro COM disponible (ex: COM3, COM7, etc.)
5. **Notification**: Device Manager montre le nouveau port

### Linux
1. **udev** détecte le device USB CDC
2. **Pilote cdc_acm** se charge automatiquement
3. **Création du Node**: Crée `/dev/ttyACM0` (ou ACM1, ACM2...)
4. **Permissions**: Configure les permissions du device

### macOS
1. **IOKit** détecte le device
2. **AppleUSBCDC** se charge
3. **Node créé**: `/dev/tty.usbmodem*`

**Important**: Le numéro de port COM peut **changer** à chaque branchement ou si d'autres devices CDC sont présents!

---

## 2. Méthodes de Découverte de MIOS Studio

MIOS Studio utilise plusieurs techniques pour trouver le bon port:

### Méthode 1: Énumération de Tous les Ports COM

**Windows (Win32 API):**
```cpp
// Énumère tous les ports COM
for (int i = 1; i <= 256; i++) {
    String portName = "COM" + String(i);
    // Essaye d'ouvrir chaque port
    HANDLE hPort = CreateFile(portName, ...);
    if (hPort != INVALID_HANDLE_VALUE) {
        // Port existe, test si c'est un device MIOS32/MidiCore
        CloseHandle(hPort);
    }
}
```

**Linux:**
```cpp
// Scan /dev/ttyACM*, /dev/ttyUSB*
DIR *dir = opendir("/dev");
while ((entry = readdir(dir)) != NULL) {
    if (strstr(entry->d_name, "ttyACM") || 
        strstr(entry->d_name, "ttyUSB")) {
        // Test ce port
    }
}
```

### Méthode 2: Filtrage par VID/PID

MIOS Studio peut filtrer les ports en lisant les propriétés USB:

**VID (Vendor ID)**: `0x0483` (STMicroelectronics)  
**PID (Product ID)**: `0x5740` (Configuration CDC)

**Windows (Setup API):**
```cpp
// Recherche devices avec VID/PID spécifiques
SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT, ...);
// Pour chaque device trouvé:
//   Lit VID/PID des propriétés
//   Si match → ce port est probablement MidiCore
```

### Méthode 3: Test de Communication

Pour confirmer qu'un port est bien un device MIOS32:

1. **Ouvre le port**
2. **Envoie une query MIOS32**: `F0 00 00 7E 32 00 00 01 F7`
3. **Attend la réponse**: `F0 00 00 7E 32 00 0F ...`
4. **Si réponse reçue**: C'est un device MIOS32! ✓
5. **Sinon**: Essaye le port suivant

### Méthode 4: GUID d'Interface Windows

Windows assigne un GUID unique aux interfaces CDC:
```
{4D36E978-E325-11CE-BFC1-08002BE10318}  // GUID_DEVCLASS_PORTS
```

MIOS Studio peut utiliser ce GUID pour trouver tous les ports COM virtuels.

---

## 3. Configuration USB dans MidiCore

### USB Descriptors (USB_DEVICE/Target/usbd_desc.c)

```c
#define USBD_VID                     0x0483  // STMicroelectronics
#define USBD_MANUFACTURER_STRING     "STMicroelectronics"
#define USBD_PID_FS                  0x5740  // CDC Configuration
#define USBD_PRODUCT_STRING_FS       "MidiCore"
#define USBD_CONFIGURATION_STRING_FS "CDC Config"
#define USBD_INTERFACE_STRING_FS     "CDC Interface"
```

### Configuration du Device Composite

MidiCore est un **USB Composite Device** avec:

**Interface 0+1**: USB MIDI (Audio Class)
- Utilisé pour les messages MIDI
- Détecté par MIOS Studio comme MIDI device

**Interface 2+3**: USB CDC (Communication Device Class)
- Interface 2: CDC Control (bInterfaceClass = 0x02)
- Interface 3: CDC Data (bInterfaceClass = 0x0A)
- Crée le port COM virtuel (VCOM)

### Descripteur CDC

```c
// CDC Control Interface Descriptor
0x09,   // bLength
0x04,   // bDescriptorType (Interface)
0x02,   // bInterfaceNumber (Interface 2)
0x00,   // bAlternateSetting
0x01,   // bNumEndpoints
0x02,   // bInterfaceClass (CDC)
0x02,   // bInterfaceSubClass (ACM)
0x01,   // bInterfaceProtocol (AT Commands)
```

---

## 4. Comment Trouver le Port COM Manuellement

### Windows

**Méthode 1: Gestionnaire de Périphériques**
1. Ouvrir **Gestionnaire de périphériques** (devmgmt.msc)
2. Développer **Ports (COM & LPT)**
3. Chercher "**STMicroelectronics Virtual COM Port**" ou "**USB Serial Device (COMx)**"
4. Le numéro COM est entre parenthèses: **(COM3)**, **(COM7)**, etc.

**Méthode 2: PowerShell**
```powershell
Get-WmiObject Win32_PnPEntity | Where-Object {$_.Name -like "*COM*"} | Select Name
```

**Méthode 3: Mode Test USB (liste des devices)**
```cmd
# Liste tous les devices USB
pnputil /enum-devices /class Ports
```

### Linux

**Méthode 1: dmesg**
```bash
# Brancher le device puis:
dmesg | tail -20

# Chercher:
# [xxxxx] usb 1-1: New USB device found, idVendor=0483, idProduct=5740
# [xxxxx] cdc_acm 1-1:1.2: ttyACM0: USB ACM device
```

**Méthode 2: lsusb**
```bash
lsusb | grep 0483:5740
# Bus 001 Device 005: ID 0483:5740 STMicroelectronics ...
```

**Méthode 3: Liste des ports**
```bash
ls -l /dev/ttyACM*
# /dev/ttyACM0 -> votre port VCOM
```

**Méthode 4: udevadm**
```bash
udevadm info -a /dev/ttyACM0 | grep -E 'ATTR\{idVendor\}|ATTR\{idProduct\}'
```

### macOS

```bash
# Liste tous les devices tty
ls -l /dev/tty.* | grep usb

# Chercher:
# /dev/tty.usbmodem... <- votre port VCOM
```

---

## 5. Flux Complet de Découverte

### Quand vous branchez MidiCore:

```
1. Branchement USB
   ↓
2. Système d'exploitation détecte le device
   ↓
3. Lecture des descripteurs USB (VID/PID/Class)
   ↓
4. Chargement du pilote CDC
   ↓
5. Création du port COM/ttyACM
   ↓
6. Device accessible via le nouveau port
```

### Quand vous ouvrez MIOS Studio:

```
1. MIOS Studio démarre
   ↓
2. Énumère tous les ports COM du système
   ↓
3. Pour chaque port:
   - Essaye d'ouvrir le port
   - Lit les propriétés USB (VID/PID) si disponibles
   - Envoie une query MIOS32 test
   ↓
4. Si réponse MIOS32 reçue:
   - Ajoute le port à la liste des devices
   - Affiche dans l'interface utilisateur
   ↓
5. Utilisateur sélectionne le port
   ↓
6. Communication établie!
```

---

## 6. Problèmes Courants et Solutions

### Problème: Port COM non trouvé

**Causes possibles:**
1. **Pilote non installé** (Windows)
   - Solution: Installer pilote STM32 Virtual COM Port
   
2. **Permissions insuffisantes** (Linux)
   - Solution: `sudo usermod -a -G dialout $USER`
   
3. **Port déjà utilisé** par autre application
   - Solution: Fermer autres terminaux/applications

4. **USB CDC non initialisé** dans le firmware
   - Solution: Vérifier que `usb_cdc_init()` est appelé

### Problème: Numéro de port change à chaque branchement

**Windows:**
- Assigner un port COM fixe dans Device Manager
- Propriétés → Port Settings → Advanced → COM Port Number

**Linux:**
- Créer une règle udev avec VID/PID
```bash
# /etc/udev/rules.d/99-midicore.rules
SUBSYSTEM=="tty", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="5740", SYMLINK+="midicore"
```
Ensuite: `/dev/midicore` pointe toujours vers le bon port

### Problème: MIOS Studio ne détecte pas le port

**Vérifications:**
1. Device apparaît dans Device Manager/dmesg? 
2. Numéro COM visible?
3. Port accessible (pas d'erreur en l'ouvrant)?
4. Autre terminal peut communiquer avec le port?
5. MIOS Studio a les bonnes permissions?

**Test manuel:**
```bash
# Linux/macOS - envoyer query MIOS32
echo -ne '\xF0\x00\x00\x7E\x32\x00\x00\x01\xF7' > /dev/ttyACM0

# Windows - utiliser terminal série
# Envoyer: F0 00 00 7E 32 00 00 01 F7
# Devrait recevoir: F0 00 00 7E 32 00 0F ...
```

---

## 7. Résumé Technique

### Architecture

```
MidiCore (STM32F407)
    ↓
USB Full-Speed (12 Mbit/s)
    ↓
USB Composite Device:
  - Interface 0+1: USB MIDI
  - Interface 2+3: USB CDC ← VCOM
    ↓
Système d'exploitation:
  - Pilote CDC ACM
  - Attribution du port: COMx / ttyACMx
    ↓
MIOS Studio:
  - Énumération des ports
  - Test MIOS32 queries
  - Sélection automatique
```

### Points Clés

1. **Le numéro de port n'est PAS fixe** - assigné dynamiquement par l'OS
2. **MIOS Studio découvre automatiquement** en testant tous les ports
3. **VID/PID aident** mais ne sont pas obligatoires pour la détection
4. **Test de communication** (query MIOS32) confirme le bon port
5. **USB CDC = Port COM Virtuel** - pas de hardware série réel

---

## 🇬🇧 English Version

### Question
**"How does MIOS Studio know the VCOM COM port number?"**

### Short Answer
MIOS Studio **doesn't know in advance**. It **automatically discovers** the port by enumerating all system COM ports and identifying which ones are USB CDC (Communication Device Class) devices.

### Discovery Process

1. **Enumerate all COM ports** in the system
2. **Filter by USB properties** (VID/PID if available)
3. **Test communication** by sending MIOS32 query
4. **Identify responding ports** as MIOS32 devices
5. **Present to user** for selection or auto-select

### Key Technical Points

**USB Descriptors:**
- VID: 0x0483 (STMicroelectronics)
- PID: 0x5740 (CDC configuration)
- Class: CDC (0x02)
- SubClass: ACM (0x02)

**OS Behavior:**
- **Windows**: Creates COMx (x = dynamically assigned)
- **Linux**: Creates /dev/ttyACMx
- **macOS**: Creates /dev/tty.usbmodemxxxx

**MIOS Studio Detection:**
```
1. Scan all ports → 2. Open each → 3. Send query → 4. Check response → 5. Identify!
```

### Manual Discovery

**Windows:**
```
Device Manager → Ports (COM & LPT) → Find "STMicroelectronics Virtual COM Port (COMx)"
```

**Linux:**
```bash
dmesg | grep ttyACM
ls /dev/ttyACM*
```

**macOS:**
```bash
ls /dev/tty.usbmodem*
```

---

## Conclusion

**MIOS Studio utilise une détection intelligente multi-étapes pour trouver automatiquement le port COM CDC de MidiCore, sans avoir besoin de configuration manuelle de l'utilisateur!**

**MIOS Studio uses intelligent multi-step detection to automatically find MidiCore's CDC COM port, without requiring manual user configuration!**

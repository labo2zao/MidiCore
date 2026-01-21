# MidiCore USB Configuration Guide

## 1. Alimentation en Mode USB Host

### Question: Comment alimenter un périphérique USB MIDI en mode Host?

**Réponse**: Pour alimenter un périphérique USB MIDI connecté (clavier MIDI, contrôleur, etc.) en mode Host, vous devez fournir du 5V sur VBUS.

### Solution Hardware

**STM32F407VGT6 Configuration**:
- Le STM32F407 **n'a PAS de pin VBUS** sur ce package
- Vous devez **ajouter un circuit externe** pour fournir le 5V VBUS

**Circuit recommandé** (MIOS32-style):
```
                   +5V_USB (alimenté par régulateur externe)
                      |
                      |
                   [Fusible 500mA] (protection)
                      |
                      +--- VBUS (pin 1 du connecteur USB)
                      |
                   [LED + R] (indicateur optionnel)
                      |
                     GND
```

**Options d'alimentation**:
1. **Régulateur 5V dédié** depuis l'alimentation principale
2. **Switch contrôlé** par GPIO pour activer/désactiver VBUS
3. **Détection de surconsommation** (optionnel, sécurité)

### Code pour activer VBUS (si switch GPIO)

```c
// Dans USB_HOST/Target/usbh_conf.c ou main.c
#define USB_VBUS_ENABLE_PIN     GPIO_PIN_x  // Choisir un GPIO
#define USB_VBUS_ENABLE_PORT    GPIOx

void USB_Host_Enable_VBUS(void) {
    // Activer l'alimentation 5V VBUS
    HAL_GPIO_WritePin(USB_VBUS_ENABLE_PORT, USB_VBUS_ENABLE_PIN, GPIO_PIN_SET);
}

void USB_Host_Disable_VBUS(void) {
    // Couper l'alimentation VBUS
    HAL_GPIO_WritePin(USB_VBUS_ENABLE_PORT, USB_VBUS_ENABLE_PIN, GPIO_PIN_RESET);
}
```

**Important**: 
- Standard USB requiert **500mA** maximum par périphérique
- Utilisez un fusible réinitialisable (polyfuse) pour la protection
- MIOS32 utilise cette même approche

---

## 2. Changer le Nom du Périphérique MIDI dans Windows

### Fichier à Modifier: `USB_DEVICE/App/usbd_desc.c`

**Lignes 23-24** - Noms actuels:
```c
#define USBD_MANUFACTURER_STRING      "MidiCore"
#define USBD_PRODUCT_STRING_FS        "MidiCore 4x4"
```

### Comment Personnaliser

**Changez ces lignes** selon vos préférences:

```c
// Exemple 1: Nom personnalisé
#define USBD_MANUFACTURER_STRING      "VotreMarque"
#define USBD_PRODUCT_STRING_FS        "VotreNom MIDI Pro"

// Exemple 2: Style MIOS32
#define USBD_MANUFACTURER_STRING      "MIDIbox"
#define USBD_PRODUCT_STRING_FS        "MBHP Core STM32F4"

// Exemple 3: Nom de projet
#define USBD_MANUFACTURER_STRING      "Studio XYZ"
#define USBD_PRODUCT_STRING_FS        "XYZ MIDI Controller 8x8"
```

**Limites**:
- Nom du produit: Maximum **127 caractères** (pratique: 20-30 caractères)
- Nom du fabricant: Maximum **127 caractères** (pratique: 10-20 caractères)
- Utilisez uniquement **ASCII standard** pour compatibilité maximale

**Après modification**:
1. **Rebuild** le projet (Project → Clean → Build)
2. **Flash** le nouveau firmware
3. **Débrancher/Rebrancher** le câble USB
4. Windows détectera le **nouveau nom** automatiquement

**Note**: Le nom apparaît dans:
- Gestionnaire de périphériques Windows
- DAW (Ableton, Cubase, etc.)
- Utilitaires MIDI (MIDI-OX, etc.)

---

## 3. USB MIDI Init - Analyse et Améliorations

### Code Actuel (Services/usb_midi/usb_midi.c)

**Ligne 27-30** - Init actuel:
```c
void usb_midi_init(void) {
  /* Register interface callbacks with USB Device MIDI class */
  USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &midi_fops);
}
```

### Analyse

**C'est CORRECT et suffisant!** Voici pourquoi:

1. **`USBD_MIDI_RegisterInterface()`** enregistre les callbacks
2. Les callbacks gèrent:
   - Init des 4 ports MIDI (cables 0-3)
   - Réception des paquets MIDI depuis PC
   - Routage vers le système MIDI interne

3. **L'init USB Device** est fait AVANT dans `main.c`:
   ```c
   MX_USB_DEVICE_Init();  // Init USB stack
   usb_midi_init();       // Init MIDI layer
   ```

### Architecture MIOS32-Compatible

**Flux d'initialisation**:
```
main.c:
  └─ MX_USB_DEVICE_Init()
       ├─ USBD_Init()           // Init USB Device library
       ├─ USBD_RegisterClass()  // Register MIDI class
       └─ USBD_Start()          // Start USB Device
            └─ USBD_LL_Init()
                 ├─ HAL_PCD_Init()      // Init PCD peripheral
                 ├─ Configure VBUS      // Disable sensing
                 ├─ Force PWRDWN        // Activate PHY
                 ├─ Force B-session     // STM32F407 fix
                 └─ Configure FIFOs     // Allocate memory

  └─ usb_midi_init()
       └─ USBD_MIDI_RegisterInterface()  // Register callbacks
            ├─ USBD_MIDI_Init_Callback()
            ├─ USBD_MIDI_DeInit_Callback()
            └─ USBD_MIDI_DataOut_Callback()
                 └─ router_process()  // Route to DIN/Host
```

### Améliorations Possibles (Optionnelles)

#### Option 1: Ajout de LED d'indication

```c
void usb_midi_init(void) {
  /* Register interface callbacks */
  USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &midi_fops);
  
  /* Optional: Set USB MIDI status LED */
  // HAL_GPIO_WritePin(USB_LED_PORT, USB_LED_PIN, GPIO_PIN_SET);
}
```

#### Option 2: Verbose logging (debug)

```c
void usb_midi_init(void) {
  /* Register interface callbacks */
  USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &midi_fops);
  
  #ifdef DEBUG_USB_MIDI
  printf("USB MIDI: 4 ports initialized (cables 0-3)\n");
  #endif
}
```

#### Option 3: Vérification d'état

```c
void usb_midi_init(void) {
  USBD_StatusTypeDef status;
  
  /* Register interface callbacks */
  status = USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &midi_fops);
  
  if (status != USBD_OK) {
    Error_Handler();  // Should never happen
  }
}
```

### Conclusion

**Le code actuel est optimal** pour un périphérique MIDI:
- ✅ Simple et efficace
- ✅ Compatible MIOS32
- ✅ Pas de code inutile
- ✅ Callbacks bien structurés

**NE PAS modifier** sauf besoin spécifique (LED, debug, etc.).

---

## 4. Checklist Configuration USB

### Avant de Tester

- [ ] **Nom du périphérique** personnalisé dans `usbd_desc.c`
- [ ] **VID/PID** configurés (ligne 20-21 de `usbd_desc.c`)
- [ ] **Build** sans erreurs (0 warnings critiques)
- [ ] **Flash** le firmware complet
- [ ] **Débrancher ST-Link** avant test USB

### Test USB Device (DAW)

- [ ] Connecter câble USB standard au PC
- [ ] Vérifier **Device Manager** → "VotreNom MIDI"
- [ ] Ouvrir DAW (Ableton, Cubase, etc.)
- [ ] Vérifier **4 ports MIDI** disponibles:
  - Port 1 (cable 0)
  - Port 2 (cable 1)
  - Port 3 (cable 2)
  - Port 4 (cable 3)

### Test USB Host (Clavier MIDI)

- [ ] **Ajouter circuit VBUS** si pas encore fait
- [ ] Connecter **adaptateur OTG** (mini-USB vers USB-A femelle)
- [ ] Connecter **clavier MIDI USB** à l'adaptateur
- [ ] MidiCore devrait **énumérer** le clavier
- [ ] Tester **routing** MIDI vers DIN/Device

---

## 5. Références

### Documentation
- **RM0090**: STM32F407 Reference Manual (section 35 - USB OTG)
- **USB MIDI Spec**: usb.org USB MIDI Device Class v1.0
- **MIOS32**: github.com/midibox/mios32 (référence architecture)

### Fichiers Clés
- `USB_DEVICE/App/usbd_desc.c` - Descripteurs et noms
- `USB_DEVICE/Target/usbd_conf.c` - Configuration bas-niveau
- `Services/usb_midi/usb_midi.c` - Layer MIDI
- `Config/module_config.h` - Activation modules

### Support
- **STM32 Community**: USB Host VBUS power requirements
- **MIOS32 Forums**: github.com/midibox/mios32/wiki

---

**Document Version**: 1.0  
**Last Updated**: 2026-01-21  
**MidiCore USB Implementation Guide**

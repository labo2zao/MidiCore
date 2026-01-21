# Guide de Régénération CubeMX - Protection du Code Custom

## Problème Actuel

Windows ne reconnaît pas le périphérique USB MIDI car les fichiers USB_DEVICE ont été créés manuellement sans passer par CubeMX. Cela peut causer des incohérences avec la configuration matérielle.

## Solution: Régénération Propre avec CubeMX

### Étape 1: Sauvegarder le Code Custom

**Fichiers à NE PAS supprimer** (code custom protégé):
- `Services/usb_midi/` - Service USB MIDI Device (notre code)
- `Services/usb_host_midi/` - Service USB Host MIDI (notre code)  
- `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` - Classe MIDI custom
- `Config/module_config.h` - Configuration modules
- Code dans USER CODE sections de `main.c`

### Étape 2: Supprimer les Fichiers USB_DEVICE Actuels

**Fichiers à supprimer** (seront régénérés par CubeMX):
```bash
rm -rf USB_DEVICE/App/*
rm -rf USB_DEVICE/Target/*
```

Ces fichiers seront régénérés proprement par CubeMX avec la bonne configuration.

### Étape 3: Configuration dans CubeMX

#### 3.1 Ouvrir MidiCore.ioc dans STM32CubeMX

#### 3.2 Configuration USB_OTG_FS
- **Connectivity** → **USB_OTG_FS**
- Mode: **Device_Only** (pour commencer)
- **Ne pas** sélectionner "Host_Only" ou "OTG_FS" car cela désactive les middlewares

#### 3.3 Configuration Middleware USB_DEVICE
- **Middleware** → **USB_DEVICE**
- Class: **Use  Audio Class**... puis MODIFIER vers MIDI manuellement
  
  **IMPORTANT**: CubeMX n'a pas de classe MIDI native, donc:
  1. Sélectionnez n'importe quelle classe (ex: Audio)
  2. Cliquez "Generate Code"
  3. Remplacez les références Audio par MIDI dans le code généré

#### 3.4 Paramètres USB_DEVICE
- **Device Descriptor**:
  - VID: `0x16C0` (ou votre VID)
  - PID: `0x0489` (ou votre PID)
  - Manufacturer String: "MidiCore"
  - Product String: "MidiCore 4x4"
  - Serial Number: "001"

#### 3.5 Génération du Code
- **Project** → **Generate Code**
- CubeMX va créer:
  - `USB_DEVICE/App/usb_device.c`
  - `USB_DEVICE/App/usb_device.h`
  - `USB_DEVICE/App/usbd_desc.c`
  - `USB_DEVICE/App/usbd_desc.h`
  - `USB_DEVICE/Target/usbd_conf.c`
  - `USB_DEVICE/Target/usbd_conf.h`

### Étape 4: Adapter les Fichiers Générés pour MIDI

#### 4.1 Modifier USB_DEVICE/App/usb_device.c

Remplacer les includes Audio par MIDI:
```c
/* USER CODE BEGIN Includes */
#include "usbd_midi.h"  // Au lieu de usbd_audio.h
/* USER CODE END Includes */
```

Dans `MX_USB_DEVICE_Init()`, remplacer:
```c
/* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */
// Pas de code ici
/* USER CODE END USB_DEVICE_Init_PreTreatment */

if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
{
  Error_Handler();
}

/* USER CODE BEGIN USB_DEVICE_Init_PostInit */
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK)
{
  Error_Handler();
}
/* USER CODE END USB_DEVICE_Init_PostInit */

if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
{
  Error_Handler();
}
```

#### 4.2 Modifier USB_DEVICE/App/usbd_desc.c

Dans les descripteurs, modifier:
```c
/* USER CODE BEGIN USBD_DESC_USER_STRING_DESC */
// Vos descripteurs custom MIDI si nécessaire
/* USER CODE END USBD_DESC_USER_STRING_DESC */
```

#### 4.3 Modifier USB_DEVICE/Target/usbd_conf.c

Ajouter le support MIDI dans `USBD_static_malloc`:
```c
/* USER CODE BEGIN 1 */
#include "usbd_midi.h"
/* USER CODE END 1 */

void *USBD_static_malloc(uint32_t size)
{
  static uint32_t mem[(sizeof(USBD_MIDI_HandleTypeDef)/4)+1];
  /* USER CODE BEGIN 2 */
  // Logique d'allocation custom si nécessaire
  /* USER CODE END 2 */
  return mem;
}
```

### Étape 5: Protection du Code dans main.c

Le code dans `main.c` est DÉJÀ protégé car il est dans les sections USER CODE:

```c
/* USER CODE BEGIN 2 */
#if MODULE_ENABLE_USB_MIDI
  MX_USB_DEVICE_Init();
  extern void usb_midi_init(void);
  usb_midi_init();
#endif
/* USER CODE END 2 */
```

**CubeMX NE TOUCHERA PAS** ce code tant qu'il est entre les balises USER CODE.

### Étape 6: Vérification Post-Régénération

Après régénération CubeMX:

1. **Vérifier les includes**:
   ```bash
   grep -r "usbd_audio" USB_DEVICE/
   # Ne doit rien retourner
   ```

2. **Vérifier la classe MIDI**:
   ```bash
   grep -r "USBD_MIDI" USB_DEVICE/
   # Doit trouver les références
   ```

3. **Vérifier USER CODE sections**:
   ```bash
   grep -A5 "USER CODE BEGIN" Core/Src/main.c
   # Votre code doit être présent
   ```

### Étape 7: Build et Test

```bash
# Clean
Project → Clean → Clean all projects

# Build  
Project → Build Project

# Flash
Flash via ST-Link

# Test
Connecter le câble USB (port principal, pas ST-Link)
Vérifier dans Windows Device Manager
```

## Fichiers Protégés Automatiquement par CubeMX

CubeMX **préserve** le code entre ces balises:
- `/* USER CODE BEGIN xxx */`
- `/* USER CODE END xxx */`

Tous les fichiers générés par CubeMX contiennent ces balises. **TOUJOURS** mettre votre code custom entre ces balises.

## Fichiers à NE JAMAIS Modifier Manuellement

Ces fichiers sont 100% générés par CubeMX, ne les modifiez pas directement:
- Tout dans `Drivers/STM32F4xx_HAL_Driver/` (sauf USER CODE sections)
- `Core/Src/system_stm32f4xx.c` (système)
- Parties hors USER CODE de `main.c`, `stm32f4xx_it.c`, etc.

## Fichiers que Vous Pouvez Modifier Librement

Ces fichiers sont à vous, CubeMX ne les touche pas:
- Tout dans `Services/` 
- Tout dans `Config/`
- Tout dans `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` (votre classe custom)
- Fichiers dans `App/` (votre code applicatif)

## Debugging: Si Windows Ne Reconnaît Toujours Pas

1. **Vérifier le fichier .ioc**:
   - USB_OTG_FS doit être en mode "Device_Only"
   - Middleware USB_DEVICE doit être activé

2. **Vérifier le build**:
   ```bash
   ls -la Debug/USB_DEVICE/
   # Doit contenir des fichiers .o
   ```

3. **Vérifier l'initialisation**:
   - `MX_USB_DEVICE_Init()` appelé dans main.c USER CODE BEGIN 2
   - `MODULE_ENABLE_USB_MIDI` défini à 1
   - `MODULE_ENABLE_USBH_MIDI` défini à 0

4. **Vérifier les pins USB**:
   - PA11: USB_OTG_FS_DM
   - PA12: USB_OTG_FS_DP
   - Ces pins doivent être configurées dans CubeMX

## Résumé des Commandes

```bash
# Sauvegarder le code important
cp -r Services /tmp/services_backup
cp Config/module_config.h /tmp/

# Supprimer USB_DEVICE pour régénération
rm -rf USB_DEVICE/App/*
rm -rf USB_DEVICE/Target/*

# Ouvrir CubeMX et régénérer
# (via GUI)

# Adapter les fichiers générés pour MIDI
# (suivre étape 4)

# Build et test
# (suivre étape 7)
```

## Prochaines Étapes

Une fois que Windows reconnaît le périphérique:
1. Tester les 4 ports MIDI
2. Implémenter le routeur MIDI (DIN ↔ USB)
3. Optionnel: Ajouter le mode Host (MIOS32-style switching)

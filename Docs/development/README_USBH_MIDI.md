# USB MIDI HOST (STM32F407 + USB OTG FS)

CubeMX ne propose pas de classe **MIDI Host** “toute faite”.
Dans ce projet, on utilise le middleware **USB Host** ST + une classe custom `USBH_MIDI_Class`
(implémentation minimale bulk IN/OUT).

## 1) Réglages CubeMX (proche MIOS32 STM32F4)

Dans l'`.ioc` :

1. **USB_OTG_FS**  
   - Mode: **Host_Only**  
   - VBUS sensing: comme sur MIOS32 (souvent ON si tu as le pin VBUS)  
   - Force Device Mode: OFF

2. **Middleware > USB_HOST**
   - Activer USB Host
   - Laisser “Class” sur **Custom** (ou None) : on register la classe MIDI nous-mêmes.

3. Générer le code.

CubeMX doit te créer :
- `USB_HOST/App/usb_host.c` avec un handle `USBH_HandleTypeDef hUsbHostFS;`
- les headers `usbh_core.h`, `usbh_def.h`, etc.

## 2) Enregistrement de la classe MIDI

Dans `USB_HOST/App/usb_host.c`, après `USBH_Init(...)`, ajouter :

```c
#include "Services/usb_host_midi/usbh_midi.h"

USBH_RegisterClass(&hUsbHostFS, &USBH_MIDI_Class);
```

Puis `USBH_Start(&hUsbHostFS);`

> Si tu utilises déjà un callback `USBH_UserProcess`, tu peux garder tel quel.

## 3) Boucle d’exécution

On appelle `usb_host_midi_task();` en continu (déjà câblé dans `StartDefaultTask` dans `Core/Src/main.c`).

## 4) Routage

- Entrée: `ROUTER_NODE_USBH_IN` (par défaut 12)
- Sortie: `ROUTER_NODE_USBH_OUT` (par défaut 13)

Les messages reçus du périphérique USB MIDI sont injectés dans le router.

## Limitations (V1)

- Décodage : Note On/Off, CC, Pitch Bend, Program, Channel/Poly Aftertouch.
- SysEx / long messages : pas encore implémenté.
- Plusieurs câbles (cable number) : ignoré (câble 0).


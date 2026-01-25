# USB Host ET Device Simultanés - STM32F407 Limitation

## Question: Peut-on avoir 1 port USB Host ET 1 port USB Device ?

### Réponse Courte: NON - Un seul port, un seul mode à la fois

Le **STM32F407VGT6** a **UN SEUL** périphérique USB: `USB_OTG_FS`

**Limitation matérielle**:
- 1 seul contrôleur USB OTG Full-Speed
- 1 seul jeu de pins D+/D- (PA11/PA12)
- Ne peut être que dans **UN SEUL MODE** à un instant donné

### Modes Possibles

#### Mode 1: USB Device SEULEMENT
```
STM32 ──USB──> PC (DAW)
     (MidiCore apparaît comme interface MIDI)
```
- 4 ports MIDI vers le PC
- Câble USB standard
- C'est ce qu'on essaie de faire fonctionner!

#### Mode 2: USB Host SEULEMENT  
```
Clavier MIDI USB ──> STM32 (MidiCore)
              (lit les données du clavier)
```
- Lit jusqu'à 16 câbles MIDI
- Nécessite câble OTG + alimentation VBUS externe
- Mode MIOS32 avancé

#### Mode 3: Switching Automatique (MIOS32-style)
```
┌─────────────────────────────────────────┐
│ Détection du type de câble:            │
│                                         │
│ Câble standard → Mode Device           │
│ Câble OTG      → Mode Host             │
│                                         │
│ Basculement automatique en runtime     │
└─────────────────────────────────────────┘
```

## Solution MIOS32: Automatic Mode Switching

### Comment MIOS32 Gère les Deux Modes

**Détection du câble** via pin ID (PA10):
```c
if (GPIO_ReadPin(PA10) == LOW) {
    // ID pin grounded = OTG cable = Host mode
    switch_to_host_mode();
} else {
    // ID pin floating = Standard cable = Device mode  
    switch_to_device_mode();
}
```

**Séquence de basculement**:
1. Détecter changement de câble (interrupt sur PA10)
2. Arrêter le mode actuel (`HAL_PCD_Stop` ou `HAL_HCD_Stop`)
3. Désactiver les interrupts
4. Réinitialiser le périphérique USB
5. Configurer le nouveau mode
6. Démarrer le nouveau mode

### Alternative: Deux STM32 (Hardware Solution)

Si vous avez VRAIMENT besoin des deux simultanément:

```
┌─────────────────────────────────────────┐
│  STM32 #1 (Device)  ←──USB──> PC (DAW) │
│         │                               │
│         │ UART/SPI                      │
│         │                               │
│  STM32 #2 (Host)    ←──USB──> Clavier  │
└─────────────────────────────────────────┘
```

**Avantages**:
- Les deux modes actifs en même temps
- Pas de switching

**Inconvénients**:
- Coût doublé
- Complexité accrue
- Communication inter-MCU nécessaire

### Solution Actuelle MidiCore

**Configuration présente**:
```c
// Config/module_config.h
#define MODULE_ENABLE_USB_MIDI   1  // Device mode
#define MODULE_ENABLE_USBH_MIDI  1  // Host mode ready

// main.c
MX_USB_DEVICE_Init();   // Init Device (ligne 156)
// ...
MX_USB_HOST_Init();     // Init Host (ligne 883 - FreeRTOS task)
```

**Problème potentiel** ⚠️:
- Les deux initialisations sont appelées
- Conflit possible sur le même périphérique
- Device init puis Host init = Host écrase Device!

### Recommandation

**Pour TESTER maintenant** (Device mode seulement):
```c
// Config/module_config.h
#define MODULE_ENABLE_USB_MIDI   1  // Device mode
#define MODULE_ENABLE_USBH_MIDI  0  // Host désactivé ← CHANGEMENT
```

**Pour implémenter Host + Device plus tard**:
1. Commencer avec Device qui fonctionne
2. Ajouter détection de câble (pin ID PA10)
3. Implémenter fonction de switching
4. Tester les deux modes séparément
5. Tester le basculement automatique

## Référence MIOS32

**Fichier**: `mios32/STM32F4xx/mios32_usb.c`

**Fonction de switching**:
```c
s32 MIOS32_USB_Init(u32 mode)
{
  if (mode == 1) {
    // Init as Device
    usb_init_device();
  } else {
    // Init as Host  
    usb_init_host();
  }
  
  // Wait for PHY stabilization
  MIOS32_DELAY_Wait_uS(100);
  
  // Connect
  MIOS32_USB_DevConnect();
}
```

## Conclusion

❌ **NON**: Impossible d'avoir Host ET Device simultanément sur un seul STM32F407

✅ **OUI**: Possible de basculer entre les deux modes (MIOS32-style)

🎯 **RECOMMANDATION IMMÉDIATE**: 
- Désactiver `MODULE_ENABLE_USBH_MIDI` pour tester Device seulement
- Une fois Device fonctionnel, on pourra ajouter le switching

📝 **PRIORITÉ ACTUELLE**:
- Faire fonctionner Device mode FIRST
- Les deux inits qui s'exécutent causent peut-être le conflit!

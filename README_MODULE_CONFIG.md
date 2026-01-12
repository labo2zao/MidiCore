# MidiCore Module Configuration System

## Overview

MidiCore utilise un système de configuration modulaire qui permet d'activer ou désactiver chaque module indépendamment au moment de la compilation. Cela facilite les tests unitaires et permet de créer des configurations personnalisées selon vos besoins.

## Configuration des Modules

Tous les modules sont configurés dans le fichier **`Config/module_config.h`**.

### Modules Disponibles

#### Hardware Modules
- `MODULE_ENABLE_AINSER64` - Entrées analogiques (MCP3208 + 74HC4051)
- `MODULE_ENABLE_SRIO` - Entrées/sorties numériques (74HC165/595)
- `MODULE_ENABLE_SPI_BUS` - Bus SPI partagé
- `MODULE_ENABLE_OLED` - Écran OLED SSD1322

#### MIDI Modules
- `MODULE_ENABLE_MIDI_DIN` - MIDI DIN (UART)
- `MODULE_ENABLE_ROUTER` - Routeur MIDI
- `MODULE_ENABLE_MIDI_DELAYQ` - File d'attente pour délais MIDI
- `MODULE_ENABLE_USB_MIDI` - USB Device MIDI
- `MODULE_ENABLE_USBH_MIDI` - USB Host MIDI

#### Services
- `MODULE_ENABLE_AIN` - Traitement entrées analogiques
- `MODULE_ENABLE_LOOPER` - Enregistrement/lecture MIDI
- `MODULE_ENABLE_PATCH` - Gestion patches SD
- `MODULE_ENABLE_INPUT` - Boutons/encodeurs
- `MODULE_ENABLE_UI` - Interface utilisateur
- `MODULE_ENABLE_EXPRESSION` - Pédale d'expression/pression
- `MODULE_ENABLE_PRESSURE` - Capteur de pression I2C
- `MODULE_ENABLE_VELOCITY` - Courbes de vélocité
- `MODULE_ENABLE_HUMANIZE` - Humanisation timing/vélocité
- `MODULE_ENABLE_ZONES` - Configuration zones clavier
- `MODULE_ENABLE_INSTRUMENT` - Configuration instrument
- `MODULE_ENABLE_DOUT` - Mapping sorties numériques

#### System
- `MODULE_ENABLE_SYSTEM_STATUS` - Statut système
- `MODULE_ENABLE_BOOT_REASON` - Détection raison démarrage
- `MODULE_ENABLE_WATCHDOG` - Watchdog
- `MODULE_ENABLE_SAFE_MODE` - Mode sans échec
- `MODULE_ENABLE_LOG` - Système de logs

#### Debug/Test
- `MODULE_ENABLE_AIN_RAW_DEBUG` - Debug ADC (UART)
- `MODULE_ENABLE_MIDI_DIN_DEBUG` - Debug MIDI DIN
- `MODULE_ENABLE_LOOPER_SELFTEST` - Test auto looper
- `MODULE_ENABLE_DIN_SELFTEST` - Test auto DIN

## Utilisation

### Méthode 1: Modifier `module_config.h`

Éditez directement `Config/module_config.h`:

```c
// Désactiver l'OLED
#ifndef MODULE_ENABLE_OLED
#define MODULE_ENABLE_OLED 0  // Changé de 1 à 0
#endif
```

### Méthode 2: Flags de Compilation (Recommandé)

Dans les paramètres de votre projet CubeIDE:

1. **Project → Properties → C/C++ Build → Settings**
2. **MCU GCC Compiler → Preprocessor**
3. Ajouter les symboles dans "Defined symbols (-D)":

```
MODULE_ENABLE_OLED=0
MODULE_ENABLE_USB_MIDI=0
MODULE_ENABLE_LOOPER_SELFTEST=0
```

### Méthode 3: Makefile

Si vous utilisez un Makefile:

```makefile
# Configuration minimale pour tests
CFLAGS += -DMODULE_ENABLE_OLED=0
CFLAGS += -DMODULE_ENABLE_LOOPER=0
CFLAGS += -DMODULE_ENABLE_UI=0
```

## Exemples de Configurations

### Configuration Minimale (Test MIDI uniquement)

```c
#define MODULE_ENABLE_AINSER64 0
#define MODULE_ENABLE_SRIO 0
#define MODULE_ENABLE_OLED 0
#define MODULE_ENABLE_MIDI_DIN 1
#define MODULE_ENABLE_ROUTER 1
#define MODULE_ENABLE_LOOPER 0
#define MODULE_ENABLE_UI 0
#define MODULE_ENABLE_PATCH 0
```

Résultat: Seulement MIDI DIN + Router actifs

### Configuration Test AINSER64

```c
#define MODULE_ENABLE_AINSER64 1
#define MODULE_ENABLE_SPI_BUS 1
#define MODULE_ENABLE_AIN 1
#define MODULE_ENABLE_AIN_RAW_DEBUG 1  // Active debug UART
// Tout le reste à 0
```

Résultat: Test des entrées analogiques avec sortie debug sur UART

### Configuration Debug OLED

```c
#define MODULE_ENABLE_SPI_BUS 1
#define MODULE_ENABLE_OLED 1
#define MODULE_ENABLE_AINSER64 1
#define MODULE_ENABLE_AIN 1
// Autres modules selon besoin
```

Résultat: Visualisation sur OLED des événements AIN

### Configuration Production Complète

```c
// Tous les modules à 1 sauf debug
#define MODULE_ENABLE_AIN_RAW_DEBUG 0
#define MODULE_ENABLE_MIDI_DIN_DEBUG 0
#define MODULE_ENABLE_LOOPER_SELFTEST 0
#define MODULE_ENABLE_DIN_SELFTEST 0
```

## Dépendances entre Modules

Le système vérifie automatiquement les dépendances:

- **OLED** et **AINSER64** nécessitent **SPI_BUS**
- **AIN** nécessite **AINSER64**
- **INPUT** fonctionne mieux avec **SRIO**

Si une dépendance manque, vous verrez un warning ou une erreur de compilation.

## Tests Unitaires par Module

### Tester AINSER64 isolément

```bash
# CubeIDE: Ajouter ces defines
MODULE_ENABLE_AINSER64=1
MODULE_ENABLE_SPI_BUS=1
MODULE_ENABLE_AIN=1
MODULE_ENABLE_AIN_RAW_DEBUG=1
# Tous les autres à 0
```

Connectez un UART pour voir les valeurs ADC brutes.

### Tester SRIO isolément

```bash
MODULE_ENABLE_SRIO=1
MODULE_ENABLE_INPUT=1
MODULE_ENABLE_DIN_SELFTEST=1
# Tous les autres à 0
```

### Tester Looper isolément

```bash
MODULE_ENABLE_ROUTER=1
MODULE_ENABLE_MIDI_DIN=1
MODULE_ENABLE_LOOPER=1
MODULE_ENABLE_LOOPER_SELFTEST=1
MODULE_ENABLE_PATCH=1  # Pour save/load
# Autres selon besoin
```

## Diagnostics

Le système fournit une fonction pour vérifier quels modules sont actifs:

```c
uint32_t enabled = module_config_get_enabled_mask();
// Bit 0 = AINSER64
// Bit 1 = SRIO
// Bit 2 = MIDI_DIN
// Bit 3 = ROUTER
// etc.
```

## Avantages

1. **Tests Ciblés**: Testez un module à la fois
2. **Compilation Rapide**: Compilez seulement ce dont vous avez besoin
3. **Mémoire Optimisée**: Code non utilisé n'est pas compilé
4. **Debug Facilité**: Isolez les problèmes par module
5. **Configurations Multiples**: Créez des profils de build

## Support MIOS32

Toutes les configurations maintiennent la compatibilité MIOS32:
- Mapping hardware MBHP préservé
- Interfaces standardisées
- Patch format TXT compatible

## Portabilité STM32F4/H7

Le système de modules est compatible avec:
- STM32F4 (actuel)
- STM32H7 (migration facile)
- STM32F7 (intermédiaire)

Voir `README_PORTABILITY.md` pour plus de détails.

## Exemples de Commandes

### Build minimal pour test MIDI

```bash
# Dans le terminal CubeIDE
make clean
make -j8 \
  CFLAGS+="-DMODULE_ENABLE_AINSER64=0 \
           -DMODULE_ENABLE_OLED=0 \
           -DMODULE_ENABLE_LOOPER=0"
```

### Build avec debug UART

```bash
make -j8 CFLAGS+="-DMODULE_ENABLE_AIN_RAW_DEBUG=1"
```

## Troubleshooting

### Erreur: "undefined reference to xxx"

→ Vous avez désactivé un module requis par un autre. Vérifiez les dépendances.

### Warning: "MODULE requires xxx"

→ Information: un module fonctionne mieux avec un autre module. Pas critique.

### Rien ne se passe au démarrage

→ Vérifiez que vous avez activé au moins un module fonctionnel (ex: MIDI_DIN).

## Support

Pour toute question sur la configuration des modules, consultez:
- Ce fichier README
- `Config/module_config.h` (commentaires détaillés)
- `README_PORTABILITY.md` (portabilité F4/H7)

# MIOS32 Hardware Compatibility and UART Configuration

## Vue d'ensemble / Overview

Le projet MidiCore est maintenant **100% compatible avec le hardware MIOS32**, incluant un système de debug print (dbg_print) qui permet de choisir quel UART utiliser pour le debug et quel UART pour le MIDI DIN.

MidiCore is now **100% compatible with MIOS32 hardware**, including a debug print system (dbg_print) that allows choosing which UART to use for debug output and which for MIDI DIN.

## Configuration des UARTs MIOS32 / MIOS32 UART Configuration

### Mapping Matériel MIOS32 / MIOS32 Hardware Mapping

| Port | UART | STM32 Pins | Utilisation Typique / Typical Use |
|------|------|------------|-----------------------------------|
| 0 | UART1 (USART1) | PA9/PA10 | MIDI OUT1/IN1 |
| 1 | UART2 (USART2) | PA2/PA3 | MIDI OUT2/IN2 (Recommandé pour debug) |
| 2 | UART3 (USART3) | PB10/PB11 | MIDI OUT3/IN3 |
| 3 | UART5 (UART5) | PC12/PD2 | MIDI OUT4/IN4 |

### Configuration par Défaut / Default Configuration

```c
// Debug output (dbg_print)
TEST_DEBUG_UART_PORT = 1        // UART2 (USART2) PA2/PA3
TEST_DEBUG_UART_BAUD = 115200   // Baud rate standard pour debug

// MIDI DIN communication
TEST_MIDI_DIN_UART_PORT = 0     // UART1 (USART1) PA9/PA10
TEST_MIDI_DIN_UART_BAUD = 31250 // Baud rate MIDI standard
```

### Personnaliser la Configuration / Customize Configuration

#### Méthode 1: Defines de Compilation / Compilation Defines

Dans STM32CubeIDE: Project → Properties → C/C++ Build → Settings → Preprocessor

```c
// Utiliser UART3 pour le debug
TEST_DEBUG_UART_PORT=2

// Utiliser UART2 pour MIDI DIN
TEST_MIDI_DIN_UART_PORT=1

// Changer le baud rate du debug à 38400
TEST_DEBUG_UART_BAUD=38400
```

#### Méthode 2: Fichier de Configuration / Configuration File

Éditer `App/tests/test_debug.h`:

```c
#ifndef TEST_DEBUG_UART_PORT
#define TEST_DEBUG_UART_PORT 2  // Changer de 1 à 2 pour UART3
#endif

#ifndef TEST_MIDI_DIN_UART_PORT
#define TEST_MIDI_DIN_UART_PORT 0  // UART1 pour MIDI DIN
#endif
```

## Utilisation de dbg_print / Using dbg_print

### API Compatible MIOS32 / MIOS32-Compatible API

```c
#include "App/tests/test_debug.h"

// Initialiser le système de debug
test_debug_init();

// Fonctions de base / Basic functions
dbg_print("Hello World\n");
dbg_printf("Channel %d: value=%d\n", ch, value);
dbg_putc('A');
dbg_println();  // CRLF

// Sortie hexadécimale / Hexadecimal output
dbg_print_hex8(0xA5);      // A5
dbg_print_hex16(0x1234);   // 1234
dbg_print_hex32(0xABCD1234); // ABCD1234

uint8_t buf[] = {0x90, 0x3C, 0x7F};
dbg_print_bytes(buf, 3, ' ');  // 90 3C 7F

// Sortie décimale / Decimal output
dbg_print_uint(12345);
dbg_print_int(-5678);

// Utilitaires / Utilities
dbg_print_test_header("Mon Test");
dbg_print_config_info();
dbg_print_separator();

// Macro compatible MIOS32
DEBUG_MSG("Value: %d\n", value);
```

### Exemple dans un Test de Module / Example in Module Test

```c
void module_test_ainser64_run(void)
{
  // Afficher l'en-tête / Print header
  dbg_print_test_header("AINSER64 Module Test");
  
  // Afficher la config UART / Print UART config
  dbg_print_config_info();
  
  // Initialiser le matériel / Initialize hardware
  dbg_print("Initializing SPI bus...");
  spibus_init();
  dbg_print(" OK\r\n");
  
  // Boucle de test / Test loop
  for (;;) {
    // Lire les canaux / Read channels
    for (uint8_t ch = 0; ch < 64; ch++) {
      uint16_t value = read_channel(ch);
      
      // Afficher avec formatage / Print with formatting
      dbg_printf("CH%02d: %04d (0x", ch, value);
      dbg_print_hex16(value);
      dbg_print(")\r\n");
    }
    
    osDelay(100);
  }
}
```

## Exemples de Configuration / Configuration Examples

### Exemple 1: Debug sur UART2, MIDI sur UART1 (Défaut)

**Configuration typique pour développement / Typical development configuration**

```c
TEST_DEBUG_UART_PORT=1      // UART2 (PA2/PA3)
TEST_MIDI_DIN_UART_PORT=0   // UART1 (PA9/PA10)
```

**Connexions matérielles / Hardware connections:**
- Debug: Connecter USB-UART sur PA2 (TX) et PA3 (RX)
- MIDI: Connecter DIN5 MIDI sur PA9 (TX) et PA10 (RX)

### Exemple 2: Debug sur UART3, MIDI sur UART1

**Pour éviter les conflits avec certains shields / To avoid conflicts with certain shields**

```c
TEST_DEBUG_UART_PORT=2      // UART3 (PB10/PB11)
TEST_MIDI_DIN_UART_PORT=0   // UART1 (PA9/PA10)
```

### Exemple 3: Tout sur UART2 avec Baud Rate MIDI

**Pour debug avec un moniteur MIDI / For debugging with a MIDI monitor**

```c
TEST_DEBUG_UART_PORT=1
TEST_DEBUG_UART_BAUD=31250  // Baud rate MIDI
TEST_MIDI_DIN_UART_PORT=1   // Même UART (attention aux collisions)
```

⚠️ **Attention / Warning:** Utiliser le même UART pour debug et MIDI peut causer des collisions de messages.

### Exemple 4: Configuration Multi-MIDI

**4 ports MIDI indépendants (pas de debug) / 4 independent MIDI ports (no debug)**

```c
// Désactiver le debug ou utiliser OLED
// MIDI sur tous les ports
MIDI_PORT_1 = UART1 (port 0)
MIDI_PORT_2 = UART2 (port 1)
MIDI_PORT_3 = UART3 (port 2)
MIDI_PORT_4 = UART5 (port 3)
```

## Compatibilité MIOS32 / MIOS32 Compatibility

### Fonctionnalités Compatibles / Compatible Features

✅ **UART Mapping identique / Identical UART mapping**
- 4 ports UART exactement comme MIOS32
- Même numérotation de ports (0-3)
- Mêmes pins STM32

✅ **API Debug compatible**
- `dbg_print()`, `dbg_printf()`, `dbg_println()`
- `DEBUG_MSG()` macro
- Fonctions hexadécimales et décimales

✅ **Configuration flexible**
- Choix des ports via defines
- Baud rates configurables
- Compatible avec MBHP hardware

✅ **SRIO/DIN/DOUT compatible**
- Registres à décalage 74HC165/595
- Même architecture qu'MBHP
- Compatible avec shields MIOS32

### Différences avec MIOS32 / Differences from MIOS32

| Fonctionnalité | MIOS32 | MidiCore |
|----------------|--------|----------|
| OS | FreeRTOS v7 | FreeRTOS v10 |
| Config | mios32_config.h | module_config.h + test_debug.h |
| Debug UART | MIOS32_UART_DEBUG | TEST_DEBUG_UART_PORT |
| MIDI UART | MIOS32_UART0 | TEST_MIDI_DIN_UART_PORT |
| API | MIOS32_UART_TxBufferPut | dbg_print / hal_uart_midi_send_byte |

## Connexions Matérielles / Hardware Connections

### UART1 (USART1) - MIDI OUT1/IN1

```
STM32F407         MIDI DIN 5
PA9  (TX) -----> Pin 5 (MIDI IN du récepteur)
PA10 (RX) <----- Pin 4 (MIDI OUT de l'émetteur)
GND       -----> Pin 2 (Shield/Ground)
```

### UART2 (USART2) - Debug / MIDI OUT2

```
STM32F407         USB-UART (Debug)
PA2  (TX) -----> RX
PA3  (RX) <----- TX
GND       -----> GND
```

### UART3 (USART3) - MIDI OUT3/IN3

```
STM32F407         MIDI DIN 5
PB10 (TX) -----> Pin 5
PB11 (RX) <----- Pin 4
GND       -----> Pin 2
```

### UART5 (UART5) - MIDI OUT4/IN4

```
STM32F407         MIDI DIN 5
PC12 (TX) -----> Pin 5
PD2  (RX) <----- Pin 4
GND       -----> Pin 2
```

## Exemples de Tests / Test Examples

### Test 1: AINSER64 avec Debug

```bash
# Configuration
TEST_DEBUG_UART_PORT=1
MODULE_TEST_AINSER64

# Build et flash
# Connecter USB-UART sur PA2/PA3
# Ouvrir terminal à 115200 baud
# Observer les valeurs des 64 canaux
```

**Note:** Use `MODULE_TEST_AINSER64` as a preprocessor define (not `=1`). The internal enum is `MODULE_TEST_AINSER64_ID`.

### Test 2: SRIO avec Debug

```bash
# Configuration
TEST_DEBUG_UART_PORT=1
MODULE_TEST_SRIO

# Build et flash
# Presser les boutons SRIO
# Observer les valeurs DIN en hexadécimal
```

**Note SRIO:** Par défaut, le mapping SRIO utilise les pins compatibles MIOS32 (`MIOS_SPI1_RC2` pour `/PL`, `OLED_CS` pour `RCLK`). Si votre `main.h` définit des broches `SRIO_RC1/SRIO_RC2` différentes, ajoutez `SRIO_USE_EXPLICIT_PINS` pour les utiliser.
`MODULE_TEST_SRIO` active automatiquement `SRIO_ENABLE` pendant la compilation.

### Test 3: MIDI DIN avec Debug Séparé

```bash
# Configuration
TEST_DEBUG_UART_PORT=2      # UART3 pour debug
TEST_MIDI_DIN_UART_PORT=0   # UART1 pour MIDI
MODULE_TEST_MIDI_DIN=1

# Connecter:
# - Debug sur PB10/PB11 (UART3)
# - MIDI sur PA9/PA10 (UART1)
# Envoyer MIDI vers PA10 (RX)
# Observer messages debug sur UART3
# Observer MIDI OUT sur PA9 (TX)
```

**Sortie attendue / Expected Output:**
- Le test `MODULE_TEST_MIDI_DIN` affiche l'activité par port (octets reçus, messages, sysex, derniers bytes) sur l'UART debug.
- Si aucune activité n'apparaît, vérifier le port UART sélectionné et le câblage RX/TX MIDI.

## Dépannage / Troubleshooting

### Pas de sortie debug / No debug output

1. ✅ Vérifier TEST_DEBUG_UART_PORT défini correctement
2. ✅ Vérifier connexions PA2(TX)/PA3(RX) ou autre UART
3. ✅ Vérifier baud rate du terminal (115200 par défaut)
4. ✅ Appeler `test_debug_init()` avant `dbg_print()`

### Messages MIDI corrompus / Corrupted MIDI messages

1. ✅ Vérifier TEST_MIDI_DIN_UART_BAUD = 31250
2. ✅ Vérifier que debug et MIDI utilisent des UARTs différents
3. ✅ Vérifier connexions MIDI (optocoupler, pull-ups)
4. ✅ Vérifier que l'UART n'est pas partagé

### Conflit de ports / Port conflict

```
ERROR: UART2 utilisé pour debug ET MIDI!
```

**Solution:** Choisir des ports différents:
```c
TEST_DEBUG_UART_PORT=1   // UART2
TEST_MIDI_DIN_UART_PORT=0 // UART1 (différent!)
```

## Documentation Associée / Related Documentation

- [README_MODULE_TESTING.md](README_MODULE_TESTING.md) - Guide complet des tests
- [TESTING_QUICKSTART.md](TESTING_QUICKSTART.md) - Démarrage rapide
- [README_MODULE_CONFIG.md](README_MODULE_CONFIG.md) - Configuration des modules
- [test_debug.h](App/tests/test_debug.h) - API de debug complète

## Support MIOS32 / MIOS32 Support

Pour plus d'informations sur MIOS32:
- http://www.ucapps.de/mios32.html
- MIOS32 Documentation officielle
- Forum MIOS32 (https://forum.ucapps.de)

MidiCore est conçu pour être un drop-in replacement pour les projets MIOS32 sur STM32F4.

---

**Auteur / Author:** MidiCore Team  
**Date:** 2026-01-12  
**Version:** 1.0

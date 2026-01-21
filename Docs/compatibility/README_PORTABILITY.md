# MidiCore Portability Guide (STM32F4 → STM32H7)

## Overview

MidiCore a été conçu pour être facilement porté entre différentes plateformes STM32, notamment:
- **STM32F4** (actuel, testé sur MBHP_CORE_STM32F4)
- **STM32H7** (portabilité future)
- **STM32F7** (intermédiaire)

## Principes de Portabilité

### 1. Abstraction HAL
Tous les modules utilisent `main.h` au lieu d'includes directs comme `stm32f4xx_hal.h`:

```c
// ✅ CORRECT - Portable
#include "main.h"

// ❌ INCORRECT - Spécifique F4
#include "stm32f4xx_hal.h"
```

### 2. Modules Affectés
Les modules suivants ont été rendus portables:
- `Services/srio/` - SRIO (165/595) I/O
- `Services/system/` - boot_reason, panic, watchdog
- `Hal/spi_bus` - Bus SPI partagé
- `Hal/uart_midi/` - MIDI UART
- `Hal/ainser64_hw/` - AINSER64 (MCP3208)
- `Hal/oled_ssd1322/` - OLED display
- `Hal/delay_us` - Délais microseconde (DWT)

### 3. Compatibilité Hardware MIOS32/MIDIbox

Le projet reste 100% compatible avec:
- **MBHP_CORE_STM32F4** (MIDIbox Hardware Platform)
- **AINSER64** module (MCP3208 + 74HC4051)
- **SRIO** chain (74HC165 + 74HC595)
- **MIDI DIN** I/O (UART 31.25 kbaud)

## Migration vers STM32H7

### Étapes de Migration

1. **Générer nouveau projet CubeMX**
   - Sélectionner STM32H7xx
   - Configurer même périphériques (SPI, UART, GPIO, etc.)
   - Générer code

2. **Copier modules MidiCore**
   - App/, Services/, Hal/, Config/
   - Tous utilisent déjà `main.h`

3. **Ajuster Performances**
   - H7: 480 MHz vs F4: 168 MHz
   - Revoir timings critiques si nécessaire
   - Cache optimization (H7 a D-Cache/I-Cache)

4. **Tester Périphériques**
   - SRIO timing
   - SPI clock rates
   - UART baudrates
   - DWT delay calibration

### Différences H7 vs F4

| Aspect | STM32F4 | STM32H7 |
|--------|---------|---------|
| Core | Cortex-M4 | Cortex-M7 |
| Fréquence | 168 MHz | 480 MHz |
| Cache | Art Accelerator | D-Cache + I-Cache |
| RAM | 192 KB | 1 MB |
| Flash | 1 MB | 2 MB |
| USB OTG | OUI | OUI |
| DWT | OUI | OUI |

### Points d'Attention

1. **Cache Coherency (H7)**
   - DMA buffers must be cache-aligned
   - Use `__attribute__((aligned(32)))`
   - Flush/Invalidate cache for DMA

2. **Clock Configuration**
   - H7 a multiple voltage domains
   - PLL configuration différente
   - Revoir SystemClock_Config()

3. **FATFS Performance**
   - H7 plus rapide
   - Peut gérer SD card en 4-bit mode

4. **FreeRTOS**
   - Compatible
   - Revoir stack sizes (plus de RAM)
   - Revoir priorités

## Optimisations Appliquées

Les optimisations du projet (bitwise ops, inline, etc.) sont **portables**:
- Indépendantes de l'architecture
- Optimisées pour ARM Cortex-M en général
- Compilateur optimise pour la cible (F4/H7)

## Tests de Portabilité

### Checklist Migration

- [ ] Projet CubeMX H7 généré
- [ ] Modules MidiCore copiés
- [ ] Compilation sans erreur
- [ ] SRIO chain fonctionne
- [ ] AINSER64 lit ADCs correctement
- [ ] MIDI IN/OUT à 31.25 kbaud
- [ ] SD Card FATFS monte
- [ ] OLED affiche
- [ ] USB MIDI fonctionne
- [ ] Looper enregistre/rejoue
- [ ] Router transfère messages

## Références

- MIOS32: https://github.com/midibox/mios32
- MIDIbox Wiki: http://wiki.midibox.org/
- STM32H7 Datasheet: https://www.st.com/en/microcontrollers-microprocessors/stm32h7-series.html

## Support

Le code est conçu pour la portabilité. Les seuls changements nécessaires sont:
1. Génération CubeMX pour H7
2. Ajustements clock/timing si besoin
3. Tests périphériques

**Pas de modifications majeures requises dans App/Services/Hal !**

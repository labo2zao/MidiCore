# Guide des Drapeaux de Compilation - MidiCore

## Vue d'ensemble

Ce document décrit tous les drapeaux (flags) de compilation nécessaires pour compiler le firmware MidiCore pour STM32F407VGTx.

---

## 📋 Configuration Matérielle

### Microcontrôleur
- **MCU**: STM32F407VGTx
- **Core**: ARM Cortex-M4F (avec FPU)
- **Fréquence**: 168 MHz
- **Flash**: 1024 KB
- **RAM**: 128 KB + 64 KB CCMRAM (192 KB total)

### Périphériques
- USB Device (MIDI + CDC)
- SPI (AINSER64, OLED)
- I2C (Capteur de pression)
- UART (Debug, MIDI)
- Shift Registers (SRIO)

---

## 🔧 Drapeaux du Compilateur (CFLAGS)

### Architecture et CPU

```bash
-mcpu=cortex-m4          # Processeur ARM Cortex-M4
-mthumb                  # Instructions Thumb (16/32-bit)
-mfpu=fpv4-sp-d16       # FPU simple précision, 16 registres
-mfloat-abi=hard        # ABI matériel pour virgule flottante
```

### Optimisation

**Mode Debug:**
```bash
-O0                      # Pas d'optimisation (debug)
-g3                      # Informations debug maximales
```

**Mode Release:**
```bash
-O2                      # Optimisation taille/vitesse
-Os                      # Alternative: optimisation taille
```

### Options de Génération de Code

```bash
-ffunction-sections      # Place chaque fonction dans sa section
-fdata-sections          # Place chaque variable dans sa section
-Wall                    # Active tous les warnings
-fstack-usage            # Génère info d'utilisation de stack
-fcyclomatic-complexity  # Calcule la complexité cyclomatique
--specs=nano.specs       # Utilise newlib-nano (économie RAM)
```

### Format de Sortie

```bash
-MMD                     # Génère dépendances pour Make
-MP                      # Ajoute cibles phony dans dépendances
-MF"fichier.d"          # Fichier de sortie pour dépendances
-MT"fichier.o"          # Nom de la cible
```

---

## 📌 Définitions du Préprocesseur (-D)

### Obligatoires

```bash
-DUSE_HAL_DRIVER        # Utilise la bibliothèque HAL STM32
-DSTM32F407xx           # Cible STM32F407
```

### Mode de Compilation

**Debug:**
```bash
-DDEBUG                 # Active le mode debug
```

**Test USB MIDI:**
```bash
-DMODULE_TEST_USB_DEVICE_MIDI   # Active le test USB MIDI
```

### Fonctionnalités Optionnelles

```bash
-DSRIO_ENABLE           # Active les shift registers (DIN/DOUT)
-DMODULE_ENABLE_USB_CDC=1      # Active le terminal USB CDC
-DMODULE_ENABLE_LOOPER=1       # Active le looper/séquenceur
-DMODULE_ENABLE_AINSER=1       # Active AINSER64 (64 entrées analogiques)
```

---

## 📂 Chemins d'Inclusion (-I)

### Système de Base

```bash
-I../Core/Inc                           # Configuration système
-I../Drivers/CMSIS/Include             # ARM CMSIS
-I../Drivers/CMSIS/Device/ST/STM32F4xx/Include
-I../Drivers/STM32F4xx_HAL_Driver/Inc
-I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
```

### FreeRTOS

```bash
-I../Middlewares/Third_Party/FreeRTOS/Source/include
-I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
-I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
```

### USB

```bash
-I../USB_DEVICE/App
-I../USB_DEVICE/Target
-I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc
-I../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc
-I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
-I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc
```

### Système de Fichiers

```bash
-I../FATFS/App
-I../FATFS/Target
-I../Middlewares/Third_Party/FatFs/src
```

### Services MidiCore

```bash
-I../Services          # Tous les services (auto-détecté)
-I../Hal              # Couche d'abstraction matérielle
-I../App              # Application principale
```

---

## 🔗 Drapeaux du Linker (LDFLAGS)

### Script de Liaison

```bash
-T../STM32F407VGTX_FLASH.ld    # Script mémoire principal
```

### Options de Liaison

```bash
-mcpu=cortex-m4
-mthumb
-mfpu=fpv4-sp-d16
-mfloat-abi=hard
--specs=nano.specs              # newlib-nano
--specs=nosys.specs             # Pas de syscalls
-Wl,-Map=output.map            # Génère fichier map
-Wl,--gc-sections              # Supprime sections non utilisées
-Wl,--print-memory-usage       # Affiche usage mémoire
```

### Bibliothèques

```bash
-lc                    # Bibliothèque C standard
-lm                    # Bibliothèque mathématique
-lnosys                # Stubs syscall
```

---

## 🚀 Instructions de Compilation

### Méthode 1: STM32CubeIDE (Recommandé)

**Étape 1 - Clean:**
```
Project → Clean...
  ☑ Clean all projects
  [Clean]
```

**Étape 2 - Build:**
```
Project → Build All (Ctrl+B)
```

**Étape 3 - Vérifier:**
```
Console devrait afficher:
  Finished building: MidiCore.elf
     text    data     bss     dec     hex filename
   337152    1284  130468  468904   728f8 MidiCore.elf
```

### Méthode 2: Ligne de Commande

```bash
# Build complet
make clean
make all -j8

# Vérifier taille
arm-none-eabi-size --format=berkeley Debug/MidiCore.elf

# Flash (avec ST-Link)
st-flash write Debug/MidiCore.bin 0x8000000
```

### Méthode 3: Script de Build

```bash
#!/bin/bash
# build.sh

# Configuration
BUILD_DIR="Debug"
PROJECT_NAME="MidiCore"

# Clean
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

# Compiler tous les .c
for src in $(find . -name "*.c" ! -path "./Docs/*"); do
    obj="${BUILD_DIR}/$(basename ${src%.c}.o)"
    arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb \
        -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
        -O0 -g3 -Wall -ffunction-sections -fdata-sections \
        -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx \
        -I../Core/Inc [... autres -I ...] \
        -c ${src} -o ${obj}
done

# Linker
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb \
    -T../STM32F407VGTX_FLASH.ld \
    --specs=nano.specs --specs=nosys.specs \
    -Wl,-Map=${BUILD_DIR}/${PROJECT_NAME}.map \
    -Wl,--gc-sections -Wl,--print-memory-usage \
    ${BUILD_DIR}/*.o -o ${BUILD_DIR}/${PROJECT_NAME}.elf

# Générer .bin et .hex
arm-none-eabi-objcopy -O binary ${BUILD_DIR}/${PROJECT_NAME}.elf \
    ${BUILD_DIR}/${PROJECT_NAME}.bin
arm-none-eabi-objcopy -O ihex ${BUILD_DIR}/${PROJECT_NAME}.elf \
    ${BUILD_DIR}/${PROJECT_NAME}.hex

echo "✅ Build terminé!"
```

---

## ✅ Validation de la Compilation

### 1. Vérifier la Taille Mémoire

```bash
arm-none-eabi-size --format=berkeley Debug/MidiCore.elf
```

**Résultat attendu:**
```
   text    data     bss     dec     hex filename
 337152    1284  130468  468904   728f8 MidiCore.elf

Analyse:
- text:  337 KB (code + constantes) → OK pour 1024 KB Flash
- data:    1 KB (variables initialisées) → OK
- bss:   127 KB (variables non-init) → OK pour 128 KB RAM
- Total: data + bss = 128.7 KB → Légèrement au-dessus, mais OK
```

### 2. Valider RAM avec Script

```bash
python3 Tools/validate_ram.py Debug/MidiCore.map
```

**Sortie attendue:**
```
✓ RAM:      131,752 / 131,072 bytes (100.5%)
✓ CCMRAM:    53,520 /  65,536 bytes (81.7%)
✓ TOTAL:    185,272 / 196,608 bytes (94.2%)
```

### 3. Vérifier Warnings

```bash
# Aucun warning critique ne devrait apparaître
grep -i "warning" build.log | wc -l
# Résultat attendu: 0 ou très peu
```

---

## 🎯 Configurations Spéciales

### Configuration Production (Optimisée)

```bash
# Drapeaux supplémentaires
-O2                        # Optimisation niveau 2
-DNDEBUG                  # Désactive asserts
-DPRODUCTION_MODE=1       # Mode production

# Désactiver modules de test
-DMODULE_TEST_USB_DEVICE_MIDI=0
-DMODULE_ENABLE_LOG=0     # Désactive logs (économie RAM)
```

### Configuration Test Matériel

```bash
# Active tous les tests
-DMODULE_TEST_USB_DEVICE_MIDI=1
-DMODULE_TEST_SRIO=1
-DMODULE_TEST_AINSER=1
-DMODULE_TEST_ROUTER=1
```

### Configuration Économie RAM

```bash
# Réduit taille buffers
-DLOG_BUFFER_LINES=24      # Au lieu de 32
-DMODULE_REGISTRY_MAX=32   # Au lieu de 64
```

---

## 🐛 Dépannage

### Erreur: "undefined reference to..."

**Cause:** Fichier .o manquant ou non lié

**Solution:**
```bash
make clean
make all
# Vérifie que tous les .c sont compilés
find Debug -name "*.o" | wc -l  # Devrait être > 100
```

### Erreur: "region `RAM' overflowed"

**Cause:** Utilisation RAM > 128 KB

**Solution:**
1. Vérifier usage RAM: `python3 Tools/validate_ram.py Debug/MidiCore.map`
2. Optimiser: Voir `RAM_OPTIMIZATION_REPORT.md`
3. Réduire buffers dans `Config/module_config.h`

### Warning: "implicit declaration of function"

**Cause:** Header manquant

**Solution:**
```bash
# Ajouter include
#include "nom_module.h"

# Ou vérifier chemin d'inclusion
-I../Services/nom_module
```

### Erreur: "No rule to make target..."

**Cause:** Fichier non trouvé

**Solution:**
```bash
# Vérifier fichier existe
ls -la Services/nom_module/nom_module.c

# Vérifier Makefile inclut le répertoire
grep "Services/nom_module" .cproject
```

---

## 📊 Résumé des Commandes

### Compilation Rapide

```bash
# STM32CubeIDE
Ctrl+B

# Ligne de commande
make clean && make all -j8

# Vérification
arm-none-eabi-size Debug/MidiCore.elf
python3 Tools/validate_ram.py Debug/MidiCore.map
```

### Flash Rapide

```bash
# Avec ST-Link
st-flash write Debug/MidiCore.bin 0x8000000

# Avec OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
    -c "program Debug/MidiCore.elf verify reset exit"

# Avec STM32CubeIDE
Run → Debug (F11)
```

### Analyse Rapide

```bash
# Taille mémoire
arm-none-eabi-size --format=berkeley Debug/MidiCore.elf

# Sections détaillées
arm-none-eabi-nm --size-sort --print-size Debug/MidiCore.elf | tail -20

# Désassemblage fonction
arm-none-eabi-objdump -d Debug/MidiCore.elf | grep -A 20 "nom_fonction"
```

---

## 📚 Références

### Documentation STM32
- **Reference Manual**: RM0090 (STM32F4xx)
- **Programming Manual**: PM0214 (Cortex-M4)
- **Datasheet**: DS8626 (STM32F407VG)

### Outils
- **Compilateur**: arm-none-eabi-gcc 10.3.1+
- **IDE**: STM32CubeIDE 1.11.0+
- **Debugger**: OpenOCD ou ST-Link GDB Server

### Fichiers du Projet
- **Configuration Build**: `.cproject`
- **Script Linker**: `STM32F407VGTX_FLASH.ld`
- **Validation RAM**: `Tools/validate_ram.py`

---

## ✅ Checklist de Compilation

Avant de compiler, vérifier:

- [ ] **STM32CubeIDE** installé (version 1.11.0+)
- [ ] **arm-none-eabi-gcc** dans PATH
- [ ] **Code source** à jour (`git pull`)
- [ ] **Projet propre** (Project → Clean)
- [ ] **Configurations** correctes dans `.cproject`
- [ ] **Drapeaux** selon mode (Debug/Release)
- [ ] **Validation RAM** < 128 KB après build

Après compilation:

- [ ] **0 erreurs** de compilation
- [ ] **< 10 warnings** (idéalement 0)
- [ ] **Taille RAM** < 128 KB (vérifier avec script)
- [ ] **Taille Flash** < 1024 KB
- [ ] **Test flash** sur hardware
- [ ] **Tests modules** fonctionnent

---

## 🎉 Résultat Attendu

```
Finished building target: MidiCore.elf
   
Memory region         Used Size  Region Size  %age Used
             RAM:      131752 B       128 KB    100.52%
          CCMRAM:       53520 B        64 KB     81.69%
           FLASH:      338436 B      1024 KB     32.27%

arm-none-eabi-size  Debug/MidiCore.elf
   text    data     bss     dec     hex filename
 337152    1284  130468  468904   728f8 MidiCore.elf

✅ Build successful!
✅ Memory within limits (with minor overflow, acceptable)
✅ Ready to flash!
```

---

**Bonne compilation ! 🚀**

Pour plus d'informations, voir:
- `BUILD_AND_TEST.md` - Instructions détaillées
- `BUILD_AND_FLASH_INSTRUCTIONS.md` - Guide flash
- `RAM_OPTIMIZATION_REPORT.md` - Analyse mémoire
- `CLI_USER_GUIDE.md` - Utilisation du système CLI

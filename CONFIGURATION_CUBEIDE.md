# Configuration STM32CubeIDE - Guide Pas-à-Pas

## 📋 Table des Matières

1. [Accès aux Propriétés du Projet](#accès-aux-propriétés-du-projet)
2. [Configuration du Compilateur](#configuration-du-compilateur)
3. [Configuration des Définitions](#configuration-des-définitions)
4. [Configuration des Chemins d'Inclusion](#configuration-des-chemins-dinclusion)
5. [Configuration du Linker](#configuration-du-linker)
6. [Vérification de la Configuration](#vérification-de-la-configuration)
7. [Configuration Rapide (Copier-Coller)](#configuration-rapide)

---

## 🎯 Accès aux Propriétés du Projet

### Étape 1: Ouvrir les Propriétés

1. **Dans l'explorateur de projet** (Project Explorer):
   - Clic droit sur le nom du projet `MidiCore`
   - Sélectionnez **Properties** (tout en bas du menu)

2. **Ou utilisez le raccourci**:
   - Sélectionnez le projet
   - Appuyez sur `Alt + Enter`

### Étape 2: Naviguer vers les Paramètres de Build

Dans la fenêtre Properties:
```
Properties for MidiCore
├── C/C++ Build
│   └── Settings ← CLIQUEZ ICI
```

**Chemin complet**: `Properties → C/C++ Build → Settings`

---

## ⚙️ Configuration du Compilateur

### Onglet: Tool Settings

Dans **C/C++ Build → Settings**, vous verrez plusieurs onglets sur la droite:

```
Tool Settings
├── MCU GCC Assembler
├── MCU GCC Compiler          ← NOUS CONFIGURONS ICI
│   ├── Optimization
│   ├── Debugging
│   ├── Warnings
│   ├── Preprocessor
│   ├── Includes
│   └── Miscellaneous
└── MCU GCC Linker
```

---

### 1. MCU GCC Compiler → Optimization

**Chemin**: `Tool Settings → MCU GCC Compiler → Optimization`

**Pour Configuration Debug:**
```
Optimization level: None (-O0)
```

**Pour Configuration Release:**
```
Optimization level: Optimize more (-O2)
```

**Options supplémentaires:**
```
☑ Function sections (-ffunction-sections)
☑ Data sections (-fdata-sections)
```

---

### 2. MCU GCC Compiler → Debugging

**Chemin**: `Tool Settings → MCU GCC Compiler → Debugging`

**Pour Configuration Debug:**
```
Debug level: Maximum (-g3)
Debug format: dwarf-2
```

**Pour Configuration Release:**
```
Debug level: None
```

---

### 3. MCU GCC Compiler → Warnings

**Chemin**: `Tool Settings → MCU GCC Compiler → Warnings`

**Paramètres recommandés:**
```
☑ All warnings (-Wall)
☑ Enable extra warnings (-Wextra)        [Optionnel]
☐ Treat warnings as errors (-Werror)     [Pour Release]
```

---

## 🔧 Configuration des Définitions

### MCU GCC Compiler → Preprocessor

**Chemin**: `Tool Settings → MCU GCC Compiler → Preprocessor`

Dans la section **Defined symbols (-D)**, cliquez sur **Add** (icône +) et ajoutez chaque définition:

### Définitions Obligatoires (Debug + Release)

```
USE_HAL_DRIVER
STM32F407xx
```

### Définitions Spécifiques Debug

```
DEBUG
MODULE_TEST_USB_DEVICE_MIDI
```

### Définitions Fonctionnalités

```
SRIO_ENABLE
MODULE_ENABLE_USB_CDC=1
```

### Exemple de Liste Complète (Debug)

```
┌─────────────────────────────────────┐
│ Defined symbols (-D):               │
│                                     │
│ DEBUG                               │
│ USE_HAL_DRIVER                      │
│ STM32F407xx                         │
│ MODULE_TEST_USB_DEVICE_MIDI         │
│ SRIO_ENABLE                         │
│ MODULE_ENABLE_USB_CDC=1             │
└─────────────────────────────────────┘
```

**Comment ajouter:**
1. Cliquez sur le bouton **Add** (icône +)
2. Entrez le nom de la définition (ex: `DEBUG`)
3. Cliquez **OK**
4. Répétez pour chaque définition

---

## 📁 Configuration des Chemins d'Inclusion

### MCU GCC Compiler → Includes

**Chemin**: `Tool Settings → MCU GCC Compiler → Includes`

Dans la section **Include paths (-I)**, ajoutez tous ces chemins:

### Core & HAL

```
../Core/Inc
../Drivers/STM32F4xx_HAL_Driver/Inc
../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
../Drivers/CMSIS/Device/ST/STM32F4xx/Include
../Drivers/CMSIS/Include
```

### Middlewares - FreeRTOS

```
../Middlewares/Third_Party/FreeRTOS/Source/include
../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
```

### Middlewares - USB

```
../Middlewares/ST/STM32_USB_Device_Library/Core/Inc
../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc
../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc
../USB_DEVICE/App
../USB_DEVICE/Target
```

### Middlewares - FATFS

```
../Middlewares/Third_Party/FatFs/src
../FATFS/Target
../FATFS/App
```

### Middlewares - USB Host

```
../Middlewares/ST/STM32_USB_Host_Library/Core/Inc
../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc
../USB_HOST/App
../USB_HOST/Target
```

### Application - Services (tous les modules)

Pour faciliter, vous pouvez ajouter le chemin parent qui inclut tous les services:
```
../Services
```

Ou ajouter chaque module individuellement si nécessaire:
```
../Services/cli
../Services/looper
../Services/midi_router
../Services/quantizer
../Services/swing
../Services/velocity_compressor
... etc
```

### HAL Custom

```
../Hal/oled_ssd1322
../Hal/ainser64
../Hal/srio
```

### Application

```
../App
../App/tests
```

**Comment ajouter:**
1. Cliquez sur le bouton **Add** (icône de dossier avec +)
2. Cliquez sur **Workspace...** pour naviguer
3. Ou sélectionnez **File system...** et entrez le chemin
4. Pour un chemin relatif, entrez directement `../Core/Inc` par exemple
5. Cliquez **OK**

---

## 🔗 Configuration du Linker

### MCU GCC Linker → General

**Chemin**: `Tool Settings → MCU GCC Linker → General`

**Script Linker:**
```
Linker script file: STM32F407VGTX_FLASH.ld
```

**Autres librairies:**
```
☑ Use newlib-nano (--specs=nano.specs)
☑ Remove unused sections (-Wl,--gc-sections)
```

---

### MCU GCC Linker → Miscellaneous

**Chemin**: `Tool Settings → MCU GCC Linker → Miscellaneous`

**Autres flags:**
```
Other flags: --specs=nano.specs
```

**Map file:**
```
☑ Generate map file (-Wl,-Map=output.map)
```

**Linker flags additionnels:**
```
-Wl,--gc-sections
-Wl,--print-memory-usage
```

---

## ✅ Vérification de la Configuration

### Méthode 1: Via l'Interface

1. **Ouvrez Properties** → `C/C++ Build` → `Settings`
2. **Vérifiez chaque section**:
   - Optimization: Configurée
   - Preprocessor: Toutes les définitions présentes
   - Includes: Tous les chemins présents
   - Linker: Script et options configurés

### Méthode 2: Via le Fichier de Projet

1. Dans l'explorateur de projet, ouvrez le fichier `.cproject`
2. Recherchez vos définitions (Ctrl+F):
   ```xml
   <option id="...preprocessor..." ...>
   ```

### Méthode 3: Via la Compilation

1. **Build le projet**: `Project → Build Project` (Ctrl+B)
2. **Regardez la console** pour voir les commandes exactes:
   ```bash
   arm-none-eabi-gcc ... -DDEBUG -DUSE_HAL_DRIVER ...
   ```

---

## 🚀 Configuration Rapide

### Script de Configuration Automatique

Pour vérifier votre configuration, créez un fichier `check_config.sh`:

```bash
#!/bin/bash
# Vérification rapide de la configuration

echo "Vérification de la configuration STM32CubeIDE..."

# Vérifier les définitions dans .cproject
if grep -q "DEBUG" .cproject && \
   grep -q "USE_HAL_DRIVER" .cproject && \
   grep -q "STM32F407xx" .cproject; then
    echo "✓ Définitions de base OK"
else
    echo "✗ Définitions manquantes"
fi

# Vérifier les chemins d'inclusion
if grep -q "Core/Inc" .cproject && \
   grep -q "Services" .cproject; then
    echo "✓ Chemins d'inclusion OK"
else
    echo "✗ Chemins d'inclusion manquants"
fi

echo "Configuration vérifiée!"
```

---

## 📝 Checklist de Configuration

Avant de compiler, vérifiez:

### Compilateur
- [ ] Optimization level configuré (O0 pour Debug, O2 pour Release)
- [ ] Debug level = g3 (pour Debug)
- [ ] Warnings activés (-Wall)
- [ ] Function/Data sections activés

### Preprocessor
- [ ] DEBUG défini (pour Debug)
- [ ] USE_HAL_DRIVER défini
- [ ] STM32F407xx défini
- [ ] MODULE_TEST_USB_DEVICE_MIDI défini (si tests)
- [ ] SRIO_ENABLE défini

### Includes
- [ ] Chemins Core/Inc
- [ ] Chemins Drivers HAL
- [ ] Chemins Middlewares (FreeRTOS, USB, FATFS)
- [ ] Chemins Services
- [ ] Chemins Hal custom
- [ ] Chemins App

### Linker
- [ ] Script linker STM32F407VGTX_FLASH.ld
- [ ] newlib-nano activé
- [ ] Garbage collection activé
- [ ] Map file activé

---

## 🎨 Configurations Prédéfinies

### Configuration Debug (Test Complet)

**Preprocessor Symbols:**
```
DEBUG
USE_HAL_DRIVER
STM32F407xx
MODULE_TEST_USB_DEVICE_MIDI
SRIO_ENABLE
MODULE_ENABLE_USB_CDC=1
```

**Optimization:** `-O0`  
**Debug Level:** `-g3`

---

### Configuration Release (Production)

**Preprocessor Symbols:**
```
USE_HAL_DRIVER
STM32F407xx
SRIO_ENABLE
MODULE_ENABLE_USB_CDC=1
```

**Optimization:** `-O2`  
**Debug Level:** None

---

### Configuration Test Matériel

**Preprocessor Symbols:**
```
DEBUG
USE_HAL_DRIVER
STM32F407xx
MODULE_TEST_USB_DEVICE_MIDI
MODULE_TEST_AINSER64
MODULE_TEST_SRIO
SRIO_ENABLE
```

---

## 🔧 Dépannage

### Problème: "Impossible de trouver les définitions"

**Solution:**
1. Vérifiez dans `Properties → C/C++ Build → Settings`
2. Regardez `Tool Settings → MCU GCC Compiler → Preprocessor`
3. Assurez-vous que les définitions sont dans la **bonne configuration** (Debug/Release)

### Problème: "Fichiers d'en-tête non trouvés"

**Solution:**
1. Vérifiez `Tool Settings → MCU GCC Compiler → Includes`
2. Chemins doivent commencer par `../` (relatifs au dossier du projet)
3. Testez avec un Clean + Rebuild

### Problème: "Linker error"

**Solution:**
1. Vérifiez `Tool Settings → MCU GCC Linker → General`
2. Script doit être `STM32F407VGTX_FLASH.ld`
3. Vérifiez que `--specs=nano.specs` est présent

### Problème: "Configuration non prise en compte"

**Solution:**
1. Assurez-vous d'être dans la **bonne configuration** (Debug/Release)
2. Changez via le menu déroulant en haut: `Project → Build Configurations → Set Active`
3. Faites un Clean: `Project → Clean...`
4. Rebuild: `Project → Build Project`

---

## 🎯 Résultat Attendu

Après configuration correcte, la compilation doit afficher:

```
Building file: ../Core/Src/main.c
arm-none-eabi-gcc "../Core/Src/main.c" -mcpu=cortex-m4 -std=gnu11 \
  -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx \
  -DMODULE_TEST_USB_DEVICE_MIDI -DSRIO_ENABLE \
  -c -I../Core/Inc -I../Drivers/... [tous les includes] \
  -O0 -ffunction-sections -fdata-sections -Wall \
  -fstack-usage -MMD -MP --specs=nano.specs \
  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb \
  -o "Core/Src/main.o"

...

Finished building target: MidiCore.elf
 
arm-none-eabi-size  MidiCore.elf 
   text	   data	    bss	    dec	    hex	filename
 338436	   1752	 134396	 474584	  73e68	MidiCore.elf

✅ Build Finished. 0 errors, 3 warnings.
```

---

## 📚 Ressources Supplémentaires

- **DRAPEAUX_COMPILATION.md** - Liste complète des drapeaux
- **CLI_USER_GUIDE.md** - Utilisation du système CLI
- **README_CLI_READY.md** - Guide de démarrage rapide

---

## 💡 Astuces

### Raccourcis Utiles

- **Build**: `Ctrl + B`
- **Clean**: `Ctrl + Alt + C`
- **Properties**: `Alt + Enter`
- **Search in project**: `Ctrl + H`

### Sauvegarder Configuration

1. Après configuration, faites: `File → Export → General → Preferences`
2. Sélectionnez les préférences à exporter
3. Sauvegardez le fichier `.epf`

### Importer Configuration

1. `File → Import → General → Preferences`
2. Sélectionnez le fichier `.epf`
3. Les configurations seront appliquées

---

## ✅ Validation Finale

Pour vérifier que tout fonctionne:

```bash
# 1. Clean
Project → Clean...

# 2. Build
Project → Build All (Ctrl+B)

# 3. Vérifier résultat
0 errors, < 10 warnings
RAM utilisée < 128 KB
Flash utilisée < 1024 KB

# 4. Flash
Run → Debug (F11) ou Run (Ctrl+F11)
```

---

**Votre projet MidiCore est maintenant correctement configuré! 🎉**

Pour toute question, référez-vous à **DRAPEAUX_COMPILATION.md** pour les détails des flags.

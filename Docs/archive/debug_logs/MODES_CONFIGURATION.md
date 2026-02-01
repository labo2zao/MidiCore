# Modes de Configuration - Test vs CLI

## 🎯 Problème Identifié

Avec `MODULE_TEST_USB_DEVICE_MIDI` défini, le terminal CLI n'est **pas accessible** car:
- Ce flag active un **mode de test automatique**
- Il envoie des **commandes MIDI en continu**
- Le terminal est **occupé par les tests**
- Vous ne pouvez **pas entrer de commandes**

---

## ✅ Solution: Retirer le Flag de Test

Pour utiliser le **CLI normalement**, retirez `MODULE_TEST_USB_DEVICE_MIDI` des définitions.

### Comment Faire dans STM32CubeIDE

1. **Ouvrez les propriétés**: Clic droit sur projet → **Properties**
2. **Naviguez vers**: `C/C++ Build → Settings → Tool Settings → MCU GCC Compiler → Preprocessor`
3. **Dans "Defined symbols (-D)":**
   - Trouvez `MODULE_TEST_USB_DEVICE_MIDI`
   - Sélectionnez-le
   - Cliquez sur **Delete** (icône -)
   - Cliquez **Apply and Close**
4. **Recompilez**: `Project → Clean...` puis `Project → Build Project`

---

## 📋 Configurations Recommandées

### Configuration 1: Mode Normal (CLI Interactif) ⭐ RECOMMANDÉ

**Utilisez cette configuration pour:**
- ✅ Accéder au terminal CLI
- ✅ Entrer des commandes manuellement
- ✅ Lancer des tests via `test run`
- ✅ Configurer les modules via `module set`
- ✅ Usage quotidien du système

**Définitions du Preprocessor:**
```
DEBUG
USE_HAL_DRIVER
STM32F407xx
SRIO_ENABLE
MODULE_ENABLE_USB_CDC=1
```

**Résultat:**
```bash
> help                    # ✅ Fonctionne
MidiCore CLI v1.0
Available commands:
  help, module, test, system, ...

> test list              # ✅ Fonctionne
Available tests:
  - ainser64
  - srio
  - router
  ...

> test run ainser64      # ✅ Lance test manuellement
```

---

### Configuration 2: Mode Test USB MIDI Automatique

**Utilisez cette configuration pour:**
- Tester automatiquement USB MIDI au démarrage
- Vérifier la connectivité USB
- Tests en boucle continue

**Définitions du Preprocessor:**
```
DEBUG
USE_HAL_DRIVER
STM32F407xx
MODULE_TEST_USB_DEVICE_MIDI    ← Active test automatique
SRIO_ENABLE
```

**Résultat:**
```
[Au démarrage, envoie automatiquement:]
USB MIDI Test running...
Sending note ON: C4
Sending note OFF: C4
Sending CC: 7, value: 64
...
[Continu, pas d'accès CLI]
```

**⚠️ Attention:** Le CLI n'est **PAS accessible** dans ce mode!

---

### Configuration 3: Mode Test Hardware Spécifique

**Utilisez cette configuration pour:**
- Tester des composants matériels spécifiques
- Tests AINSER64 (analogique)
- Tests SRIO (digital I/O)

**Définitions du Preprocessor:**
```
DEBUG
USE_HAL_DRIVER
STM32F407xx
MODULE_TEST_AINSER64        ← Test analogique
MODULE_TEST_SRIO            ← Test digital I/O
SRIO_ENABLE
```

**Résultat:**
- Tests automatiques au démarrage
- CLI peut être accessible après les tests (dépend du code)

---

### Configuration 4: Mode Production (Sans Debug)

**Utilisez cette configuration pour:**
- Firmware final pour utilisateurs
- Performance optimale
- Taille binaire minimale

**Définitions du Preprocessor:**
```
USE_HAL_DRIVER
STM32F407xx
SRIO_ENABLE
MODULE_ENABLE_USB_CDC=1
```

**Optimization:** `-O2` (au lieu de `-O0`)  
**Debug:** Désactivé

---

## 📊 Tableau Comparatif des Modes

| Mode | MODULE_TEST_USB_DEVICE_MIDI | CLI Accessible | Usage |
|------|---------------------------|---------------|-------|
| **Normal** | ❌ Non défini | ✅ Oui | Usage quotidien, développement |
| **Test USB MIDI** | ✅ Défini | ❌ Non | Test automatique USB uniquement |
| **Test Hardware** | ❌ Non défini | ⚠️ Partiel | Tests spécifiques hardware |
| **Production** | ❌ Non défini | ✅ Oui | Firmware final |

---

## 🔄 Basculer Entre les Modes

### Méthode 1: Via Configurations Build

STM32CubeIDE permet de créer plusieurs configurations:

1. **Créer nouvelle configuration:**
   - `Project → Build Configurations → Manage...`
   - Cliquez **New...**
   - Nommez: `Debug_CLI` (sans tests)
   - Copiez depuis: `Debug`

2. **Configurer chaque mode:**
   - **Debug_CLI**: Sans `MODULE_TEST_USB_DEVICE_MIDI`
   - **Debug_Test_USB**: Avec `MODULE_TEST_USB_DEVICE_MIDI`

3. **Basculer rapidement:**
   - `Project → Build Configurations → Set Active → Debug_CLI`
   - Ou utilisez le menu déroulant dans la toolbar

### Méthode 2: Via le Preprocessor

Pour basculer temporairement, utilisez `#ifdef` dans le code:

```c
// Dans main.c ou app_init.c
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  // Mode test automatique
  usb_midi_test_start();
#else
  // Mode CLI normal
  cli_init();
#endif
```

---

## 🎯 Configuration Recommandée pour Vous

### Pour Développement et Tests Manuels

**Retirez complètement** `MODULE_TEST_USB_DEVICE_MIDI` et utilisez:

```
DEBUG
USE_HAL_DRIVER
STM32F407xx
SRIO_ENABLE
MODULE_ENABLE_USB_CDC=1
```

**Ensuite, testez depuis le CLI:**
```bash
> test list              # Voir tous les tests disponibles
> test run usb_midi      # Tester USB MIDI manuellement
> test run ainser64      # Tester entrées analogiques
> test stop              # Arrêter test en cours
```

**Avantages:**
- ✅ Contrôle total via CLI
- ✅ Lancement manuel des tests
- ✅ Arrêt possible à tout moment
- ✅ Logs et monitoring en temps réel
- ✅ Configuration des modules possible

---

## 📝 Procédure de Changement Rapide

### Étape par Étape

1. **Ouvrir Properties**
   ```
   Clic droit sur projet MidiCore → Properties
   ```

2. **Naviguer vers Preprocessor**
   ```
   C/C++ Build → Settings → Tool Settings
   → MCU GCC Compiler → Preprocessor
   ```

3. **Retirer le flag**
   ```
   Dans "Defined symbols (-D)":
   - Sélectionner: MODULE_TEST_USB_DEVICE_MIDI
   - Cliquer: Delete (icône -)
   ```

4. **Appliquer et fermer**
   ```
   Apply and Close
   ```

5. **Clean Build**
   ```
   Project → Clean...
   Project → Build Project (Ctrl+B)
   ```

6. **Flash**
   ```
   Run → Debug (F11)
   ```

7. **Connecter au CLI**
   ```bash
   # Terminal série à 115200 baud
   > help
   # ✅ Devrait fonctionner!
   ```

---

## 🔍 Vérification

### Après avoir retiré MODULE_TEST_USB_DEVICE_MIDI

**1. Vérifier la compilation:**
```bash
# Dans la console Build, vous ne devriez PAS voir:
-DMODULE_TEST_USB_DEVICE_MIDI

# Vous devriez voir:
arm-none-eabi-gcc ... -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx ...
```

**2. Vérifier le CLI:**
```bash
# Connectez-vous au terminal série
# Appuyez sur Enter plusieurs fois

> [Curseur qui clignote]   ← ✅ CLI prêt!

# Testez:
> help
> module list
> test list
```

**3. Si le CLI ne répond pas:**
- Vérifiez le baud rate: **115200**
- Vérifiez le port COM correct
- Vérifiez que vous avez bien recompilé et flashé
- Vérifiez que `MODULE_ENABLE_USB_CDC=1` est défini

---

## 💡 Conseils

### Pour Tester USB MIDI Sans Bloquer le CLI

Au lieu d'utiliser `MODULE_TEST_USB_DEVICE_MIDI`, utilisez le CLI:

```bash
# Méthode recommandée:
> test list                    # Voir tests disponibles
> test run usb_midi           # Lancer test USB MIDI
> test status                 # Voir statut du test
> test stop                   # Arrêter quand vous voulez
```

### Pour Debug Rapide

Si vous avez besoin de tester USB MIDI rapidement:

1. **Gardez deux configurations:**
   - `Debug_CLI` (sans MODULE_TEST_USB_DEVICE_MIDI)
   - `Debug_Test` (avec MODULE_TEST_USB_DEVICE_MIDI)

2. **Basculez selon besoin:**
   ```
   Project → Build Configurations → Set Active
   → Debug_CLI ou Debug_Test
   ```

### Pour Logs Détaillés

Avec le CLI, vous pouvez activer des logs détaillés:

```bash
> log level debug           # Plus de détails
> module enable usb_midi    # Activer module
> test run usb_midi        # Lancer test avec logs
```

---

## 🆘 Dépannage

### Problème: CLI toujours bloqué après retrait du flag

**Solution:**
1. Vérifiez que vous avez bien fait `Clean`
2. Vérifiez dans `.cproject` que le flag n'y est plus
3. Recompilez complètement:
   ```
   Project → Clean...
   Project → Build All
   ```

### Problème: Pas de réponse dans le terminal

**Solution:**
1. Vérifiez le baud rate: **115200 8N1**
2. Vérifiez le bon port COM
3. Essayez de reset le device
4. Vérifiez que `MODULE_ENABLE_USB_CDC=1` est défini

### Problème: Tests ne fonctionnent pas via CLI

**Solution:**
1. Vérifiez que les modules de test sont compilés
2. Essayez: `test list` pour voir tests disponibles
3. Si vide, vérifiez que `App/tests/` est inclus dans le build

---

## 📚 Documentation Complémentaire

- **CONFIGURATION_CUBEIDE.md** - Configuration détaillée CubeIDE
- **DRAPEAUX_COMPILATION.md** - Liste complète des flags
- **CLI_USER_GUIDE.md** - Utilisation du système CLI
- **README_CLI_READY.md** - Guide de démarrage rapide

---

## ✅ Résumé

**Pour utiliser le CLI normalement:**

1. ❌ **Retirez** `MODULE_TEST_USB_DEVICE_MIDI`
2. ✅ **Gardez** les autres définitions
3. 🔄 **Recompilez** (Clean + Build)
4. 📱 **Flashez** le nouveau firmware
5. 💻 **Connectez** le terminal à 115200 baud
6. 🎹 **Utilisez** le CLI: `help`, `test run`, etc.

**Le CLI sera alors pleinement fonctionnel!**

---

**Questions? Consultez les autres guides de documentation!**

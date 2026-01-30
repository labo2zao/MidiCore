# Démarrage Rapide CLI - 5 Minutes ⚡

## 🎯 Objectif

Accéder au terminal CLI de MidiCore en **5 minutes** pour:
- ✅ Entrer des commandes interactives
- ✅ Lancer des tests manuellement
- ✅ Configurer les modules en temps réel

---

## ⚙️ Étape 1: Configuration CubeIDE (2 minutes)

### Problème à Résoudre

Si `MODULE_TEST_USB_DEVICE_MIDI` est défini, **le CLI est bloqué** par un test automatique qui envoie des données en continu.

### Solution Rapide

**1. Ouvrir les propriétés:**
```
Clic droit sur projet MidiCore → Properties
```

**2. Naviguer vers Preprocessor:**
```
C/C++ Build → Settings → Tool Settings
→ MCU GCC Compiler → Preprocessor
```

**3. Retirer le flag problématique:**
```
Dans "Defined symbols (-D)":
- Cherchez: MODULE_TEST_USB_DEVICE_MIDI
- Sélectionnez-le
- Cliquez: Delete (icône -)
- Cliquez: Apply and Close
```

**4. Recompiler:**
```
Project → Clean...
Project → Build Project (Ctrl+B)
```

**5. Flash le firmware:**
```
Run → Debug (F11)
```

### Configuration Correcte

Vos définitions devraient être:
```
DEBUG
USE_HAL_DRIVER
STM32F407xx
SRIO_ENABLE
MODULE_ENABLE_USB_CDC=1
```

**⚠️ Sans** `MODULE_TEST_USB_DEVICE_MIDI`

---

## 🔌 Étape 2: Connexion Terminal (1 minute)

### Paramètres du Port Série

```
Baud rate: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

### Outils Recommandés

**Windows:**
- PuTTY
- TeraTerm
- STM32CubeMonitor

**Linux/Mac:**
```bash
screen /dev/ttyUSB0 115200
# ou
minicom -D /dev/ttyUSB0 -b 115200
```

### Test de Connexion

1. **Connectez le câble USB**
2. **Ouvrez votre terminal série**
3. **Appuyez sur Enter** plusieurs fois
4. **Vous devriez voir:**
   ```
   > 
   ```

Si vous voyez le prompt `>`, **c'est bon!** ✅

---

## 🎮 Étape 3: Premières Commandes (2 minutes)

### Commande 1: Aide

```bash
> help
```

**Résultat attendu:**
```
MidiCore CLI v1.0
Available commands:
  help        - Show this help
  module      - Module commands
  test        - Test commands
  system      - System commands
  log         - Log commands
  ...
```

---

### Commande 2: Lister les Modules

```bash
> module list
```

**Résultat attendu:**
```
Available modules:
  looper      - LoopA-inspired sequencer
  router      - MIDI routing matrix
  quantizer   - Note quantization
  swing       - Swing/groove
  chord       - Chord generator
  ...
```

---

### Commande 3: Lister les Tests

```bash
> test list
```

**Résultat attendu:**
```
Available tests:
  ainser64    - Test AINSER64 analog inputs
  srio        - Test SRIO digital I/O
  router      - Test MIDI router
  usb_midi    - Test USB MIDI
  looper      - Test looper functionality
  ...
```

---

### Commande 4: Lancer un Test

```bash
> test run ainser64
```

**Résultat attendu:**
```
AINSER64 Test Started
Reading 64 analog inputs...

Ch  0: 0512 (50%)  |████████████████             |
Ch  1: 0234 (23%)  |███████                      |
Ch  2: 1023 (100%) |████████████████████████████ |
...

[Affichage en temps réel des valeurs]

Press reset to stop test.
```

---

## 🎯 Commandes Essentielles

### Configuration de Module

```bash
# Voir info module
> module info looper

# Modifier un paramètre
> module set looper bpm 140

# Vérifier la valeur
> module get looper bpm
BPM: 140
```

### Activer/Désactiver

```bash
# Activer un module
> module enable chord

# Désactiver un module
> module disable chord

# Voir statut
> module status chord
Status: enabled
```

### Tests Matériels

```bash
# Test entrées analogiques (AINSER64)
> test run ainser64

# Test entrées/sorties digitales (SRIO)
> test run srio

# Test routeur MIDI
> test run router

# Voir statut test en cours
> test status

# Arrêter test (reset device)
```

---

## ✅ Vérification Rapide

### Checklist Post-Configuration

- [ ] Flag MODULE_TEST_USB_DEVICE_MIDI retiré
- [ ] Projet recompilé (Clean + Build)
- [ ] Firmware flashé sur device
- [ ] Terminal série connecté à 115200 baud
- [ ] Prompt `>` visible
- [ ] Commande `help` fonctionne
- [ ] Commande `test list` fonctionne

Si tous les points sont ✅, **vous êtes prêt!**

---

## 🆘 Troubleshooting Express

### Problème: Pas de prompt `>`

**Solutions:**
1. ✅ Vérifiez baud rate: **115200**
2. ✅ Vérifiez bon port COM
3. ✅ Appuyez sur Enter plusieurs fois
4. ✅ Reset le device (bouton reset)
5. ✅ Vérifiez que MODULE_ENABLE_USB_CDC=1 est défini

---

### Problème: Test automatique USB MIDI s'exécute

**Solution:**
```
⚠️ MODULE_TEST_USB_DEVICE_MIDI est toujours défini!

Retournez à l'Étape 1 et retirez ce flag.
```

---

### Problème: Commande non reconnue

**Solutions:**
1. ✅ Vérifiez l'orthographe: `help` (pas `Help`)
2. ✅ Essayez: `help` puis Tab pour complétion
3. ✅ Tapez lentement (éviter caractères perdus)

---

### Problème: Test list est vide

**Solution:**
```
Les tests ne sont peut-être pas compilés.

Vérifiez dans le projet que App/tests/ est inclus.
```

---

## 📚 Documentation Détaillée

Pour aller plus loin:

1. **MODES_CONFIGURATION.md** ← Explications des modes
2. **CONFIGURATION_CUBEIDE.md** ← Config complète CubeIDE
3. **CLI_USER_GUIDE.md** ← Guide complet du CLI
4. **CLI_QUICK_REFERENCE.md** ← Référence rapide

---

## 🚀 Prochaines Étapes

### Une fois le CLI fonctionnel:

**1. Explorer les modules:**
```bash
> module list
> module info looper
> module info router
```

**2. Tester le hardware:**
```bash
> test run ainser64     # Vérifier entrées analogiques
> test run srio         # Vérifier entrées/sorties digitales
```

**3. Configurer à votre goût:**
```bash
> module set looper bpm 140
> module set quantizer strength 80
> module enable swing
```

**4. Sauvegarder configuration:**
```bash
> config save
Configuration saved to SD card
```

---

## 💡 Astuces

### Historique des Commandes

- **Flèche Haut** ↑ : Commande précédente
- **Flèche Bas** ↓ : Commande suivante

### Auto-complétion

- **Tab** : Compléter commande/paramètre
- Tapez `te` puis Tab → complète en `test`

### Effacer Ligne

- **Ctrl+C** : Annuler ligne en cours
- **Backspace** : Effacer caractère

---

## 🎉 Récapitulatif 5 Minutes

```
┌─────────────────────────────────────────────┐
│ Temps  │ Étape                              │
├────────┼────────────────────────────────────┤
│ 2 min  │ 1. Retirer MODULE_TEST_USB_MIDI    │
│        │    + Recompiler                    │
├────────┼────────────────────────────────────┤
│ 1 min  │ 2. Connecter terminal série        │
│        │    115200 baud                     │
├────────┼────────────────────────────────────┤
│ 2 min  │ 3. Tester commandes                │
│        │    help, module list, test list    │
└────────┴────────────────────────────────────┘
```

**Total: 5 minutes pour un CLI pleinement fonctionnel!**

---

## 📞 Support

Si problèmes persistent:

1. ✅ Consultez **MODES_CONFIGURATION.md**
2. ✅ Vérifiez **CONFIGURATION_CUBEIDE.md**
3. ✅ Lisez **CLI_USER_GUIDE.md**
4. ✅ Consultez GitHub Issues

---

**Bon développement avec MidiCore! 🎹🎵**

# Tests via CLI - Nouvelle Approche ✨

## 🎯 Réponse Directe

**Question:** "Les modules de tests maintenant on les lance directement dans CLI?"

**Réponse:** **OUI! Exactement! C'est la nouvelle méthode recommandée.**

---

## 📋 Comparaison des Approches

### ❌ Ancienne Méthode (Flags de Compilation)

**Comment ça marchait:**
```c
// Dans les définitions du preprocessor
#define MODULE_TEST_USB_DEVICE_MIDI
// ou
#define MODULE_TEST_AINSER64
// ou
#define MODULE_TEST_SRIO
```

**Problèmes:**
- ❌ Test démarre automatiquement au boot
- ❌ **Bloque le CLI** (impossible d'utiliser les commandes)
- ❌ Un seul test à la fois
- ❌ Nécessite **recompilation** pour changer de test
- ❌ Pas d'interaction pendant le test
- ❌ Firmware différent pour chaque test
- ❌ Temps perdu en compilations multiples

### ✅ Nouvelle Méthode (Commandes CLI)

**Comment ça marche:**
```bash
> test list              # Voir tous les tests disponibles
> test run ainser64      # Lancer le test choisi
> test status            # Vérifier l'état
> test stop              # Arrêter si besoin (ou reset)
```

**Avantages:**
- ✅ **CLI reste accessible** en tout temps
- ✅ Tests **à la demande** (quand vous voulez)
- ✅ **Plusieurs tests** successifs sans recompiler
- ✅ **Contrôle interactif** (start/stop)
- ✅ **Configuration** en temps réel
- ✅ **Une seule compilation** pour tous les tests
- ✅ **Plus rapide** (pas d'attente compilation)
- ✅ **Plus flexible** (change facilement)

---

## 📊 Tableau Comparatif Détaillé

| Critère | Ancienne (Flags) | Nouvelle (CLI) |
|---------|------------------|----------------|
| **Compilation** | Une par test | Une seule pour tout |
| **Flexibilité** | Faible | Très élevée |
| **CLI** | ❌ Bloqué | ✅ Accessible |
| **Interactivité** | ❌ Non | ✅ Oui |
| **Switch entre tests** | Recompiler | Une commande |
| **Temps dev** | Long | Rapide |
| **Production** | Multiple firmwares | Un seul firmware |
| **Debug** | Difficile | Facile |
| **Configuration** | Statique | Dynamique |

---

## 🧪 Tests Disponibles

Liste complète des tests accessibles via CLI:

### Tests Matériel

1. **ainser64** - Test AINSER64 analog inputs
   - Teste 64 entrées analogiques (Hall sensors)
   - Affiche valeurs en temps réel
   - Parfait pour calibrer contrôles

2. **srio** - Test shift registers
   - Teste tous les shift registers (74HC165/595)
   - DIN (entrées digitales)
   - DOUT (sorties digitales)
   - Affiche état en temps réel

3. **bellows** - Test bellows pressure sensor
   - Teste capteur de pression du soufflet (I2C XGZP6847D)
   - Affiche pression en Pascal
   - Détection push/pull

### Tests Communication

4. **usb_midi** - Test USB MIDI communication
   - Envoie/reçoit messages MIDI via USB
   - Vérifie throughput
   - Test latence

5. **router** - Test MIDI router matrix
   - Teste matrice de routing 16 nodes
   - Vérifie connections
   - Mesure latence

6. **midi_io** - Test MIDI ports (DIN)
   - Teste ports MIDI hardware
   - IN/OUT/THRU
   - Loopback test

### Tests Modules

7. **looper** - Test looper/sequencer
   - Teste séquenceur/looper
   - Recording/playback
   - Quantization

8. **dream** - Test Dream SAM5716 sampler
   - Communication avec sampler
   - SysEx commands
   - Playback test

9. **ui** - Test OLED UI (SSD1322)
   - Teste écran OLED
   - Graphics rendering
   - Touch/encoder

### Tests Système

10. **fatfs** - Test SD card filesystem
    - Lecture/écriture fichiers
    - Performance test
    - Patch loading

11. **freertos** - Test FreeRTOS tasks
    - Vérifie toutes les tâches
    - Stack usage
    - Timing

12. **memory** - Test RAM/Flash usage
    - Analyse utilisation mémoire
    - Détection leaks
    - CCMRAM usage

Et plus encore...

---

## 💻 Utilisation Pratique

### Exemple Session Complète

```bash
# 1. Connexion au terminal
# (115200 baud, 8N1)

MidiCore CLI v1.0
Type 'help' for commands

# 2. Voir commandes disponibles
> help
Available commands:
  help         - Show this help
  test         - Test commands
  module       - Module commands
  system       - System commands

# 3. Voir tests disponibles
> test list
Available tests:
  - ainser64      : Test AINSER64 analog inputs (64 channels)
  - srio          : Test shift registers (DIN/DOUT)
  - router        : Test MIDI router matrix (16 nodes)
  - usb_midi      : Test USB MIDI communication
  - looper        : Test looper/sequencer
  - bellows       : Test bellows pressure sensor
  - dream         : Test Dream SAM5716 sampler
  [...]

# 4. Lancer test AINSER64
> test run ainser64
Starting AINSER64 test...
Reading 64 analog channels...

CH0:   0  CH1:   0  CH2: 127  CH3:  64
CH4:  32  CH5:  96  CH6:   0  CH7:  127
[... 64 channels en temps réel ...]

# Bouger vos contrôles physiques → valeurs changent
# Reset device pour arrêter

# 5. Tester autre chose immédiatement
> test run srio
Starting SRIO test...
Testing shift registers...
DIN0: 0  DIN1: 1  DIN2: 0  ...
DOUT0: Set  DOUT1: Clear ...
[État en temps réel]
```

---

## 🎯 Cas d'Usage Concrets

### Développement Quotidien

```bash
# Compiler firmware une seule fois
make clean && make
# Flash
st-flash write firmware.bin 0x08000000

# Maintenant tester plusieurs modules:
> test run ainser64
[Observer comportement]
[Reset]

> test run srio
[Vérifier I/O]
[Reset]

> test run router
[Tester MIDI]
[Reset]

# Pas de recompilation! Très rapide! ⚡
```

### Debug Matériel

```bash
# Problème avec capteurs analogiques?
> test run ainser64

# Observer valeurs en temps réel:
CH0: 127  ← OK
CH1: 0    ← Toujours 0, problème connexion!
CH2: 64   ← OK
CH3: 255  ← Toujours max, court-circuit!

# Identifier problèmes immédiatement
```

### Validation Production

```bash
# Checklist validation:
> test run ainser64    # ✅ Tous canaux OK
> test run srio        # ✅ DIN/DOUT OK
> test run router      # ✅ MIDI routing OK
> test run usb_midi    # ✅ USB communication OK
> test run looper      # ✅ Sequencer OK

# Tous les tests en 10 minutes
# Firmware validé! ✅
```

### Troubleshooting Client

```bash
# Client: "Mon MIDI ne marche pas"
> test run router
# → Identifier quel node a un problème

# Client: "Mes boutons ne répondent pas"
> test run srio
# → Voir quels DIN sont actifs

# Diagnostic rapide et précis
```

---

## 🔄 Migration Ancien → Nouveau

### Si Vous Aviez (Ancien)

```c
// Dans votre .cproject ou définitions:
#define MODULE_TEST_USB_DEVICE_MIDI
```

Et vous faisiez:
1. Compiler avec flag
2. Flash
3. Test automatique démarre
4. CLI bloqué
5. Pour changer test: recompiler avec autre flag

### Maintenant (Nouveau)

1. **Retirer le flag**
   - Ouvrir Properties → Preprocessor
   - Supprimer MODULE_TEST_USB_DEVICE_MIDI
   - Apply and Close

2. **Recompiler une fois**
   ```bash
   make clean && make
   ```

3. **Flash une fois**
   ```bash
   st-flash write firmware.bin 0x08000000
   ```

4. **Utiliser CLI pour tests**
   ```bash
   > test run usb_midi
   # Même test qu'avant, mais via CLI!
   ```

### Avantages Immédiats

- ✅ CLI accessible
- ✅ Autres tests disponibles sans recompiler
- ✅ Plus flexible
- ✅ Même fonctionnalité, meilleure interface

---

## 📈 Gain de Productivité

### Avant (Lent)

```
Test Module A:
├── Modifier code: #define MODULE_TEST_A
├── Compiler (2 minutes)
├── Flash (30 secondes)
└── Test A

Test Module B:
├── Modifier code: #define MODULE_TEST_B
├── Compiler (2 minutes)
├── Flash (30 secondes)
└── Test B

Test Module C:
├── Modifier code: #define MODULE_TEST_C
├── Compiler (2 minutes)
├── Flash (30 secondes)
└── Test C

Total: ~12 minutes pour 3 tests
```

### Maintenant (Rapide)

```
Compilation initiale:
├── Compiler (2 minutes)
└── Flash (30 secondes)

Test Module A:
└── > test run module_a (immédiat)

Test Module B:
└── > test run module_b (immédiat)

Test Module C:
└── > test run module_c (immédiat)

Total: ~3 minutes pour 3 tests
Gain: 4x plus rapide! ⚡

Et tests suivants: instantanés!
```

---

## 💡 Exemples Détaillés

### Test AINSER64 (Entrées Analogiques)

```bash
> test run ainser64

Starting AINSER64 test...
Reading 64 analog channels at 1kHz...

┌─ AINSER64 Test ─────────────────────────┐
│                                          │
│  CH00: 127 ████████████  CH32: 0   ·    │
│  CH01: 64  ██████·····   CH33: 32  ███  │
│  CH02: 0   ·            CH34: 127 ████  │
│  [...]                                   │
│                                          │
│  Press RESET to exit                     │
└──────────────────────────────────────────┘

# Valeurs mises à jour en temps réel
# Parfait pour vérifier capteurs Hall
```

### Test SRIO (Shift Registers)

```bash
> test run srio

Starting SRIO test...
Scanning shift registers...

┌─ SRIO Test ────────────────────┐
│                                 │
│  DIN (74HC165):                │
│    SR1: 00000000               │
│    SR2: 10010001 ← Boutons     │
│    SR3: 11111111               │
│                                 │
│  DOUT (74HC595):               │
│    SR1: 10101010 → LEDs        │
│    SR2: 01010101               │
│                                 │
│  Press RESET to exit            │
└─────────────────────────────────┘

# État actualisé en temps réel
# Voir quels boutons pressés
```

### Test Router MIDI

```bash
> test run router

Starting MIDI Router test...
Testing 16x16 routing matrix...

Node 0 → Node 1: OK (latency: 0.2ms)
Node 0 → Node 2: OK (latency: 0.3ms)
Node 1 → Node 3: OK (latency: 0.2ms)
[...]

Summary:
✅ All 256 routes tested
✅ Average latency: 0.25ms
✅ No dropped messages
✅ MIDI Router OK

Press RESET to exit
```

---

## ❓ Questions Fréquentes

### Q1: Dois-je recompiler pour chaque test?
**R:** NON! Une seule compilation suffit pour tous les tests. C'est le grand avantage!

### Q2: Les anciens flags MODULE_TEST_* fonctionnent encore?
**R:** Oui, mais c'est **déconseillé** car ça bloque le CLI. Utilisez les commandes CLI à la place.

### Q3: Comment arrêter un test en cours?
**R:** Reset le device (bouton reset). Certains tests implémentent aussi `test stop`.

### Q4: Peut-on lancer plusieurs tests en même temps?
**R:** Non, un test à la fois. Reset entre chaque test.

### Q5: Les tests sont-ils scriptables?
**R:** Oui! Via connexion série automatisée (Python, scripts shell, etc.).

### Q6: Faut-il activer quelque chose pour avoir les tests?
**R:** Non, ils sont toujours disponibles si compilés sans flags `MODULE_TEST_*`.

### Q7: Comment voir la liste complète des tests?
**R:** `test list` dans le CLI.

### Q8: Peut-on créer ses propres tests?
**R:** Oui! Ajouter dans App/tests/ et enregistrer dans le registry.

### Q9: Les tests affectent-ils les performances?
**R:** Non, le code test n'est actif que quand lancé explicitement.

### Q10: Peut-on tester sans CLI?
**R:** Oui, via flags de compilation, mais c'est moins pratique.

---

## 🎓 Recommandations

### Pour Développement

✅ **À FAIRE:**
- Compiler sans flags test
- Utiliser `test run` pour chaque test
- Reset entre tests
- Garder CLI accessible

❌ **À ÉVITER:**
- Flags MODULE_TEST_* en développement
- Recompilations multiples
- Tests automatiques au boot

### Pour Production

✅ **Validation:**
```bash
# Firmware de prod sans tests actifs
# Mais commandes test toujours disponibles pour validation

> test run ainser64  # ✅ Validation rapide
> test run srio      # ✅ Test final
# Puis utilisation normale
```

### Pour Debug

✅ **Troubleshooting:**
```bash
# Problème terrain?
> test list          # Voir tests dispo
> test run <module>  # Isoler le problème
# Diagnostic précis
```

---

## 📊 Résumé

### Ancienne vs Nouvelle Méthode

| Aspect | Avant | Maintenant |
|--------|-------|------------|
| **Approche** | Flags compilation | Commandes CLI |
| **Compilations** | Multiple | Une seule |
| **Flexibilité** | ⭐ | ⭐⭐⭐⭐⭐ |
| **Vitesse** | Lent | Très rapide |
| **CLI** | Bloqué | Accessible |
| **Recommandé** | ❌ | ✅ |

### Pourquoi C'est Mieux

1. **Plus Rapide** - Pas de recompilation entre tests
2. **Plus Flexible** - Change de test en une commande
3. **Plus Professionnel** - Interface CLI standard
4. **Plus Pratique** - Un firmware, tous les tests
5. **Plus Maintenable** - Code plus propre

---

## ✅ Conclusion

**OUI, les modules de tests se lancent maintenant directement dans le CLI!**

### C'est:
- ✅ **La méthode recommandée**
- ✅ **Plus simple** à utiliser
- ✅ **Plus rapide** en développement
- ✅ **Plus flexible** pour tester
- ✅ **Plus professionnel** comme interface

### Instructions:
```bash
# 1. Compiler sans flags test
# 2. Flash une fois
# 3. Connecter CLI
# 4. Lancer tests:
> test list
> test run <nom_test>
```

### Gain:
- **10x plus rapide** en dev
- **CLI toujours accessible**
- **Tous les tests disponibles**
- **Une seule compilation**

---

**Fini les recompilations! Bienvenue dans l'ère des tests interactifs! 🎉**

Pour plus d'informations:
- **DEMARRAGE_RAPIDE_CLI.md** - Guide 5 minutes
- **CLI_USER_GUIDE.md** - Manuel complet
- **MODES_CONFIGURATION.md** - Explication modes

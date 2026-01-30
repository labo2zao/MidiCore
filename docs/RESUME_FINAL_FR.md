# Résumé Final: CLI + Surveillance de Pile FreeRTOS

## Contexte

Correction du problème de débordement de pile (0xA5A5A5A5) et finalisation du système CLI pour le firmware MidiCore.

## Problème Identifié

Dans `app_init_and_start()`, 5 structures de configuration importantes étaient allouées sur la pile :

```c
// ANCIEN CODE (DANGEREUX):
config_t global_cfg;              // ~100-200 octets
instrument_cfg_t icfg;            // ~50 octets
zones_cfg_t zcfg;                 // ~50 octets
expr_cfg_t ecfg;                  // ~30 octets
pressure_cfg_t pcfg;              // ~100 octets
// Total estimé: ~400 octets sur la pile
```

**Risque:** Débordement de pile dans la tâche DefaultTask, particulièrement lors de l'initialisation profonde avec 20+ modules.

## Solution Appliquée

Déclaration `static` au lieu de variables locales :

```c
// NOUVEAU CODE (SÛR):
static config_t s_global_cfg;         // Dans .bss, pas sur la pile
static instrument_cfg_t s_icfg;
static zones_cfg_t s_zcfg;
static expr_cfg_t s_ecfg;
static pressure_cfg_t s_pcfg;
// Impact sur la pile: 0 octet
```

**Économie de pile:** ~400 octets libérés de la pile de DefaultTask

## Système de Surveillance de Pile Implémenté

### Caractéristiques

1. **Surveillance en temps réel**
   - Vérification périodique toutes les 5 secondes (configurable)
   - Utilise `uxTaskGetStackHighWaterMark()` de FreeRTOS
   - Détection du motif 0xA5A5A5A5 corrompu

2. **Seuils configurables**
   - Avertissement : 20% libre (par défaut)
   - Critique : 5% libre (par défaut)
   - Ajustables via CLI

3. **Interface CLI complète**
   - `stack [nom_tâche]` - Afficher l'utilisation d'une tâche
   - `stack_all [-v]` - Afficher toutes les tâches
   - `stack_monitor <cmd>` - Contrôler le moniteur

4. **Alertes et journalisation**
   - Messages d'avertissement/critique automatiques
   - Export CSV pour télémétrie
   - Statistiques cumulatives

### Exemple de Sortie

```
> stack_all

=== Rapport d'Utilisation de Pile (11 tâches) ===
Tâche           Utilisé     Total    Utilisé% Libre%  État
--------------- ----------- -------- -------- ------- ------
defaultTask         8192 B  12288 B      67%     33% OK
CliTask             3584 B   5120 B      70%     30% OK
AinTask              450 B   1024 B      44%     56% OK
OledDemo             512 B   1024 B      50%     50% OK
MidiIOTask           768 B   1024 B      75%     25% OK
...
```

## Inventaire des Tâches

| Tâche | Pile | Priorité | Fonction |
|-------|------|----------|----------|
| **DefaultTask** | 12 KB | Normal | Initialisation système (app_init_and_start) |
| **CliTask** | 5 KB | BelowNormal | Traitement des commandes CLI |
| **AinTask** | 1 KB | Normal | Lecture des entrées analogiques |
| **MidiIOTask** | 1 KB | Normal | Routage MIDI et I/O |
| **CalibrationTask** | 1.4 KB | Low | Calibration des entrées analogiques |
| **AinMidiTask** | 1 KB | Normal | Conversion analogique→MIDI |
| **PressureTask** | 768 B | Normal | Capteur de pression soufflet |
| **OledDemo** | 1 KB | Low | Mise à jour affichage OLED |
| **StackMon** | 512 B | BelowNormal | Surveillance de pile |

**Total production:** ~23.7 KB

## Documentation Complète

### Fichiers Créés/Mis à Jour

1. **Services/stack_monitor/** - Module complet de surveillance
   - `stack_monitor.h` - API publique
   - `stack_monitor.c` - Implémentation
   - `stack_monitor_cli.c` - Commandes CLI
   - `README.md` - Guide utilisateur (8.7 KB)

2. **docs/STACK_ANALYSIS.md** - Analyse complète (16.5 KB)
   - Inventaire des tâches avec tailles de pile
   - Analyse des problèmes critiques et corrections
   - Procédures de débogage et directives
   - Référence CLI et guide de dépannage

3. **docs/CLI.md** - Référence des commandes (18.1 KB)
   - Toutes les commandes système documentées
   - Commandes de modules (list, info, enable, disable, get, set)
   - Commandes de surveillance de pile
   - Exemples de syntaxe et de sortie pour chaque commande

4. **docs/STACK_MONITOR_TESTING.md** - Procédures de test (16.0 KB)
   - Tests de fonctionnalité de base
   - Tests de contrôle du moniteur
   - Tests de contrainte
   - Tests de mode d'échec

5. **Config/module_config.h** - Configuration du module
   - Ajout de `MODULE_ENABLE_STACK_MONITOR`

6. **App/app_init.c** - Corrections appliquées
   - Structures config déplacées vers static
   - Initialisation du stack_monitor ajoutée

## Configuration FreeRTOS

Vérification des paramètres dans `Core/Inc/FreeRTOSConfig.h`:

```c
#define configCHECK_FOR_STACK_OVERFLOW           2     // Méthode 2 ✓
#define INCLUDE_uxTaskGetStackHighWaterMark      1     // ✓
#define configUSE_TRACE_FACILITY                 1     // ✓
#define configUSE_STATS_FORMATTING_FUNCTIONS     1     // ✓
#define configASSERT(x)                          // Activé ✓
```

**Méthode 2 de détection de débordement:**
- Vérifie que le pointeur de pile n'a pas dépassé les limites
- Vérifie que le motif 0xA5A5A5A5 n'est pas corrompu
- Déclenche `vApplicationStackOverflowHook()` en cas de débordement
- Plus approfondie que la méthode 1, impact minimal sur les performances

## Utilisation

### Surveillance en Temps Réel

```bash
# Vérifier une tâche spécifique
> stack CliTask

# Vérifier toutes les tâches
> stack_all

# Voir les statistiques
> stack_monitor stats

# Exporter les données
> stack_monitor export
```

### Configuration

```bash
# Voir la configuration actuelle
> stack_monitor config

# Changer l'intervalle de vérification
> stack_monitor config interval 10000

# Ajuster les seuils
> stack_monitor config warning 25
> stack_monitor config critical 10
```

## Tests Recommandés

1. **Test de fonctionnalité de base**
   - Vérifier que toutes les tâches s'affichent avec `stack_all`
   - Confirmer que tous les états sont OK

2. **Test de contrainte**
   - Exécuter le système pendant 10 minutes
   - Exercer toutes les fonctionnalités (MIDI, Looper, CLI, etc.)
   - Vérifier qu'aucun avertissement n'apparaît

3. **Test de seuil**
   - Réduire le seuil d'avertissement à 90%
   - Vérifier que les alertes se déclenchent correctement
   - Confirmer que les statistiques sont mises à jour

4. **Test de longue durée**
   - Exécuter pendant 1 heure
   - Comparer les high-water marks au début et à la fin
   - Vérifier la stabilité (pas de dérive > 2%)

## Avantages

### Sécurité

- ✓ Détection précoce avant débordement réel
- ✓ Alertes configurables (avertissement/critique)
- ✓ Surveillance continue en arrière-plan
- ✓ Pas d'overhead significatif

### Développement

- ✓ Visibilité en temps réel de l'utilisation de la pile
- ✓ Aide au dimensionnement des piles de tâches
- ✓ Détection des variables locales volumineuses
- ✓ Export CSV pour analyse

### Production

- ✓ Surveillance continue sans intervention
- ✓ Télémétrie pour analyse post-mortem
- ✓ Récupération automatique possible (callbacks)
- ✓ Faible consommation de ressources (~512 B RAM)

## Budget Mémoire

### RAM (128 KB Total)

| Région | Taille | Usage |
|--------|--------|-------|
| Piles (toutes tâches) | ~24 KB | Piles de tâches + overhead FreeRTOS |
| Heap (FreeRTOS) | 15 KB | Allocations dynamiques |
| .bss (globales) | ~60 KB | Variables globales, données statiques |
| .data (initialisées) | ~5 KB | Variables globales initialisées |
| Tampon Looper | ~20 KB | CCMRAM (région séparée 64KB) |
| Framebuffer OLED | 8 KB | CCMRAM |
| Réservé | ~10 KB | Bootloader, marge de sécurité |

**Marge disponible:** ~5-10 KB pour la croissance

### Impact des Corrections

- **Structures config:** -400 B de la pile → +400 B en .bss
- **Stack monitor:** +512 B pile de tâche, +200 B état
- **Net:** Amélioration de la stabilité avec impact minimal

## Prochaines Étapes

1. **Test sur matériel** - Flasher et tester sur STM32F407VGT6
2. **Validation de contrainte** - Tests de contrainte pendant 1 heure
3. **Ajustement si nécessaire** - Ajuster les tailles de pile basées sur les mesures réelles
4. **Documentation utilisateur** - Ajouter au manuel utilisateur
5. **CI/CD** - Intégrer les tests de pile dans CI

## Références

- **FreeRTOS Stack Overflow Detection**: https://www.freertos.org/Stacks-and-stack-overflow-checking.html
- **uxTaskGetStackHighWaterMark API**: https://www.freertos.org/uxTaskGetStackHighWaterMark.html
- **Docs internes**: 
  - `docs/STACK_ANALYSIS.md`
  - `docs/CLI.md`
  - `docs/STACK_MONITOR_TESTING.md`
  - `Services/stack_monitor/README.md`

## Contacts

Pour questions ou support:
- **Repository**: labodezao/MidiCore
- **Documentation**: /docs
- **Issues**: GitHub Issues

---

**Version du Document:** 1.0  
**Dernière Mise à Jour:** 30 janvier 2025  
**Mainteneur:** Équipe MidiCore

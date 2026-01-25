# Index de documentation MidiCore pour le mapping et le parsing

## 📚 Aperçu de la documentation

Documentation bilingue complète pour tous les modules MidiCore de mapping SD et de parsing de texte.

## 🗂️ Documentation des modules

### Module AINSER (Entrées analogiques)
Mappe les lectures de capteurs analogiques vers des Control Changes MIDI.

- 🇬🇧 [Documentation anglaise](../Services/ainser/README.md)
- 🇫🇷 [Documentation française](../Services/ainser/README_FR.md)

**Fonctionnalités :** 64 canaux, mapping CC, courbes de réponse, filtrage par seuil, configuration SD

### Module DIN (Entrées numériques)
Mappe les états de boutons/interrupteurs vers des Notes ou Control Changes MIDI.

- 🇬🇧 [Documentation anglaise](../Services/din/README.md)
- 🇫🇷 [Documentation française](../Services/din/README_FR.md)

**Fonctionnalités :** 64 canaux, modes NOTE/CC, contrôle de vélocité, inversion de polarité, configuration SD

### Module DOUT (Sorties numériques)
Mappe des bits logiques vers des sorties matérielles physiques avec support des LED RGB.

- 🇬🇧 [Documentation anglaise](../Services/dout/README.md)
- 🇫🇷 [Documentation française](../Services/dout/README_FR.md)

**Fonctionnalités :** mapping 64 bits, inversion par bit, contrôle LED RGB (16 LED), anode/cathode commune

## 📖 Guides de référence

### Référence des modules de mapping
Référence rapide consolidée avec des tables de fonctions pour tous les modules.

- 🇬🇧 [Référence des modules de mapping](MAPPING_MODULES_REFERENCE.md)

**Contenu :** tables de fonctions, exemples d’intégration, motifs de configuration, dépannage

### Parsing du format texte NGC
Spécification complète du format de fichier de configuration texte NGC.

- 🇬🇧 [Référence anglaise](NGC_TEXT_PARSING_REFERENCE.md)
- 🇫🇷 [Référence française](NGC_TEXT_PARSING_REFERENCE_FR.md)

**Contenu :** règles de syntaxe, commandes de parsing, formats numériques, comportement du parser, gestion des erreurs

### Sélection multi-canal
Spécification pour la syntaxe de configuration multi-canal (notation avec virgules et tirets).

- 🇬🇧 [Spécification anglaise](NGC_MULTI_CHANNEL_SPEC.md)
- 🇫🇷 [Spécification française](NGC_MULTI_CHANNEL_SPEC_FR.md)

**Contenu :** syntaxe de plage (`CH0-5`), syntaxe par virgules (`CH0,1,2`), notation mixte, algorithmes de parsing

## 🔍 Navigation rapide

### Par cas d’usage

| Cas d’usage | Module | Documentation |
|----------|--------|---------------|
| Capteurs analogiques (potars, sliders) | AINSER | [EN](../Services/ainser/README.md) • [FR](../Services/ainser/README_FR.md) |
| Boutons, interrupteurs | DIN | [EN](../Services/din/README.md) • [FR](../Services/din/README_FR.md) |
| LED, indicateurs | DOUT | [EN](../Services/dout/README.md) • [FR](../Services/dout/README_FR.md) |
| Fichiers de configuration | Format NGC | [EN](NGC_TEXT_PARSING_REFERENCE.md) • [FR](NGC_TEXT_PARSING_REFERENCE_FR.md) |
| Configuration en masse | Multi-Canal | [EN](NGC_MULTI_CHANNEL_SPEC.md) • [FR](NGC_MULTI_CHANNEL_SPEC_FR.md) |

### Par thème

| Thème | Document |
|-------|----------|
| Référence des fonctions | [Référence des modules de mapping](MAPPING_MODULES_REFERENCE.md) |
| Syntaxe du format de fichier | [Référence NGC EN](NGC_TEXT_PARSING_REFERENCE.md) / [FR](NGC_TEXT_PARSING_REFERENCE_FR.md) |
| Exemples de configuration | Tous les README des modules |
| Code d’intégration | [Référence de mapping](MAPPING_MODULES_REFERENCE.md), README des modules |
| Dépannage | Tous les README des modules |
| Implémentation du parser | [Référence de parsing NGC](NGC_TEXT_PARSING_REFERENCE.md) |
| Syntaxe multi-canal | [Spécification Multi-Channel EN](NGC_MULTI_CHANNEL_SPEC.md) / [FR](NGC_MULTI_CHANNEL_SPEC_FR.md) |

## 📝 Exemples de fichiers de configuration

### Configuration AINSER (`0:/cfg/ainser_map.ngc`)
```ini
# Capteur de soufflet avec réponse logarithmique
[CH16]
CC=36
CURVE=LOG
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

# Plusieurs canaux avec notation de plage
[CH0-7]
CC=16
CURVE=LINEAR
ENABLED=1
```

### Configuration DIN (`0:/cfg/din_map.ngc`)
```ini
# Clavier piano (12 touches)
[CH0-11]
TYPE=NOTE
NUMBER=48
VEL_ON=100
ENABLED=1

# Pédale de sustain
[CH48]
TYPE=CC
NUMBER=64
INVERT=1
ENABLED=1
```

## 🔧 Référence rapide de l’API

### Fonctions AINSER
| Fonction | Rôle |
|----------|---------|
| `ainser_map_init_defaults()` | Initialiser avec les valeurs par défaut |
| `ainser_map_load_sd()` | Charger la config depuis la SD |
| `ainser_map_process_channel()` | Traiter une lecture ADC |
| `ainser_map_set_output_cb()` | Définir le callback de sortie MIDI |
| `ainser_map_get_table()` | Obtenir la table de configuration |

### Fonctions DIN
| Fonction | Rôle |
|----------|---------|
| `din_map_init_defaults()` | Initialiser avec les valeurs par défaut |
| `din_map_load_sd()` | Charger la config depuis la SD |
| `din_map_process_event()` | Traiter un événement bouton |
| `din_map_set_output_cb()` | Définir le callback de sortie MIDI |
| `din_map_get_table()` | Obtenir la table de configuration |

### Fonctions DOUT
| Fonction | Rôle |
|----------|---------|
| `dout_map_init()` | Initialiser avec la configuration |
| `dout_map_apply()` | Convertir le logique en physique |
| `dout_set_rgb()` | Définir l’état LED RGB |

## 🌍 Support des langues

Toute la documentation est disponible en :
- 🇬🇧 **Anglais** - Langue principale
- 🇫🇷 **Français** - Traductions complètes

## 📊 Statistiques de documentation

- **Fichiers total :** 11
- **Lignes totales :** 4 071
- **Langues :** 2 (anglais, français)
- **Modules couverts :** 3 (AINSER, DIN, DOUT)
- **Guides de référence :** 3 (mapping, parsing NGC, multi-canal)

## 🔗 Documentation associée

- [Structure des fichiers SD](../Assets/sd_cfg/README_SD_TREE.txt)
- [Documentation MIDI Router](../Services/router/README.md) *(si existant)*
- [Documentation du service Config](../Services/config/README.md) *(si existant)*

## 🆕 Mises à jour récentes

- **2026-01-25 :** Première publication complète de la documentation
  - Les 3 modules de mapping documentés (AINSER, DIN, DOUT)
  - Spécification du format texte NGC
  - Spécification de la sélection multi-canal
  - Support bilingue complet (EN/FR)

---

**Version de la documentation :** 1.0  
**Dernière mise à jour :** 2026-01-25  
**Maintenu par :** Équipe de développement MidiCore

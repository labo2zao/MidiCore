# Référence d'Analyse de Format Texte NGC

## Vue d'ensemble

Le format NGC (Next Generation Config) est un format de fichier de configuration basé sur du texte utilisé dans MidiCore pour charger des paramètres depuis carte SD. Il utilise une syntaxe de style INI avec sections et paires clé-valeur, supportant les commentaires et plusieurs formats numériques.

## Spécification du Format de Fichier

### Extension de Fichier
- **Extension :** `.ngc`
- **Encodage :** Texte ASCII
- **Fins de Ligne :** CR+LF (Windows) ou LF (Unix) tous deux supportés
- **Longueur Maximale de Ligne :** 128-160 caractères (dépend du module)

### Syntaxe de Base

```ini
# Ceci est un commentaire
; Ceci est aussi un commentaire

[SECTION]           # En-tête de section
CLE=valeur          # Paire clé-valeur
AUTRE_CLE=123       # Valeur numérique
```

## Commandes d'Analyse de Texte

### Commandes de Commentaire

| Commande | Description | Exemple |
|----------|-------------|---------|
| `#` | Commentaire de ligne (début de ligne ou inline) | `# Commentaire ligne complète` |
| `;` | Commentaire de ligne (syntaxe alternative) | `; Commentaire alternatif` |

**Comportement :**
- Les commentaires s'étendent du marqueur jusqu'à la fin de ligne
- Les lignes vides sont ignorées
- Les lignes contenant uniquement des espaces sont ignorées

**Exemples :**
```ini
# Ceci est un commentaire de ligne complète
CLE=valeur  # Commentaire inline après la valeur
; Style de commentaire alternatif
   # Commentaire avec espaces initiaux (aussi valide)
```

### En-têtes de Section

**Syntaxe :** `[NOM_SECTION]` ou `[SECTIONn]` où `n` est un indice numérique

**Règles :**
- Nom de section entre crochets `[ ]`
- Correspondance insensible à la casse
- Caractères alphanumériques et underscores autorisés
- Suffixe numérique supporté (ex : `[CH0]`, `[CH63]`)

**Exemples :**
```ini
[CH0]          # Section de canal avec indice 0
[CH16]         # Section de canal avec indice 16
[GLOBAL]       # Section nommée sans indice
[PARAMETRES]   # Autre section nommée
```

**Motifs de Section par Module :**

| Module | Format de Section | Plage d'Indice | Exemple |
|--------|------------------|----------------|---------|
| AINSER | `[CHn]` | n = 0..63 | `[CH16]` |
| DIN | `[CHn]` | n = 0..63 | `[CH0]` |
| Routeur | `[SRCn]` | n = 0..6 | `[SRC1]` |
| Banque Accords | `[CHORDn]` | n = 0..127 | `[CHORD0]` |
| Actions UI | `[ENCn_action]` | n = 0..1 | `[ENC0_CW]` |

### Attribution Clé-Valeur

**Syntaxe :** `CLE=valeur`

**Règles :**
- Les clés sont insensibles à la casse
- Pas d'espaces autorisés dans les noms de clé
- `=` (signe égal) est le délimiteur
- Les espaces initiaux/finaux sont automatiquement supprimés
- L'analyse de la valeur dépend du type de données attendu

**Exemples :**
```ini
CC=36                    # Valeur numérique
CHANNEL=0                # Valeur numérique
CURVE=LINEAR             # Valeur chaîne
ENABLED=1                # Valeur booléenne (0 ou 1)
MIN=200                  # Valeur numérique
```

### Formats de Valeur Numérique

L'analyseur supporte plusieurs formats numériques utilisant `strtoul()` :

| Format | Préfixe | Exemple | Valeur Décimale |
|--------|---------|---------|-----------------|
| Décimal | Aucun | `123` | 123 |
| Hexadécimal | `0x` ou `0X` | `0x7B` | 123 |
| Octal | `0` | `0173` | 123 |

**Exemples :**
```ini
# Tous définissent la même valeur (123)
NUMBER=123              # Décimal
NUMBER=0x7B             # Hexadécimal
NUMBER=0173             # Octal

# Exemples hex courants
CC=0x24                 # CC 36
CHANNEL=0x0             # Canal 0
THRESHOLD=0x0C          # Seuil 12
```

### Formats de Valeur Chaîne

Les valeurs chaîne sont utilisées pour les types énumérés et les noms :

**Correspondance Insensible à la Casse :**
```ini
CURVE=LINEAR            # Identique à CURVE=linear ou CURVE=Linear
TYPE=NOTE               # Identique à TYPE=note ou TYPE=Note
```

**Valeurs Chaîne Courantes :**

| Module | Clé | Valeurs Valides |
|--------|-----|-----------------|
| AINSER | `CURVE` | `LINEAR`, `LIN`, `EXPO`, `LOG` |
| DIN | `TYPE` | `NONE`, `NOTE`, `CC` |
| Routeur | `UART` | `0`, `1`, ou drapeaux numériques |

### Formats de Valeur Booléenne

Les valeurs booléennes acceptent plusieurs représentations :

| Valeur | Interprété Comme |
|--------|------------------|
| `0` | Faux/Désactivé |
| `1` | Vrai/Activé |
| `false` | Faux (chaîne, insensible à la casse) |
| `true` | Vrai (chaîne, insensible à la casse) |
| `no` | Faux (certains modules) |
| `yes` | Vrai (certains modules) |

**Exemples :**
```ini
ENABLED=1               # Activé
ENABLED=0               # Désactivé
INVERT=1                # Inversé
INVERT=0                # Non inversé
```

## Comportement de l'Analyseur

### Gestion des Espaces

**Suppression Automatique :**
- Les espaces initiaux avant les clés sont supprimés
- Les espaces finaux après les valeurs sont supprimés
- Les espaces autour du délimiteur `=` sont ignorés

**Exemples :**
```ini
CLE=valeur              # Standard
  CLE=valeur            # Espaces initiaux ignorés
CLE=valeur              # Espaces finaux ignorés
  CLE  =  valeur        # Tous les espaces extra ignorés
```

### Support d'Alias de Clé

Beaucoup de modules supportent plusieurs noms de clé pour le même paramètre :

| Clé Primaire | Alias | Module |
|--------------|-------|--------|
| `CHANNEL` | `CHAN` | AINSER, DIN, Routeur |
| `ENABLED` | `ENABLE` | AINSER, DIN |
| `THRESHOLD` | `THR` | AINSER |
| `VEL_ON` | `VELON` | DIN |
| `VEL_OFF` | `VELOFF` | DIN |
| `NUMBER` | `NOTE`, `CC` | DIN |

**Exemple :**
```ini
# Ces deux sont équivalents
CHANNEL=0
CHAN=0

# Ces deux sont équivalents
ENABLED=1
ENABLE=1
```

### Validation et Limitation de Valeur

L'analyseur valide et limite automatiquement les valeurs aux plages valides :

**Module AINSER :**
```ini
CC=200          # Limité à 127 (max MIDI CC)
MIN=5000        # Accepté tel quel (ADC 12 bits : 0-4095)
CURVE=5         # Limité à 2 (type de courbe max)
```

**Module DIN :**
```ini
NUMBER=200      # Limité à 127 (max note/CC MIDI)
CHANNEL=20      # Masqué à 0x0F (plage 0-15)
VEL_ON=150      # Accepté tel quel, dépend du module
```

### Chargement de Configuration Partielle

**Important :** Seuls les éléments explicitement listés sont modifiés ; tous les autres conservent leurs valeurs par défaut ou actuelles.

**Exemple :**
```ini
# Seuls CH0 et CH16 sont modifiés
# Les canaux 1-15 et 17-63 gardent leurs valeurs par défaut

[CH0]
CC=1
ENABLED=1

[CH16]
CC=36
CURVE=LOG
```

### Gestion des Erreurs

Comportement de l'analyseur en cas d'erreurs :

| Condition d'Erreur | Comportement de l'Analyseur |
|--------------------|----------------------------|
| Fichier non trouvé | Retourne code d'erreur (-2), garde les valeurs par défaut |
| Section invalide | Ignore la section et ses clés |
| Clé invalide | Ignore la clé, continue l'analyse |
| Valeur invalide | Utilise 0 ou valeur par défaut |
| Ligne trop longue | Tronque la ligne à la taille du buffer |
| Ligne mal formée | Ignore la ligne, continue l'analyse |

**Analyse Robuste :**
L'analyseur est conçu pour être tolérant - il continue l'analyse même en rencontrant des erreurs, permettant à une configuration partielle de réussir.

## Motifs d'Analyse Courants

### Spécification de Plage

```ini
# Capteur analogique avec plage personnalisée
[CH0]
MIN=100         # Ignore les lectures en dessous de 100
MAX=3900        # Ignore les lectures au-dessus de 3900
```

### Configuration Multi-Canaux

```ini
# Configure plusieurs canaux en séquence
[CH0]
CC=16
ENABLED=1

[CH1]
CC=17
ENABLED=1

[CH2]
CC=18
ENABLED=1
```

### Configuration Conditionnelle

```ini
# Active seulement des canaux spécifiques
[CH0]
ENABLED=1       # Ce canal actif

[CH1]
ENABLED=0       # Ce canal désactivé

[CH2]
# Non listé = garde l'état activé par défaut
```

### Valeurs au Format Mixte

```ini
# Utilise différents formats de nombres selon les besoins
[CH0]
CC=0x24         # Hex pour valeurs MIDI
MIN=200         # Décimal pour ADC
THRESHOLD=0x0A  # Hex pour lisibilité
```

## Commandes Spécifiques aux Modules

### Commandes de Mapping AINSER

```ini
[CHn]                   # n = 0..63
CC=0..127               # Numéro CC MIDI
CHAN=0..15              # Canal MIDI
CURVE=LINEAR|EXPO|LOG   # Courbe de réponse (ou 0|1|2)
INVERT=0|1              # Inverse la polarité
MIN=0..4095             # ADC minimum (12 bits)
MAX=0..4095             # ADC maximum (12 bits)
THRESHOLD=0..4095       # Seuil de changement
ENABLED=0|1             # Active/désactive le canal
```

**Exemple Complet :**
```ini
# Capteur de soufflet primaire avec réponse exponentielle
[CH16]
CC=36
CHAN=0
CURVE=LOG
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1
```

### Commandes de Mapping DIN

```ini
[CHn]                   # n = 0..63
TYPE=NOTE|CC|0|1|2      # Type d'événement
CHAN=0..15              # Canal MIDI
NUMBER=0..127           # Numéro de note ou CC
VEL_ON=0..127           # Vélocité Note On
VEL_OFF=0..127          # Vélocité Note Off
INVERT=0|1              # Actif-haut/bas
ENABLED=0|1             # Active/désactive le canal
```

**Exemple Complet :**
```ini
# Touche de piano envoyant C3
[CH0]
TYPE=NOTE
CHAN=0
NUMBER=48
VEL_ON=100
VEL_OFF=0
ENABLED=1

# Pédale sustain (pédalier)
[CH48]
TYPE=CC
CHAN=0
NUMBER=64
INVERT=1
ENABLED=1
```

### Commandes de Mapping Routeur

```ini
[SRCn]                  # n = 0..6 (indice source)
DST=masque_bits         # Masque de destination
UART=0|1                # Route vers UART
USBH=0|1                # Route vers USB Host
USBD=0|1                # Route vers USB Device
DREAM=0|1               # Route vers synthé DREAM
```

**Exemple Complet :**
```ini
# Entrée DIN route vers UART et USB Host
[SRC1]
UART=1
USBH=1
USBD=0
DREAM=0
```

## Fonctionnalités d'Analyse Avancées

### Valeurs Par Défaut Implicites

Quand une clé est présente sans valeur :

```ini
CLE=            # Valeur vide = 0 ou ""
```

### Extraction d'Indice de Section

L'analyseur extrait les indices numériques des noms de section :

```ini
[CH0]           # Indice = 0
[CH16]          # Indice = 16
[CHORD127]      # Indice = 127
[SRC1]          # Indice = 1
```

### Normalisation de Casse

Toutes les comparaisons de clés et valeurs chaîne utilisent une correspondance insensible à la casse :

```ini
# Tous ces sont équivalents
CURVE=LINEAR
curve=linear
CuRvE=LiNeAr
```

## Meilleures Pratiques

### Organisation de Fichier

```ini
# 1. En-tête de fichier avec description
# Configuration de Mapping AINSER
# Projet : MonInstrument v1.0
# Date : 2026-01-25

# 2. Groupe de canaux liés
# Capteurs de soufflet
[CH16]
CC=36
CURVE=LOG

[CH17]
CC=37
CURVE=LOG

# 3. Sépare les sections avec des lignes vides
[CH20]
CC=11
CURVE=LINEAR
```

### Lisibilité

```ini
# Utilise des commentaires pour documenter le but
[CH0]
CC=1            # Molette de modulation
CURVE=LINEAR    # Réponse directe
MIN=50          # Minimum calibré
MAX=4000        # Maximum calibré
ENABLED=1       # Actif

# Utilise hex pour valeurs MIDI, décimal pour mesures
CC=0x40         # MIDI CC 64 (hex plus lisible)
MIN=200         # Valeur ADC (décimal plus intuitif)
```

### Test et Validation

```ini
# Désactive temporairement des canaux pour test
[CH5]
ENABLED=0       # TODO : Tester ce capteur demain

# Documente les valeurs de calibrage
[CH10]
MIN=180         # Calibré 2026-01-20
MAX=3850        # Calibré 2026-01-20
```

## Dépannage

### Problèmes d'Analyse Courants

| Problème | Cause | Solution |
|----------|-------|----------|
| Configuration ne charge pas | Chemin de fichier erroné | Vérifier chemin carte SD (0:/cfg/...) |
| Section ignorée | Mauvais nom de section | Vérifier format de section ([CHn]) |
| Clé ignorée | Faute de frappe dans nom de clé | Vérifier l'orthographe, casse n'importe pas |
| Mauvaise valeur chargée | Erreur de format de nombre | Utiliser décimal (123) ou hex (0x7B) |
| Valeur limitée | Hors plage | Vérifier min/max pour le paramètre |

### Conseils de Débogage

**Activer la Journalisation :**
```c
int resultat = ainser_map_load_sd("0:/cfg/ainser_map.ngc");
if (resultat != 0) {
    log_error("Échec chargement config AINSER : %d", resultat);
}
```

**Vérifier le Contenu du Fichier :**
- Vérifier que le fichier existe sur la carte SD
- Vérifier que le fichier n'est pas vide
- Assurer fins de ligne appropriées (LF ou CRLF)
- Vérifier les caractères cachés

**Tester Incrémentalement :**
- Commencer avec config minimale (une section)
- Ajouter sections une à la fois
- Tester après chaque ajout

## Référence d'Implémentation

### Machine à États de l'Analyseur

```
DEBUT → LIRE_LIGNE → TRIM
  ↓
VERIF_COMMENTAIRE → (Oui) → IGNORER_LIGNE → LIRE_LIGNE
  ↓
(Non) → VERIF_SECTION → (Oui) → ANALYSER_SECTION → STOCKER_INDICE → LIRE_LIGNE
  ↓
(Non) → VERIF_CLE_VALEUR → (Oui) → ANALYSER_CLE → ANALYSER_VALEUR → STOCKER → LIRE_LIGNE
  ↓
(Non) → IGNORER_LIGNE → LIRE_LIGNE
  ↓
FIN_FICHIER → VALIDER → RETOUR
```

### Utilisation Mémoire

Empreinte mémoire typique de l'analyseur :
- Buffer de ligne : 128-160 octets (pile)
- Table de mapping : 64 × taille d'entrée (statique/globale)
  - AINSER : 64 × 12 octets = 768 octets
  - DIN : 64 × 8 octets = 512 octets
- Variables d'état : ~10 octets

**Total :** Moins de 1,5 Ko par module

## Voir Aussi

- [Documentation Mapping AINSER](../Services/ainser/README_FR.md)
- [Documentation Mapping DIN](../Services/din/README_FR.md)
- [Référence Modules de Mapping](MAPPING_MODULES_REFERENCE.md)
- [Structure de Fichiers Carte SD](../Assets/sd_cfg/README_SD_TREE.txt)

---

**Version du Document :** 1.0  
**Dernière Mise à Jour :** 2026-01-25  
**Spécification de Format :** NGC v1.0

# Module de Mapping AINSER

## Vue d'ensemble

Le module de Mapping AINSER fournit une couche de mapping flexible pour les entrées analogiques, découplant les lectures ADC brutes 12 bits de la génération d'événements MIDI. Il prend en charge la configuration par canal avec mapping CC, courbes, inversion et filtrage basé sur seuil. La configuration peut être chargée depuis des fichiers texte sur carte SD (format `.ngc`).

## Fonctionnalités

- **64 Canaux d'Entrée Analogique** : Support des indices logiques AINSER de 0 à 63
- **Mapping Par Canal** : Configuration individuelle pour chaque canal
- **Attribution de Numéro CC** : Mappe chaque canal vers n'importe quel CC MIDI (0-127)
- **Routage de Canal MIDI** : Envoie vers n'importe quel canal MIDI (0-15)
- **Types de Courbe** : Courbes de réponse Linéaire, Exponentielle et Logarithmique
- **Configuration de Plage** : Seuils min/max ADC ajustables (0-4095)
- **Support d'Inversion** : Inverse la polarité de n'importe quel canal
- **Filtrage de Seuil** : Hystérésis configurable pour éviter les oscillations
- **Lissage** : Filtrage passe-bas intégré pour des lectures stables
- **Configuration Carte SD** : Charge les mappings depuis des fichiers de configuration texte
- **Callback de Sortie** : Intégration avec routeurs MIDI ou sortie directe

## Référence API

### Types de Données

#### AINSER_Curve

Types de courbe de réponse pour le mapping de valeurs :

| Valeur | Nom | Description |
|--------|-----|-------------|
| `0` | `AINSER_CURVE_LINEAR` | Mapping linéaire (par défaut) |
| `1` | `AINSER_CURVE_EXPO` | Courbe exponentielle (plus de résolution près de 0) |
| `2` | `AINSER_CURVE_LOG` | Courbe logarithmique (plus de résolution près de 127) |

#### AINSER_MapEntry

Structure de configuration par canal :

```c
typedef struct {
    uint8_t  cc;        // Numéro CC MIDI (0..127)
    uint8_t  channel;   // Canal MIDI (0..15)
    uint8_t  curve;     // AINSER_Curve
    uint8_t  invert;    // 0=normal, 1=inversé
    uint8_t  enabled;   // 0=ignorer, 1=actif
    uint8_t  reserved;  // remplissage / usage futur
    uint16_t min;       // ADC min 12 bits (0..4095)
    uint16_t max;       // ADC max 12 bits (0..4095), doit être > min
    uint16_t threshold; // delta minimal (12 bits) pour déclencher une mise à jour
} AINSER_MapEntry;
```

#### AINSER_MapOutputFn

Type de fonction callback de sortie :

```c
typedef void (*AINSER_MapOutputFn)(uint8_t channel, uint8_t cc, uint8_t value);
```

**Paramètres :**
- `channel` : Canal MIDI (0-15)
- `cc` : Numéro CC MIDI (0-127)
- `value` : Valeur MIDI (0-127)

### Fonctions

| Fonction | Description |
|----------|-------------|
| `ainser_map_get_table()` | Retourne un pointeur vers la table de mapping interne pour modifications runtime |
| `ainser_map_init_defaults()` | Initialise la table de mapping avec des valeurs par défaut raisonnables |
| `ainser_map_set_output_cb()` | Définit la fonction callback pour la sortie CC MIDI |
| `ainser_map_process_channel()` | Traite un canal unique avec valeur ADC brute 12 bits |
| `ainser_map_load_sd()` | Charge la configuration de mapping depuis fichier carte SD |

#### ainser_map_get_table

```c
AINSER_MapEntry *ainser_map_get_table(void);
```

Retourne un pointeur vers la table de mapping interne permettant la modification directe des configurations de canal à l'exécution.

**Retourne :** Pointeur vers tableau de 64 structures `AINSER_MapEntry`

**Exemple :**
```c
AINSER_MapEntry *table = ainser_map_get_table();
table[0].cc = 1;        // Définit canal 0 sur CC1 (Modulation)
table[0].curve = AINSER_CURVE_EXPO;
```

#### ainser_map_init_defaults

```c
void ainser_map_init_defaults(void);
```

Initialise la table de mapping avec les paramètres par défaut :
- Tous les 64 canaux activés
- Numéros CC 16-79 (16+indice canal)
- Canal MIDI 0 (Canal 1)
- Plage ADC complète (0-4095)
- Courbe linéaire
- Seuil par défaut (8 unités ADC brutes)
- Pas d'inversion

**Exemple :**
```c
ainser_map_init_defaults();
```

#### ainser_map_set_output_cb

```c
void ainser_map_set_output_cb(AINSER_MapOutputFn cb);
```

Définit la fonction callback de sortie qui sera appelée lorsqu'une valeur de canal change.

**Paramètres :**
- `cb` : Fonction callback ou `NULL` pour désactiver la sortie

**Exemple :**
```c
void ma_sortie_cc(uint8_t channel, uint8_t cc, uint8_t value) {
    midi_send_cc(channel, cc, value);
}

ainser_map_set_output_cb(ma_sortie_cc);
```

#### ainser_map_process_channel

```c
void ainser_map_process_channel(uint8_t index, uint16_t raw12);
```

Traite un seul canal AINSER avec une lecture ADC brute 12 bits. Cette fonction :
1. Applique le filtrage de seuil par canal
2. Applique le lissage (filtre passe-bas)
3. Limite à la plage min/max
4. Applique l'inversion si activée
5. Applique la transformation de courbe
6. Quantifie en valeur MIDI 7 bits (0-127)
7. N'émet une sortie que lorsque la valeur change réellement

**Paramètres :**
- `index` : Indice du canal (0-63)
- `raw12` : Valeur ADC brute 12 bits (0-4095)

**Exemple :**
```c
// Dans votre boucle de scan ADC
for (uint8_t ch = 0; ch < 64; ch++) {
    uint16_t raw = adc_read_channel(ch);
    ainser_map_process_channel(ch, raw);
}
```

#### ainser_map_load_sd

```c
int ainser_map_load_sd(const char* path);
```

Charge la configuration de mapping depuis un fichier texte sur carte SD. Seuls les canaux explicitement listés dans le fichier sont modifiés ; les autres conservent leurs paramètres actuels.

**Paramètres :**
- `path` : Chemin vers fichier de configuration (ex : `"0:/cfg/ainser_map.ngc"`)

**Retourne :**
- `0` : Succès
- Valeur négative : Erreur (ex : `-2` pour fichier non trouvé, `-10` si FATFS non disponible)

**Exemple :**
```c
int resultat = ainser_map_load_sd("0:/cfg/ainser_map.ngc");
if (resultat != 0) {
    // Gérer l'erreur - les valeurs par défaut restent en place
}
```

## Format du Fichier de Configuration Carte SD

Les fichiers de configuration utilisent le format `.ngc` (texte style INI).

### Chemin du Fichier

Emplacement par défaut : `0:/cfg/ainser_map.ngc`

### Syntaxe

**Important :** Ne pas confondre `[CHn]` avec `CHAN=` :
- `[CHn]` = En-tête de section pour l'indice de **canal d'entrée** matériel (0-63)
- `CHAN=` = Clé de configuration pour le numéro de **canal MIDI** (0-15)

```ini
# Les commentaires commencent par # ou ;
# Seuls les canaux listés ici remplacent les valeurs par défaut compilées

[CHn]          # n = 0..63 indice de canal d'entrée AINSER logique
CC=numéro      # 0..127
CHAN=numéro    # Canal MIDI 0..15 (0 = ch1)
CURVE=0|1|2    # 0=linéaire, 1=expo, 2=log (ou LIN/EXPO/LOG)
INVERT=0|1
MIN=0..4095
MAX=0..4095
THRESHOLD=delta_raw_12bit
ENABLED=0|1
```

### Clés de Configuration

| Clé | Type | Plage | Description |
|-----|------|-------|-------------|
| `CC` | Entier | 0-127 | Numéro CC MIDI à envoyer |
| `CHAN` ou `CHANNEL` | Entier | 0-15 | Canal MIDI (0 = Canal 1) |
| `CURVE` | Entier/Chaîne | 0-2 ou LIN/EXPO/LOG | Type de courbe de réponse |
| `INVERT` | Booléen | 0-1 | Inverse la valeur (1 = inversé) |
| `MIN` | Entier | 0-4095 | Seuil ADC minimum |
| `MAX` | Entier | 0-4095 | Seuil ADC maximum |
| `THRESHOLD` ou `THR` | Entier | 0-4095 | Changement minimum requis pour émettre une mise à jour |
| `ENABLED` ou `ENABLE` | Booléen | 0-1 | Active ce canal (1 = activé) |

### Exemple de Configuration

```ini
# Configuration de mapping AINSER pour capteurs de soufflet

[CH16]
# Capteur de soufflet primaire
CC=36
CHAN=0
CURVE=LOG       # ou CURVE=2
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

[CH17]
# Capteur de soufflet secondaire
CC=37
CHAN=0
CURVE=LOG
INVERT=0
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

[CH20]
# Pédale d'expression avec réponse linéaire
CC=11          # Expression MIDI
CHAN=0
CURVE=LINEAR   # ou CURVE=0
MIN=50
MAX=4000
THRESHOLD=8
ENABLED=1
```

## Exemples d'Utilisation

### Configuration de Base

```c
#include "Services/ainser/ainser_map.h"

void ainser_init(void) {
    // Initialise avec les valeurs par défaut
    ainser_map_init_defaults();
    
    // Définit le callback de sortie
    ainser_map_set_output_cb(ma_sortie_midi);
    
    // Charge les mappings personnalisés depuis la carte SD
    ainser_map_load_sd("0:/cfg/ainser_map.ngc");
}

void ma_sortie_midi(uint8_t channel, uint8_t cc, uint8_t value) {
    // Route vers sortie MIDI
    midi_router_send_cc(MIDI_ROUTER_SRC_AINSER, channel, cc, value);
}
```

### Configuration à l'Exécution

```c
// Modifie les paramètres du canal 0 à l'exécution
AINSER_MapEntry *table = ainser_map_get_table();

// Configure comme molette de modulation
table[0].cc = 1;              // CC1 = Modulation
table[0].channel = 0;         // Canal MIDI 1
table[0].curve = AINSER_CURVE_LINEAR;
table[0].min = 100;           // Ignore les 100 unités ADC inférieures
table[0].max = 4000;          // Ignore les 95 unités ADC supérieures
table[0].threshold = 10;      // Réduit l'oscillation
table[0].enabled = 1;
```

### Intégration avec Boucle de Scan ADC

```c
void ainser_scan_task(void *params) {
    while (1) {
        // Scanne tous les 64 canaux AINSER
        for (uint8_t ch = 0; ch < 64; ch++) {
            uint16_t raw = hardware_adc_read(ch);
            ainser_map_process_channel(ch, raw);
        }
        
        // Délai entre scans
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### Comparaison de Courbes

```c
// Teste différentes courbes sur le même canal
AINSER_MapEntry *table = ainser_map_get_table();

// Linéaire : réponse directement proportionnelle
table[0].curve = AINSER_CURVE_LINEAR;

// Exponentielle : plus de sensibilité aux valeurs basses
table[1].curve = AINSER_CURVE_EXPO;

// Logarithmique : plus de sensibilité aux valeurs hautes
table[2].curve = AINSER_CURVE_LOG;
```

## Détails d'Implémentation

### Algorithme de Lissage

Le module utilise un filtre passe-bas à pôle unique simple pour le lissage :

```
lissé = (lissé * ALPHA + brut) / (ALPHA + 1)
```

ALPHA par défaut = 6, offrant un bon équilibre entre réactivité et stabilité.

### Comportement du Seuil

Le seuil empêche l'émission de messages CC MIDI pour de petites fluctuations ADC :
- Seules les valeurs changeant de plus de `threshold` unités ADC déclenchent le traitement
- Aide à éliminer l'oscillation des entrées analogiques bruitées
- Seuil par défaut est 8 unités ADC (~0,2% de l'échelle complète)

### Quantification des Valeurs

Le module convertit les valeurs ADC 12 bits (0-4095) en valeurs MIDI 7 bits (0-127) :
1. Limite la valeur brute à la plage min/max configurée
2. Applique l'inversion si activée
3. Met à l'échelle vers la plage 0-127 avec arrondi
4. Applique la transformation de courbe
5. N'émet que si différent de la dernière valeur envoyée

### Implémentations de Courbe

**Linéaire :** Mapping direct
```
sortie = entrée
```

**Exponentielle :** Opération au carré pour plus de résolution en bas
```
sortie = (entrée * entrée) / 127
```

**Logarithmique :** Opération racine carrée pour plus de résolution en haut
```
sortie = sqrt(entrée * 127)
```

## Conseils et Meilleures Pratiques

1. **Paramètres de Seuil** : Augmentez le seuil (12-20) pour capteurs bruités, diminuez (4-8) pour signaux propres
2. **Calibrage Min/Max** : Mesurez la plage réelle du capteur et définissez min/max en conséquence pour utiliser toute la plage MIDI
3. **Sélection de Courbe** : 
   - Utilisez LINEAR pour la plupart des applications
   - Utilisez EXPO pour contrôleurs de souffle et pédales d'expression (plus de contrôle aux niveaux faibles)
   - Utilisez LOG pour contrôles de volume/fader (plus de précision aux niveaux élevés)
4. **Lissage** : Le lissage intégré (ALPHA=6) fonctionne bien pour la plupart des capteurs ; modifiez `AINSER_MAP_DEFAULT_SMOOTHING` si nécessaire
5. **Nombre de Canaux** : N'activez que les canaux avec capteurs connectés pour réduire la charge CPU

## Modules Associés

- **Mapping DIN** : Mapping d'entrée numérique (boutons, interrupteurs)
- **Mapping DOUT** : Mapping de sortie numérique (LEDs, relais)
- **Routeur MIDI** : Route les messages MIDI entre sources et destinations
- **Service Expression** : Gestion de pédale d'expression de niveau supérieur

## Voir Aussi

- [Documentation Mapping DIN](../din/README_FR.md)
- [Documentation Mapping DOUT](../dout/README_FR.md)
- [Structure de Fichiers Carte SD](../../Assets/sd_cfg/README_SD_TREE.txt)

---

**Chemin du Module :** `Services/ainser/`  
**Fichier d'En-tête :** `ainser_map.h`  
**Implémentation :** `ainser_map.c`  
**Dépendances :** FATFS (optionnel, pour support carte SD)

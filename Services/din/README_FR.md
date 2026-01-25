# Module de Mapping DIN

## Vue d'ensemble

Le module de Mapping DIN fournit une couche de mapping flexible pour les entrées numériques (boutons, interrupteurs, pédales), découplant les états des broches matérielles de la génération d'événements MIDI. Il prend en charge la configuration par canal avec mapping NOTE ou CC, paramètres de vélocité, inversion et contrôle d'activation/désactivation. La configuration peut être chargée depuis des fichiers texte sur carte SD (format `.ngc`).

## Fonctionnalités

- **64 Canaux d'Entrée Numérique** : Support des indices logiques DIN de 0 à 63
- **Mapping Par Canal** : Configuration individuelle pour chaque canal
- **Deux Types de Sortie** : Envoie des Notes MIDI ou des Control Changes
- **Vélocités Configurables** : Vélocités indépendantes pour Note On et Note Off
- **Routage de Canal MIDI** : Envoie vers n'importe quel canal MIDI (0-15)
- **Inversion de Polarité** : Support pour entrées actives-haut ou actives-bas
- **Contrôle Activation/Désactivation** : Active/désactive les canaux individuellement
- **Configuration Carte SD** : Charge les mappings depuis fichiers de configuration texte
- **Callback de Sortie** : Intégration avec routeurs MIDI ou sortie directe

## Référence API

### Types de Données

#### DIN_MapType

Type d'événement pour le mapping d'entrée numérique :

| Valeur | Nom | Description |
|--------|-----|-------------|
| `0` | `DIN_MAP_TYPE_NONE` | Canal désactivé |
| `1` | `DIN_MAP_TYPE_NOTE` | Envoie des messages MIDI Note On/Off |
| `2` | `DIN_MAP_TYPE_CC` | Envoie des messages MIDI Control Change |

#### DIN_MapEntry

Structure de configuration par canal :

```c
typedef struct {
    uint8_t enabled;    // 0 = ignorer, 1 = actif
    uint8_t invert;     // 0 = actif-bas, 1 = actif-haut
    uint8_t type;       // DIN_MAP_TYPE_*
    uint8_t channel;    // Canal MIDI 0..15 (0 = MIDI ch1)
    uint8_t number;     // Numéro de note ou CC (0-127)
    uint8_t vel_on;     // Vélocité pour Note On (0-127)
    uint8_t vel_off;    // Vélocité pour Note Off (0-127, 0 = note-off avec vel 0)
    uint8_t reserved;   // Remplissage pour usage futur
} DIN_MapEntry;
```

#### DIN_MapOutputFn

Type de fonction callback de sortie :

```c
typedef void (*DIN_MapOutputFn)(DIN_MapType type,
                                uint8_t channel,
                                uint8_t number,
                                uint8_t value);
```

**Paramètres :**
- `type` : Type d'événement (`DIN_MAP_TYPE_NOTE` ou `DIN_MAP_TYPE_CC`)
- `channel` : Canal MIDI (0-15)
- `number` : Numéro de note ou CC (0-127)
- `value` : Vélocité (pour NOTE) ou valeur CC (pour CC : 127=enfoncé, 0=relâché)

### Fonctions

| Fonction | Description |
|----------|-------------|
| `din_map_init_defaults()` | Initialise la table de mapping avec mapping de note par défaut |
| `din_map_get_table()` | Retourne un pointeur vers la table de mapping interne pour modifications runtime |
| `din_map_set_output_cb()` | Définit la fonction callback pour sortie MIDI |
| `din_map_process_event()` | Traite un changement d'état d'entrée numérique |
| `din_map_load_sd()` | Charge la configuration de mapping depuis fichier carte SD |

#### din_map_init_defaults

```c
void din_map_init_defaults(uint8_t base_note);
```

Initialise la table de mapping avec les paramètres par défaut :
- Tous les 64 canaux activés
- Type : NOTE
- Canal MIDI 0 (Canal 1)
- Numéros de note : `base_note` + indice canal (0-63)
- Vélocité on : 100
- Vélocité off : 0
- Actif-bas (invert=0)

**Paramètres :**
- `base_note` : Numéro de note MIDI de départ (typiquement 36 pour C1 ou 48 pour C2)

**Exemple :**
```c
// Mappe vers notes commençant à C2 (48)
din_map_init_defaults(48);
```

#### din_map_get_table

```c
DIN_MapEntry *din_map_get_table(void);
```

Retourne un pointeur vers la table de mapping interne permettant la modification directe des configurations de canal à l'exécution.

**Retourne :** Pointeur vers tableau de 64 structures `DIN_MapEntry`

**Exemple :**
```c
DIN_MapEntry *table = din_map_get_table();
table[0].type = DIN_MAP_TYPE_CC;    // Change en mode CC
table[0].number = 64;                 // CC64 = Pédale de sustain
```

#### din_map_set_output_cb

```c
void din_map_set_output_cb(DIN_MapOutputFn cb);
```

Définit la fonction callback de sortie qui sera appelée lorsqu'un bouton/interrupteur change d'état.

**Paramètres :**
- `cb` : Fonction callback ou `NULL` pour désactiver la sortie

**Exemple :**
```c
void ma_sortie_din(DIN_MapType type, uint8_t channel, 
                   uint8_t number, uint8_t value) {
    if (type == DIN_MAP_TYPE_NOTE) {
        if (value > 0) {
            midi_send_note_on(channel, number, value);
        } else {
            midi_send_note_off(channel, number, value);
        }
    } else if (type == DIN_MAP_TYPE_CC) {
        midi_send_cc(channel, number, value);
    }
}

din_map_set_output_cb(ma_sortie_din);
```

#### din_map_process_event

```c
void din_map_process_event(uint8_t index, uint8_t pressed);
```

Traite un changement d'état d'entrée numérique pour un canal spécifique. Cette fonction :
1. Applique l'inversion de polarité si configurée
2. Vérifie si le canal est activé
3. Génère le message MIDI approprié selon le type (NOTE ou CC)
4. Appelle le callback de sortie si défini

**Paramètres :**
- `index` : Indice du canal (0-63)
- `pressed` : État d'entrée brut (1 = enfoncé/actif, 0 = relâché/inactif, avant inversion)

**Exemple :**
```c
// Dans votre boucle de scan de boutons
for (uint8_t i = 0; i < 64; i++) {
    uint8_t state = read_button_state(i);
    if (state != previous_state[i]) {
        din_map_process_event(i, state);
        previous_state[i] = state;
    }
}
```

#### din_map_load_sd

```c
int din_map_load_sd(const char* path);
```

Charge la configuration de mapping depuis un fichier texte sur carte SD. Seuls les canaux explicitement listés dans le fichier sont modifiés ; les autres conservent leurs paramètres actuels.

**Paramètres :**
- `path` : Chemin vers fichier de configuration (ex : `"0:/cfg/din_map.ngc"`)

**Retourne :**
- `0` : Succès
- Valeur négative : Erreur (ex : `-2` pour fichier non trouvé, `-10` si FATFS non disponible)

**Exemple :**
```c
int resultat = din_map_load_sd("0:/cfg/din_map.ngc");
if (resultat != 0) {
    // Gérer l'erreur - les valeurs par défaut restent en place
}
```

## Format du Fichier de Configuration Carte SD

Les fichiers de configuration utilisent le format `.ngc` (texte style INI).

### Chemin du Fichier

Emplacement par défaut : `0:/cfg/din_map.ngc`

### Syntaxe

**Important :** Ne pas confondre `[CHn]` avec `CHAN=` :
- `[CHn]` = En-tête de section pour **l'indice d'événement DIN** (0-63) - identifie quel bouton/switch
- `CHAN=` = Clé de configuration pour le numéro de **canal MIDI** (0-15) - définit le canal MIDI de sortie

```ini
# Les commentaires commencent par # ou ;
# Seuls les canaux listés remplacent les valeurs par défaut compilées

[CHn]           # n = 0..63 indice d'événement DIN (identifiant bouton/switch)
TYPE=NOTE|CC|0|1|2
CHAN=0..15      # 0 = MIDI ch1
NUMBER=0..127   # Numéro de note ou CC
VEL_ON=0..127
VEL_OFF=0..127
INVERT=0|1
ENABLED=0|1
LCD_TEXT="texte" # Optionnel: Affiche le texte sur LCD/OLED lors de l'appui
```

### Clés de Configuration

| Clé | Type | Plage/Valeurs | Description |
|-----|------|---------------|-------------|
| `TYPE` | Chaîne/Entier | NOTE, CC, 0-2 | Type d'événement (0=NONE, 1=NOTE, 2=CC) |
| `CHAN` ou `CHANNEL` | Entier | 0-15 | Canal MIDI (0 = Canal 1) |
| `NUMBER`, `NOTE`, ou `CC` | Entier | 0-127 | Numéro de note MIDI ou numéro CC |
| `VEL_ON` ou `VELON` | Entier | 0-127 | Vélocité pour Note On ou valeur CC quand enfoncé |
| `VEL_OFF` ou `VELOFF` | Entier | 0-127 | Vélocité pour Note Off (0 = note-off avec vel 0) |
| `INVERT` | Booléen | 0-1 | Inverse la polarité (1 = actif-haut) |
| `ENABLED` ou `ENABLE` | Booléen | 0-1 | Active ce canal (1 = activé) |
| `LCD_TEXT` ou `LCD` | Chaîne | Texte (max 64 car.) | Texte optionnel à afficher sur LCD/OLED lors de l'appui |

### Exemple de Configuration

```ini
# Configuration de mapping DIN pour matrice de boutons et pédales

[CH0]
# Premier bouton envoie Note C3 sur canal 1 avec feedback LCD
TYPE=NOTE
CHAN=0
NUMBER=48
VEL_ON=100
VEL_OFF=0
LCD_TEXT="Note C3 Appuyée"
ENABLED=1

[CH1]
# Deuxième bouton agit comme CC64 (sustain) toggle avec texte LCD
TYPE=CC
CHAN=0
NUMBER=64
LCD_TEXT="Sustain ON/OFF"
INVERT=0
ENABLED=1

[CH2]
# Pédale 1 : pédale sustain (actif-haut) avec feedback visuel
TYPE=CC
CHAN=0
NUMBER=64
LCD_TEXT="Pédale Sustain"
INVERT=1
ENABLED=1

[CH3]
# Pédale 2 : pédale douce
TYPE=CC
CHAN=0
NUMBER=67
LCD_TEXT="Pédale Douce"
INVERT=1
ENABLED=1

[CH16]
# Pad de batterie : caisse claire avec couches de vélocité et affichage
TYPE=NOTE
CHAN=9         # Canal MIDI 10 (batterie)
NUMBER=38      # Caisse claire
VEL_ON=110
VEL_OFF=0
LCD_TEXT="Frappe Caisse!"
ENABLED=1
```

## Exemples d'Utilisation

### Configuration de Base

```c
#include "Services/din/din_map.h"

void din_init(void) {
    // Initialise avec C2 comme note de base (48)
    din_map_init_defaults(48);
    
    // Définit le callback de sortie
    din_map_set_output_cb(ma_sortie_midi);
    
    // Charge les mappings personnalisés depuis la carte SD
    din_map_load_sd("0:/cfg/din_map.ngc");
}

void ma_sortie_midi(DIN_MapType type, uint8_t channel, 
                    uint8_t number, uint8_t value) {
    // Route vers sortie MIDI
    midi_router_send(MIDI_ROUTER_SRC_DIN, type, channel, number, value);
}
```

### Configuration à l'Exécution

```c
// Configure le canal 0 comme pédale sustain à l'exécution
DIN_MapEntry *table = din_map_get_table();

table[0].type = DIN_MAP_TYPE_CC;
table[0].channel = 0;       // Canal MIDI 1
table[0].number = 64;       // CC64 = Sustain
table[0].invert = 1;        // Pédale actif-haut
table[0].enabled = 1;
```

## Détails d'Implémentation

### Gestion de Polarité

Le flag `invert` contrôle comment l'état physique du bouton mappe vers l'état logique :

**Actif-Bas (invert=0, par défaut) :**
- HAUT physique (1) → RELÂCHÉ logique (0)
- BAS physique (0) → ENFONCÉ logique (1)

**Actif-Haut (invert=1) :**
- HAUT physique (1) → ENFONCÉ logique (1)
- BAS physique (0) → RELÂCHÉ logique (0)

### Comportement Type NOTE

Quand `type = DIN_MAP_TYPE_NOTE` :
- **Enfoncé :** Envoie Note On avec vélocité `vel_on`
- **Relâché :** Envoie Note Off avec vélocité `vel_off`
- Si `vel_off = 0`, envoie Note Off avec vélocité 0 (standard)

### Comportement Type CC

Quand `type = DIN_MAP_TYPE_CC` :
- **Enfoncé :** Envoie CC avec valeur 127
- **Relâché :** Envoie CC avec valeur 0

Ceci crée un comportement de toggle adapté aux pédales sustain, interrupteurs et contrôles similaires.

## Conseils et Meilleures Pratiques

1. **Anti-rebond** : Implémentez l'anti-rebond dans votre boucle de scan (délai 5-10ms) pour éviter les déclenchements multiples
2. **Suivi d'État** : N'appelez `din_map_process_event()` que lorsque l'état change réellement pour réduire la charge CPU
3. **Actif-Bas vs Actif-Haut** : 
   - Utilisez `invert=0` pour boutons avec résistances pull-up (le plus courant)
   - Utilisez `invert=1` pour pédales et interrupteurs momentanés avec résistances pull-down
4. **Couches de Vélocité** : Utilisez plusieurs canaux DIN mappés à la même note avec différentes vélocités pour pads sensibles à la vélocité
5. **Attribution de Canal** : Utilisez canal MIDI 10 (définir `channel=9`) pour mappings batterie/percussion
6. **CC vs NOTE** : Utilisez CC pour pédales, interrupteurs et toggles ; utilisez NOTE pour touches et pads

## Modules Associés

- **Mapping AINSER** : Mapping d'entrée analogique (potentiomètres, capteurs)
- **Mapping DOUT** : Mapping de sortie numérique (LEDs, relais)
- **Routeur MIDI** : Route les messages MIDI entre sources et destinations

## Voir Aussi

- [Documentation Mapping AINSER](../ainser/README_FR.md)
- [Documentation Mapping DOUT](../dout/README_FR.md)
- [Structure de Fichiers Carte SD](../../Assets/sd_cfg/README_SD_TREE.txt)

---

**Chemin du Module :** `Services/din/`  
**Fichier d'En-tête :** `din_map.h`  
**Implémentation :** `din_map.c`  
**Dépendances :** FATFS (optionnel, pour support carte SD)

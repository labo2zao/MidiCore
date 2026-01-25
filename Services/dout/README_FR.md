# Module de Mapping DOUT

## Vue d'ensemble

Le module de Mapping DOUT fournit une couche d'abstraction matérielle flexible pour les sorties numériques (LEDs, relais, indicateurs), permettant le mapping de bits logiques vers bits physiques avec contrôle d'inversion par bit et global. Il prend également en charge le mapping de LEDs RGB avec attributions de canaux configurables et inversion de polarité.

## Fonctionnalités

- **Mapping de Sortie Numérique 64 bits** : Support jusqu'à 64 bits de sortie logique
- **Mapping de Bits Flexible** : Remappe les bits logiques vers broches matérielles physiques
- **Inversion Par Bit** : Contrôle d'inversion individuel pour chaque bit de sortie
- **Inversion Globale** : Applique une inversion par défaut à toutes les sorties
- **Support LED RGB** : Mappe jusqu'à 16 LEDs RGB avec attributions de broches configurables
- **Inversion de Canal RGB** : Inversion indépendante pour canaux R, G, B
- **Intégration de Configuration** : Fonctionne avec le système de configuration global

## Référence API

### Fonctions

| Fonction | Description |
|----------|-------------|
| `dout_map_init()` | Initialise le mapping DOUT avec configuration |
| `dout_map_apply()` | Applique mapping et inversion pour convertir bits logiques en physiques |
| `dout_set_rgb()` | Définit l'état d'une LED RGB avec valeurs de couleur |

#### dout_map_init

```c
void dout_map_init(const config_t* cfg);
```

Initialise le module de mapping DOUT avec les paramètres de configuration. Si aucune configuration n'est fournie, les valeurs par défaut sont utilisées.

**Paramètres :**
- `cfg` : Pointeur vers structure de configuration ou `NULL` pour valeurs par défaut

**Exemple :**
```c
#include "Services/dout/dout_map.h"
#include "Services/config/config.h"

config_t ma_config;
config_set_defaults(&ma_config);

// Personnalise la configuration
ma_config.dout_invert_default = 0;  // Pas d'inversion globale
ma_config.bit_inv[0] = 1;           // Inverse seulement le bit 0

dout_map_init(&ma_config);
```

#### dout_map_apply

```c
void dout_map_apply(const uint8_t* logical, uint8_t* physical, uint16_t bytes);
```

Convertit les états de sortie logiques en états matériels physiques en appliquant :
1. Copie du buffer logique vers buffer physique
2. Application de l'inversion globale par défaut si configurée
3. Application de l'inversion par bit pour chaque bit configuré

**Paramètres :**
- `logical` : Buffer source avec états de bits logiques
- `physical` : Buffer destination pour états de bits physiques (sortie matérielle)
- `bytes` : Nombre d'octets à traiter (typiquement 8 pour 64 bits)

**Exemple :**
```c
uint8_t logical[8] = {0};   // État logique (niveau application)
uint8_t physical[8];         // État physique (niveau matériel)

// Définit quelques bits logiques
logical[0] |= (1 << 0);  // Définit bit 0
logical[0] |= (1 << 5);  // Définit bit 5

// Applique mapping et inversion
dout_map_apply(logical, physical, 8);

// Écrit buffer physique vers matériel
hardware_write_shift_registers(physical, 8);
```

#### dout_set_rgb

```c
void dout_set_rgb(uint8_t* logical, uint8_t led, uint8_t r, uint8_t g, uint8_t b);
```

Définit l'état d'une LED RGB en définissant les bits appropriés dans le buffer de sortie logique. Applique les inversions de canaux RGB configurées et les mappings de broches.

**Paramètres :**
- `logical` : Buffer de sortie logique à modifier (8 octets pour 64 bits)
- `led` : Indice de la LED (0-15)
- `r` : État rouge (0=éteint, 1=allumé)
- `g` : État vert (0=éteint, 1=allumé)
- `b` : État bleu (0=éteint, 1=allumé)

**Exemple :**
```c
uint8_t logical[8] = {0};

// Définit LED 0 en rouge
dout_set_rgb(logical, 0, 1, 0, 0);

// Définit LED 1 en jaune (rouge + vert)
dout_set_rgb(logical, 1, 1, 1, 0);

// Définit LED 2 en blanc
dout_set_rgb(logical, 2, 1, 1, 1);

// Applique mapping et écrit vers matériel
uint8_t physical[8];
dout_map_apply(logical, physical, 8);
hardware_write_shift_registers(physical, 8);
```

## Structure de Configuration

Le module de mapping DOUT utilise la structure `config_t` de `Services/config/config.h` :

```c
typedef struct {
    // Inversion globale
    uint8_t dout_invert_default;  // 0=normal, 1=inverse toutes les sorties par défaut
    
    // Inversion par bit (64 bits)
    uint8_t bit_inv[64];          // 0=normal, 1=inverse ce bit spécifique
    
    // Mapping de LED RGB (16 LEDs, 3 canaux chacune)
    uint8_t rgb_map_r[16];        // Indice de bit physique pour canal Rouge (0xFF=non utilisé)
    uint8_t rgb_map_g[16];        // Indice de bit physique pour canal Vert (0xFF=non utilisé)
    uint8_t rgb_map_b[16];        // Indice de bit physique pour canal Bleu (0xFF=non utilisé)
    
    // Inversion de canal RGB
    uint8_t rgb_r_invert;         // 0=normal, 1=inverse canal rouge pour toutes les LEDs
    uint8_t rgb_g_invert;         // 0=normal, 1=inverse canal vert pour toutes les LEDs
    uint8_t rgb_b_invert;         // 0=normal, 1=inverse canal bleu pour toutes les LEDs
    
    // ... autres champs de config
} config_t;
```

## Exemples d'Utilisation

### Sortie Numérique de Base

```c
#include "Services/dout/dout_map.h"

// Initialise avec valeurs par défaut
dout_map_init(NULL);

// Crée buffer d'état logique
uint8_t logical[8] = {0};
uint8_t physical[8];

// Définit quelques sorties
logical[0] = 0xFF;  // Tous les bits de l'octet 0 allumés
logical[1] = 0x0F;  // 4 bits inférieurs de l'octet 1 allumés

// Applique mapping
dout_map_apply(logical, physical, 8);

// Écrit vers matériel (registres à décalage, GPIO, etc.)
srio_write_outputs(physical, 8);
```

### Utilisation de l'Inversion Par Bit

```c
config_t cfg;
config_set_defaults(&cfg);

// Inverse des bits spécifiques (pour LEDs actives-bas ou logique inversée)
cfg.bit_inv[0] = 1;   // Inverse bit 0
cfg.bit_inv[1] = 1;   // Inverse bit 1
cfg.bit_inv[15] = 1;  // Inverse bit 15

dout_map_init(&cfg);

uint8_t logical[8] = {0};
uint8_t physical[8];

// Définir bit 0 en logique résultera en son effacement en physique
logical[0] = 0x01;  // Bit 0 = 1 (logiquement allumé)

dout_map_apply(logical, physical, 8);
// physical[0] aura bit 0 = 0 (physiquement inversé)
```

### Contrôle de LED RGB

```c
config_t cfg;
config_set_defaults(&cfg);

// Mappe LED RGB 0 vers broches physiques
cfg.rgb_map_r[0] = 0;   // Canal rouge sur bit 0
cfg.rgb_map_g[0] = 1;   // Canal vert sur bit 1
cfg.rgb_map_b[0] = 2;   // Canal bleu sur bit 2

// Mappe LED RGB 1 vers broches différentes
cfg.rgb_map_r[1] = 3;   // Canal rouge sur bit 3
cfg.rgb_map_g[1] = 4;   // Canal vert sur bit 4
cfg.rgb_map_b[1] = 5;   // Canal bleu sur bit 5

// LEDs RGB à cathode commune (actif-haut)
cfg.rgb_r_invert = 0;
cfg.rgb_g_invert = 0;
cfg.rgb_b_invert = 0;

dout_map_init(&cfg);

uint8_t logical[8] = {0};
uint8_t physical[8];

// Définit LED 0 en magenta (rouge + bleu)
dout_set_rgb(logical, 0, 1, 0, 1);

// Définit LED 1 en cyan (vert + bleu)
dout_set_rgb(logical, 1, 0, 1, 1);

// Applique et envoie sortie
dout_map_apply(logical, physical, 8);
hardware_write(physical, 8);
```

### LEDs RGB à Anode Commune

```c
// Pour LEDs RGB à anode commune (actif-bas), inverse tous les canaux RGB
config_t cfg;
config_set_defaults(&cfg);

cfg.rgb_r_invert = 1;  // Inverse canal rouge
cfg.rgb_g_invert = 1;  // Inverse canal vert
cfg.rgb_b_invert = 1;  // Inverse canal bleu

// Mappe broches de LED RGB
cfg.rgb_map_r[0] = 0;
cfg.rgb_map_g[0] = 1;
cfg.rgb_map_b[0] = 2;

dout_map_init(&cfg);

uint8_t logical[8] = {0};
uint8_t physical[8];

// Définit LED en rouge (sera inversé en actif-bas)
dout_set_rgb(logical, 0, 1, 0, 0);

dout_map_apply(logical, physical, 8);
// Bit 0 sera 0 (actif-bas pour rouge allumé)
// Bits 1,2 seront 1 (actif-bas pour vert/bleu éteints)
```

### Indicateurs de Statut LED

```c
void update_status_leds(uint8_t midi_active, uint8_t sd_ok, uint8_t error) {
    uint8_t logical[8] = {0};
    uint8_t physical[8];
    
    // Attributions de bit
    const uint8_t LED_MIDI = 0;
    const uint8_t LED_SD = 1;
    const uint8_t LED_ERROR = 2;
    
    // Définit états logiques
    if (midi_active) logical[0] |= (1 << LED_MIDI);
    if (sd_ok)       logical[0] |= (1 << LED_SD);
    if (error)       logical[0] |= (1 << LED_ERROR);
    
    // Applique mapping et écrit vers matériel
    dout_map_apply(logical, physical, 8);
    hardware_update_leds(physical);
}
```

## Détails d'Implémentation

### Indexation de Bits

Les bits sont indexés de 0 à 63 :
- Bit 0 = octet 0, bit 0 (LSB)
- Bit 7 = octet 0, bit 7 (MSB)
- Bit 8 = octet 1, bit 0
- ...
- Bit 63 = octet 7, bit 7

### Ordre d'Inversion

La fonction `dout_map_apply()` applique les inversions dans cet ordre :
1. Copie buffer logique vers buffer physique
2. Applique inversion globale par défaut (`dout_invert_default`)
3. Applique inversions par bit (`bit_inv[]`)

Ceci permet aux paramètres par bit de remplacer la valeur par défaut globale.

### Mapping de LED RGB

Les LEDs RGB utilisent des indices de bits séparés pour chaque canal de couleur :
- Chaque LED (0-15) a trois indices de bits configurables
- Définir l'indice de bit à 0xFF pour désactiver un canal
- L'inversion est appliquée après mapping mais avant écriture dans buffer logique

### Capacité Maximum

- **Total de bits** : 64 (8 octets)
- **LEDs RGB** : Jusqu'à 16 (nécessite 48 bits si tous les canaux RGB sont utilisés)
- **Bits restants** : Peuvent être utilisés pour sorties non-RGB

## Conseils et Meilleures Pratiques

1. **Inversion Cohérente** : Utilisez `dout_invert_default` global pour polarité matérielle cohérente
2. **Remplacements Par Bit** : Utilisez `bit_inv[]` pour exceptions individuelles (actif-haut/bas mixte)
3. **Ordre de Canal RGB** : Vérifiez que le câblage de votre LED RGB correspond à votre configuration (R-G-B vs B-G-R)
4. **Anode/Cathode Commune** : Définissez les inversions de canal RGB selon le type de LED
5. **Broches Non Utilisées** : Définissez les canaux RGB non utilisés à 0xFF pour éviter les conflits
6. **Taille de Buffer** : Utilisez toujours des buffers de 8 octets pour support 64 bits complet
7. **Fréquence de Mise à Jour** : N'appelez `dout_map_apply()` que lorsque l'état logique change pour réduire la charge CPU

## Configurations Matérielles Courantes

### LEDs Standard Actives-Haut

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.dout_invert_default = 0;  // Actif-haut
dout_map_init(&cfg);
```

### LEDs Standard Actives-Bas

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.dout_invert_default = 1;  // Actif-bas
dout_map_init(&cfg);
```

### Mélange Actif-Haut et Actif-Bas

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.dout_invert_default = 0;  // Par défaut actif-haut

// Inverse des bits spécifiques qui sont actifs-bas
cfg.bit_inv[5] = 1;
cfg.bit_inv[6] = 1;

dout_map_init(&cfg);
```

### LEDs RGB à Cathode Commune (Actif-Haut)

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.rgb_r_invert = 0;
cfg.rgb_g_invert = 0;
cfg.rgb_b_invert = 0;
dout_map_init(&cfg);
```

### LEDs RGB à Anode Commune (Actif-Bas)

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.rgb_r_invert = 1;
cfg.rgb_g_invert = 1;
cfg.rgb_b_invert = 1;
dout_map_init(&cfg);
```

## Modules Associés

- **Mapping AINSER** : Mapping d'entrée analogique (potentiomètres, capteurs)
- **Mapping DIN** : Mapping d'entrée numérique (boutons, interrupteurs)
- **Service SRIO** : Pilote matériel d'E/S registre à décalage
- **Service Config** : Gestion centralisée de configuration

## Voir Aussi

- [Documentation Mapping AINSER](../ainser/README_FR.md)
- [Documentation Mapping DIN](../din/README_FR.md)
- [Documentation Service Config](../config/README.md)
- [Documentation Service SRIO](../srio/README.md)

---

**Chemin du Module :** `Services/dout/`  
**Fichier d'En-tête :** `dout_map.h`  
**Implémentation :** `dout_map.c`  
**Dépendances :** `Services/config/config.h`

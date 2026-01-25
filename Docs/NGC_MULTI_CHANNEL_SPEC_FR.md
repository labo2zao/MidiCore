# Spécification de Sélection Multi-Canal NGC

## Vue d'ensemble

Le format NGC supporte la syntaxe de sélection multi-canal, permettant à un bloc de configuration unique de s'appliquer à plusieurs canaux simultanément. Cela réduit la taille du fichier de configuration et rend la configuration en masse plus maintenable.

## Syntaxe

### Canal Unique (Base)
```ini
[CH5]
CC=36
```

### Canaux Multiples (Séparés par Virgule)
```ini
[CH0,1,2,3]
CC=16
ENABLED=1
```
Applique la même configuration aux canaux 0, 1, 2 et 3.

### Sélection de Plage (Tiret)
```ini
[CH0-7]
CC=16
ENABLED=1
```
Applique la même configuration aux canaux 0 à 7 (inclus).

### Notation Mixte
```ini
[CH0,2,4-7,10]
CC=16
ENABLED=1
```
S'applique aux canaux : 0, 2, 4, 5, 6, 7 et 10.

## Algorithme d'Analyse

### Analyse de Liste de Canaux

```c
// Analyse liste de canaux : "0,1,2" ou "0-5" ou "0,2-4,7"
typedef struct {
    uint8_t channels[64];  // Tableau d'indices de canaux
    uint8_t count;         // Nombre de canaux
} ChannelList;

int parse_channel_list(const char* str, ChannelList* list) {
    list->count = 0;
    char buffer[128];
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = 0;
    
    // Découpe par virgule
    char* token = strtok(buffer, ",");
    while (token && list->count < 64) {
        // Supprime les espaces
        while (*token == ' ') token++;
        
        // Vérifie la plage (tiret)
        char* dash = strchr(token, '-');
        if (dash) {
            // Plage : "0-5"
            *dash = 0;
            int start = atoi(token);
            int end = atoi(dash + 1);
            
            if (start >= 0 && end < 64 && start <= end) {
                for (int i = start; i <= end && list->count < 64; i++) {
                    list->channels[list->count++] = (uint8_t)i;
                }
            }
        } else {
            // Canal unique
            int ch = atoi(token);
            if (ch >= 0 && ch < 64) {
                list->channels[list->count++] = (uint8_t)ch;
            }
        }
        
        token = strtok(NULL, ",");
    }
    
    return list->count > 0 ? 0 : -1;
}
```

### Amélioration de l'Analyseur d'En-tête de Section

```c
// Analyseur de section amélioré pour syntaxe [CH0,1,2] ou [CH0-5]
if (line[0] == '[') {
    char* end = strchr(line, ']');
    if (!end) continue;
    *end = 0;
    char* tag = line + 1;
    trim(tag);
    
    current_channels.count = 0;
    
    if ((tag[0] == 'C' || tag[0] == 'c') && 
        (tag[1] == 'H' || tag[1] == 'h')) {
        // Analyse liste de canaux après "CH"
        if (parse_channel_list(tag + 2, &current_channels) == 0) {
            // Liste de canaux analysée avec succès
            // Les paires clé-valeur suivantes s'appliquent à tous les canaux de la liste
        }
    }
    continue;
}

// Applique la configuration à tous les canaux dans current_channels
if (current_channels.count > 0) {
    for (uint8_t i = 0; i < current_channels.count; i++) {
        uint8_t ch = current_channels.channels[i];
        // Applique clé-valeur au canal 'ch'
        apply_config(ch, key, value);
    }
}
```

## Exemples

### Mapping AINSER avec Multi-Canal

```ini
# Configure tous les capteurs de soufflet (canaux 16-19) de manière identique
[CH16-19]
CC=36
CURVE=LOG
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

# Configure des pédales d'expression spécifiques
[CH0,2,4]
CC=11
CURVE=LINEAR
MIN=50
MAX=4000
ENABLED=1

# Configure un mélange de plages et canaux individuels
[CH8-11,14,15]
CC=20
CURVE=EXPO
ENABLED=1
```

### Mapping DIN avec Multi-Canal

```ini
# Configure 12 touches de piano (C à B)
[CH0-11]
TYPE=NOTE
CHAN=0
NUMBER=48     # Note de départ C3, auto-incrémenté par canal
VEL_ON=100
ENABLED=1

# Configure un ensemble de pédales
[CH48,49,50,51]
TYPE=CC
CHAN=0
INVERT=1
ENABLED=1
# Les numéros CC individuels seraient définis par canal
```

### Fonctionnalité d'Auto-Incrément

Certaines implémentations supportent l'auto-incrément des valeurs :

```ini
# Auto-incrément des numéros CC
[CH0-7]
CC=16          # CH0=16, CH1=17, CH2=18, ..., CH7=23
ENABLED=1

# Auto-incrément des numéros de note
[CH0-11]
TYPE=NOTE
NUMBER=48      # CH0=48, CH1=49, CH2=50, ..., CH11=59
VEL_ON=100
```

## Variations d'Implémentation

### Multi-Canal Basique (Config Identique)

Tous les canaux sélectionnés reçoivent une configuration **identique** :

```ini
[CH0-3]
CC=16          # Tous les canaux utilisent CC16
```

### Multi-Canal Avancé (Auto-Incrément)

Les canaux sélectionnés reçoivent des valeurs **incrémentées** :

```ini
[CH0-3]
CC=16          # CH0=16, CH1=17, CH2=18, CH3=19
```

### Remplacement Par Canal

Les configs de canaux individuels remplacent les configs multi-canal :

```ini
# Configuration en masse
[CH0-7]
CC=16
ENABLED=1

# Remplace un canal spécifique
[CH3]
CC=20          # CH3 utilise maintenant CC20 au lieu de 19
ENABLED=0      # CH3 est désactivé
```

## Validation de Format

### Formats Valides

```ini
[CH0]           # Canal unique
[CH0,1]         # Deux canaux
[CH0-5]         # Plage (6 canaux)
[CH0,2-4,7]     # Mixte (canaux 0,2,3,4,7)
[CH0-3,8-11]    # Plages multiples
```

### Formats Invalides (Doivent être Ignorés)

```ini
[CH]            # Pas de numéro de canal
[CH-5]          # Début de plage manquant
[CH0-]          # Fin de plage manquante
[CH5-2]         # Plage invalide (début > fin)
[CH0,,2]        # Double virgule
[CH 0-5]        # Espace dans la spécification de canal
```

## Meilleures Pratiques

### 1. Utiliser des Plages pour Canaux Séquentiels

```ini
# Bon : Utilise une plage
[CH0-15]
ENABLED=1

# Moins efficace : Canaux individuels
[CH0]
ENABLED=1
[CH1]
ENABLED=1
# ... répéter 14 fois de plus
```

### 2. Grouper les Canaux Liés

```ini
# Capteurs de soufflet
[CH16-19]
CC=36
CURVE=LOG

# Pédales d'expression
[CH0,2,4,6]
CC=11
CURVE=LINEAR
```

### 3. Documenter les Sections Multi-Canal

```ini
# Octave 1 du clavier piano (C1-B1)
[CH0-11]
TYPE=NOTE
NUMBER=36
VEL_ON=100

# Octave 2 du clavier piano (C2-B2)
[CH12-23]
TYPE=NOTE
NUMBER=48
VEL_ON=100
```

### 4. Utiliser le Motif de Remplacement pour Exceptions

```ini
# Configure tous les canaux
[CH0-63]
ENABLED=1
THRESHOLD=8

# Désactive des canaux spécifiques
[CH10,20,30]
ENABLED=0

# Seuil personnalisé pour capteurs bruyants
[CH5-7]
THRESHOLD=20
```

## Notes de Compatibilité

### Support du Format Hérité

Les analyseurs multi-canal doivent maintenir la compatibilité avec le format à canal unique :

```ini
# Format hérité - toujours valide
[CH0]
CC=16

[CH1]
CC=17
```

### Détection de l'Analyseur

Les applications peuvent détecter le support multi-canal en vérifiant :
- Virgule (`,`) dans le nom de section
- Tiret (`-`) dans le nom de section (pas au début/fin)

```c
bool is_multi_channel(const char* section) {
    // Vérifie virgule ou tiret interne
    const char* ch_start = section + 2; // Après "CH"
    return (strchr(ch_start, ',') != NULL) || 
           (strchr(ch_start, '-') != NULL && 
            ch_start[0] != '-' && 
            ch_start[strlen(ch_start)-1] != '-');
}
```

## Gestion des Erreurs

### Définitions de Canal Dupliquées

Les définitions ultérieures remplacent les précédentes :

```ini
[CH0-5]
CC=16

[CH3]      # Remplace CH3 du bloc précédent
CC=20
```

### Canaux Hors Plage

Les canaux en dehors de la plage valide (0-63 pour la plupart des modules) sont ignorés silencieusement :

```ini
[CH0-100]   # Seuls CH0-63 sont configurés
CC=16
```

### Listes de Canaux Vides

Les listes de canaux vides ou invalides sont traitées comme des erreurs et la section est ignorée :

```ini
[CH]        # Invalide - pas de canaux spécifiés
CC=16       # Cette configuration est ignorée
```

## Liste de Vérification d'Implémentation

- [ ] Analyser les listes de canaux séparés par virgule
- [ ] Analyser les plages de canaux basées sur tiret
- [ ] Supporter la notation mixte (virgules et tirets)
- [ ] Valider les indices de canal (plage 0-63)
- [ ] Gérer les espaces dans les spécifications de canal
- [ ] Appliquer une config identique à tous les canaux sélectionnés
- [ ] (Optionnel) Supporter l'auto-incrément pour certaines clés
- [ ] (Optionnel) Supporter le remplacement par canal
- [ ] Maintenir la compatibilité avec le format à canal unique
- [ ] Documenter la syntaxe supportée dans le README du module

## Voir Aussi

- [Référence d'Analyse de Texte NGC](NGC_TEXT_PARSING_REFERENCE_FR.md)
- [NGC Text Parsing Reference (English)](NGC_TEXT_PARSING_REFERENCE.md)
- [Référence des Modules de Mapping](MAPPING_MODULES_REFERENCE.md)

---

**Version de Spécification :** 1.0  
**Dernière Mise à Jour :** 2026-01-25  
**Statut :** Spécification de Fonctionnalité

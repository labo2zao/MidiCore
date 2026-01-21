# USB MIDI "Jacks" - Explication Complète

## Pourquoi des "Jacks" et pas simplement des "Ports" ?

**"Jack"** est la **terminologie officielle** de la spécification **USB Device Class Definition for MIDI Devices v1.0**.

Ce n'est PAS un choix arbitraire - c'est défini dans la spec officielle USB MIDI.

---

## Modèle Conceptuel USB MIDI

USB MIDI modélise le flux de données MIDI entre :
- Les **connecteurs physiques** (DIN MIDI 5 broches)
- Le **bus USB** (câbles virtuels)

Pour représenter ce flux bidirectionnel, USB MIDI utilise 4 types de "Jacks" :

### Les 4 Types de Jacks

#### 1. **External IN Jack** (MIDI IN Jack - External)
- Représente le **connecteur DIN MIDI IN physique**
- C'est là que les messages MIDI **entrent** dans l'appareil depuis un clavier, synthé, etc.
- Taille du descripteur : **6 bytes**

#### 2. **Embedded IN Jack** (MIDI IN Jack - Embedded)
- Représente la **connexion USB interne** qui reçoit les données
- Connecté à l'**Endpoint USB OUT** (données qui vont de l'hôte → device)
- C'est le "pont" entre le connecteur physique et le bus USB
- Taille du descripteur : **9 bytes** (contient `bNrInputPins`, `baSourceID`, `baSourcePin`)

#### 3. **Embedded OUT Jack** (MIDI OUT Jack - Embedded)
- Représente la **connexion USB interne** qui envoie les données
- Connecté à l'**Endpoint USB IN** (données qui vont du device → hôte)
- C'est le "pont" entre le bus USB et le connecteur physique
- Taille du descripteur : **9 bytes**

#### 4. **External OUT Jack** (MIDI OUT Jack - External)
- Représente le **connecteur DIN MIDI OUT physique**
- C'est là que les messages MIDI **sortent** de l'appareil vers un synthé, module, etc.
- Taille du descripteur : **9 bytes**

---

## Flux de Données - Exemple

### Scénario 1 : DAW → MidiCore → Synthétiseur

```
Logiciel DAW (PC)
    ↓ [Endpoint USB OUT]
Embedded IN Jack 5  (connexion USB interne)
    ↓
External OUT Jack 13  (connecteur DIN MIDI OUT physique - Port 1)
    ↓ [câble MIDI DIN 5 broches]
Synthétiseur Matériel
```

### Scénario 2 : Clavier MIDI → MidiCore → DAW

```
Clavier MIDI Matériel
    ↓ [câble MIDI DIN 5 broches]
External IN Jack 1  (connecteur DIN MIDI IN physique - Port 1)
    ↓
Embedded IN Jack 5  (connexion USB interne)
    ↓ [Endpoint USB IN]
Logiciel DAW (PC)
```

---

## Architecture Complète - 4 Ports MIDI

Pour **4 ports MIDI** (comme MidiCore), on a donc :

### Total de Jacks : 4 ports × 4 jacks = **16 Jacks**

#### Attribution des IDs de Jacks :

| Port | External IN | Embedded IN | Embedded OUT | External OUT |
|------|------------|-------------|--------------|--------------|
| 1    | Jack ID 1  | Jack ID 5   | Jack ID 9    | Jack ID 13   |
| 2    | Jack ID 2  | Jack ID 6   | Jack ID 10   | Jack ID 14   |
| 3    | Jack ID 3  | Jack ID 7   | Jack ID 11   | Jack ID 15   |
| 4    | Jack ID 4  | Jack ID 8   | Jack ID 12   | Jack ID 16   |

#### Connexions Logiques (pour Port 1) :

```
External IN Jack 1
    ↓ (baSourceID)
Embedded IN Jack 5  ←  Endpoint USB OUT (Cable 0)
    ↓ (routage interne)
Embedded OUT Jack 9  →  Endpoint USB IN (Cable 1)
    ↓ (baSourceID)
External OUT Jack 13
```

---

## Pourquoi Cette Complexité ?

### 1. **Flexibilité**
USB MIDI peut représenter :
- Des ports bidirectionnels (IN + OUT)
- Des ports unidirectionnels (IN seul ou OUT seul)
- Des routages complexes (merger, splitter, etc.)

### 2. **Identification Unique**
Chaque Jack a un **ID unique** (1-16 pour 4 ports), permettant de router précisément les données.

### 3. **Compatibilité**
Tous les drivers USB MIDI (Windows, macOS, Linux) comprennent ce modèle standard.

### 4. **Introspection**
Le driver peut découvrir automatiquement :
- Combien de ports MIDI sont disponibles
- Comment ils sont connectés (routage)
- Quels endpoints USB utilisent quels ports

---

## Taille des Descripteurs

Pour **4 ports MIDI** :

### Par Port (33 bytes) :
- External IN Jack : 6 bytes
- Embedded IN Jack : 9 bytes (DIFFÉRENT !)
- Embedded OUT Jack : 9 bytes
- External OUT Jack : 9 bytes

**IMPORTANT** : Les **Embedded IN Jacks** font 9 bytes car ils contiennent les champs supplémentaires :
- `bNrInputPins` : Nombre d'entrées
- `baSourceID` : ID du Jack source
- `baSourcePin` : Pin du Jack source

Les **External IN Jacks** font seulement 6 bytes car ils n'ont pas de source (ils sont la source !).

### Total pour 4 Ports :
- 16 Jacks × moyenne 7,875 bytes = **132 bytes**
- OU : 4 ports × 33 bytes/port = **132 bytes**

---

## Comparaison avec MIOS32

MIOS32 (MIDIbox) utilise **exactement la même architecture** :
- 4 Jacks par port (External IN, Embedded IN, Embedded OUT, External OUT)
- IDs de Jacks identiques (1-16 pour 4 ports)
- Connexions Endpoint identiques
- Tailles de descripteurs identiques

**Notre implémentation est 100% compatible MIOS32.**

---

## Références

- **USB Device Class Definition for MIDI Devices v1.0** (usb.org)
- **MIOS32 USB Implementation** : https://github.com/midibox/mios32/tree/master/drivers/STM32F4xx
- **Universal Serial Bus Specification 2.0**

---

## En Résumé

**"Jack" = Terminologie officielle USB MIDI, pas une invention de MidiCore.**

Chaque "port MIDI" visible par l'utilisateur = 4 Jacks internes qui gèrent le routage entre :
- Connecteurs physiques DIN MIDI (External Jacks)
- Bus USB et câbles virtuels (Embedded Jacks)

C'est le modèle standard utilisé par **tous** les devices USB MIDI du marché.

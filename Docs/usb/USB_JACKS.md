# USB MIDI "Jacks" Explained / Explications des "Jacks" USB MIDI

**Document Version:** 2.0 (Bilingual)  
**Last Updated:** 2026-01-25

---

## Table of Contents / Table des Matières

1. [English Version](#english-version)
   - [Why "Jacks" and not "Ports"?](#why-jacks-and-not-ports)
   - [The 4 Types of Jacks](#the-4-types-of-jacks)
   - [Data Flow Examples](#data-flow-examples)
   - [Complete Architecture (4 Ports)](#complete-architecture-4-ports)

2. [Version Française](#version-française)
   - [Pourquoi des "Jacks" et pas des "Ports" ?](#pourquoi-des-jacks-et-pas-des-ports-)
   - [Les 4 Types de Jacks](#les-4-types-de-jacks)
   - [Exemples de Flux de Données](#exemples-de-flux-de-données)
   - [Architecture Complète (4 Ports)](#architecture-complète-4-ports)

---

# English Version

## Why "Jacks" and not "Ports"?

**"Jack"** is the **official terminology** from the **USB Device Class Definition for MIDI Devices v1.0** specification.

This is NOT an arbitrary choice - it's defined in the official USB MIDI spec (usb.org).

---

## Conceptual Model

USB MIDI models the MIDI data flow between:
- **Physical connectors** (5-pin DIN MIDI jacks)
- **USB bus** (virtual cables)

To represent this bidirectional flow, USB MIDI uses 4 types of "Jacks":

### The 4 Types of Jacks

#### 1. **External IN Jack** (MIDI IN Jack - External)
- Represents the **physical DIN MIDI IN connector** on the device
- This is where MIDI messages **enter** the device from a keyboard, synth, etc.
- Descriptor size: **6 bytes**

#### 2. **Embedded IN Jack** (MIDI IN Jack - Embedded)
- Represents the **internal USB connection** that receives data
- Connected to the **USB OUT Endpoint** (data from PC → device)
- This is the "bridge" between the physical connector and the USB bus
- Descriptor size: **9 bytes** (contains `bNrInputPins`, `baSourceID`, `baSourcePin`)

#### 3. **Embedded OUT Jack** (MIDI OUT Jack - Embedded)
- Represents the **internal USB connection** that sends data
- Connected to the **USB IN Endpoint** (data from device → PC)
- This is the "bridge" between the USB bus and the physical connector
- Descriptor size: **9 bytes**

#### 4. **External OUT Jack** (MIDI OUT Jack - External)
- Represents the **physical DIN MIDI OUT connector** on the device
- This is where MIDI messages **exit** the device to a synth, module, etc.
- Descriptor size: **9 bytes**

---

## Data Flow Examples

### Scenario 1: DAW → MidiCore → Synthesizer

```
DAW Software (PC)
    ↓ [USB OUT Endpoint]
Embedded IN Jack 5  (internal USB connection)
    ↓
External OUT Jack 13  (physical DIN MIDI OUT connector - Port 1)
    ↓ [5-pin MIDI DIN cable]
Hardware Synthesizer
```

### Scenario 2: MIDI Keyboard → MidiCore → DAW

```
Hardware MIDI Keyboard
    ↓ [5-pin MIDI DIN cable]
External IN Jack 1  (physical DIN MIDI IN connector - Port 1)
    ↓
Embedded IN Jack 5  (internal USB connection)
    ↓ [USB IN Endpoint]
DAW Software (PC)
```

---

## Complete Architecture (4 Ports)

For **4 MIDI ports** (like MidiCore):

### Total Jacks: 4 ports × 4 jacks = **16 Jacks**

#### Jack ID Assignment:

| Port | External IN | Embedded IN | Embedded OUT | External OUT |
|------|------------|-------------|--------------|--------------|
| 1    | Jack ID 1  | Jack ID 5   | Jack ID 9    | Jack ID 13   |
| 2    | Jack ID 2  | Jack ID 6   | Jack ID 10   | Jack ID 14   |
| 3    | Jack ID 3  | Jack ID 7   | Jack ID 11   | Jack ID 15   |
| 4    | Jack ID 4  | Jack ID 8   | Jack ID 12   | Jack ID 16   |

#### Logical Connections (for Port 1):

```
External IN Jack 1  (DIN MIDI IN)
    ↓ (baSourceID)
Embedded IN Jack 5  ←  USB OUT Endpoint (Cable 0)
    ↓ (internal routing)
Embedded OUT Jack 9  →  USB IN Endpoint (Cable 0)
    ↓ (baSourceID)
External OUT Jack 13  (DIN MIDI OUT)
```

---

## Why This Complexity?

### 1. **Flexibility**
USB MIDI can represent:
- Bidirectional ports (IN + OUT)
- Unidirectional ports (IN only or OUT only)
- Complex routing (merger, splitter, etc.)

### 2. **Unique Identification**
Each Jack has a **unique ID** (1-16 for 4 ports), allowing precise data routing.

### 3. **Universal Compatibility**
All USB MIDI drivers (Windows, macOS, Linux) understand this standard model.

### 4. **Automatic Discovery**
The driver can automatically discover:
- How many MIDI ports are available
- How they are connected (routing)
- Which USB endpoints use which ports

---

## Descriptor Sizes

For **4 MIDI ports**:

### Per Port (33 bytes):
- External IN Jack: 6 bytes
- Embedded IN Jack: 9 bytes (DIFFERENT! contains source info)
- Embedded OUT Jack: 9 bytes
- External OUT Jack: 9 bytes

**IMPORTANT**: **Embedded IN Jacks** are 9 bytes because they contain additional fields:
- `bNrInputPins`: Number of inputs
- `baSourceID`: Source Jack ID
- `baSourcePin`: Source Jack pin

**External IN Jacks** are only 6 bytes because they have no source (they ARE the source!).

### Total for 4 Ports:
- 16 Jacks × average 7.875 bytes = **132 bytes**
- OR: 4 ports × 33 bytes/port = **132 bytes**

---

## Comparison with MIOS32

MIOS32 (MIDIbox) uses **exactly the same architecture**:
- 4 Jacks per port (External IN, Embedded IN, Embedded OUT, External OUT)
- Identical Jack IDs (1-16 for 4 ports)
- Identical Endpoint connections
- Identical descriptor sizes

**Our implementation is 100% MIOS32-compatible.**

---

## Terminology: "Jack" vs "Port"

### "MIDI Port" (user view)
A **MIDI port** = input + output on the physical device.

MidiCore has **4 MIDI ports**:
- Port 1: MIDI IN 1 + MIDI OUT 1
- Port 2: MIDI IN 2 + MIDI OUT 2
- Port 3: MIDI IN 3 + MIDI OUT 3
- Port 4: MIDI IN 4 + MIDI OUT 4

### "Jack" (USB/driver view)
A **Jack** = USB MIDI routing element (USB spec).

Each port = 4 internal Jacks (External IN/OUT + Embedded IN/OUT).

**Both terms coexist**:
- Marketing/user: "4 MIDI ports"
- Technical/USB: "16 MIDI Jacks (4 per port)"

---

# Version Française

## Pourquoi des "Jacks" et pas des "Ports" ?

**"Jack"** est la **terminologie officielle** de la spécification **USB Device Class Definition for MIDI Devices v1.0**.

Ce n'est PAS un choix arbitraire - c'est défini dans la spec officielle USB MIDI (usb.org).

---

## Modèle Conceptuel

USB MIDI modélise le flux de données MIDI entre :
- Les **connecteurs physiques** (prises DIN MIDI 5 broches)
- Le **bus USB** (câbles virtuels)

Pour représenter ce flux bidirectionnel, USB MIDI utilise 4 types de "Jacks" :

### Les 4 Types de Jacks

#### 1. **External IN Jack** (MIDI IN Jack - Externe)
- Représente la **prise DIN MIDI IN physique** sur l'appareil
- C'est là que les messages MIDI **entrent** depuis un clavier, synthé, etc.
- Taille du descripteur : **6 octets**

#### 2. **Embedded IN Jack** (MIDI IN Jack - Intégré)
- Représente la **connexion USB interne** qui reçoit les données
- Connecté à l'**Endpoint USB OUT** (données PC → appareil)
- C'est le "pont" entre la prise physique et le bus USB
- Taille du descripteur : **9 octets** (contient `bNrInputPins`, `baSourceID`, `baSourcePin`)

#### 3. **Embedded OUT Jack** (MIDI OUT Jack - Intégré)
- Représente la **connexion USB interne** qui envoie les données
- Connecté à l'**Endpoint USB IN** (données appareil → PC)
- C'est le "pont" entre le bus USB et la prise physique
- Taille du descripteur : **9 octets**

#### 4. **External OUT Jack** (MIDI OUT Jack - Externe)
- Représente la **prise DIN MIDI OUT physique** sur l'appareil
- C'est là que les messages MIDI **sortent** vers un synthé, module, etc.
- Taille du descripteur : **9 octets**

---

## Exemples de Flux de Données

### Scénario 1 : Logiciel → MidiCore → Synthétiseur

```
Logiciel DAW (PC)
    ↓ [Endpoint USB OUT]
Embedded IN Jack 5  (connexion USB interne)
    ↓
External OUT Jack 13  (prise DIN MIDI OUT physique - Port 1)
    ↓ [câble MIDI DIN 5 broches]
Synthétiseur Matériel
```

### Scénario 2 : Clavier MIDI → MidiCore → Logiciel

```
Clavier MIDI Matériel
    ↓ [câble MIDI DIN 5 broches]
External IN Jack 1  (prise DIN MIDI IN physique - Port 1)
    ↓
Embedded IN Jack 5  (connexion USB interne)
    ↓ [Endpoint USB IN]
Logiciel DAW (PC)
```

---

## Architecture Complète (4 Ports)

Pour **4 ports MIDI** (comme MidiCore) :

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
External IN Jack 1  (prise DIN MIDI IN)
    ↓ (baSourceID)
Embedded IN Jack 5  ←  Endpoint USB OUT (Cable 0)
    ↓ (routage interne)
Embedded OUT Jack 9  →  Endpoint USB IN (Cable 0)
    ↓ (baSourceID)
External OUT Jack 13  (prise DIN MIDI OUT)
```

---

## Pourquoi Cette Complexité ?

### 1. **Flexibilité**
USB MIDI peut représenter :
- Des ports bidirectionnels (IN + OUT)
- Des ports unidirectionnels (IN seul ou OUT seul)
- Des routages complexes (merger, splitter, etc.)

### 2. **Identification Unique**
Chaque Jack a un **ID unique** (1-16 pour 4 ports), permettant de router précisément les données MIDI.

### 3. **Compatibilité Universelle**
Tous les drivers USB MIDI (Windows, macOS, Linux) comprennent ce modèle standard.

### 4. **Découverte Automatique**
Le driver peut découvrir automatiquement :
- Combien de ports MIDI sont disponibles
- Comment ils sont connectés (routage)
- Quels endpoints USB utilisent quels ports

---

## Taille des Descripteurs

Pour **4 ports MIDI** :

### Par Port (33 octets) :
- External IN Jack : 6 octets
- Embedded IN Jack : 9 octets (DIFFÉRENT ! contient les sources)
- Embedded OUT Jack : 9 octets
- External OUT Jack : 9 octets

**IMPORTANT** : Les **Embedded IN Jacks** font 9 octets car ils contiennent les champs supplémentaires :
- `bNrInputPins` : Nombre d'entrées
- `baSourceID` : ID du Jack source
- `baSourcePin` : Pin du Jack source

Les **External IN Jacks** font seulement 6 octets car ils n'ont pas de source (ils SONT la source !).

### Total pour 4 Ports :
- 16 Jacks × moyenne 7,875 octets = **132 octets**
- OU : 4 ports × 33 octets/port = **132 octets**

---

## Comparaison avec MIOS32

MIOS32 (MIDIbox) utilise **exactement la même architecture** :
- 4 Jacks par port (External IN, Embedded IN, Embedded OUT, External OUT)
- IDs de Jacks identiques (1-16 pour 4 ports)
- Connexions Endpoint identiques
- Tailles de descripteurs identiques

**Notre implémentation est 100% compatible MIOS32.**

---

## Applications Pratiques

### Pour le Musicien
Un "port MIDI" = 1 entrée DIN + 1 sortie DIN sur l'appareil physique.

MidiCore 4x4 = **4 ports MIDI** :
- 4 prises DIN MIDI IN (pour connecter claviers, etc.)
- 4 prises DIN MIDI OUT (pour connecter synthés, modules, etc.)
- Vus par le PC/DAW comme **4 câbles MIDI USB virtuels**

### Pour Windows
Windows voit :
- 1 device USB Audio (classe 0x01)
- 2 interfaces (Audio Control + MIDIStreaming)
- 16 Jacks MIDI (4 par port × 4 ports)
- 2 endpoints Bulk (1 IN + 1 OUT)
- 4 câbles MIDI virtuels (0-3)

---

## Terminologie : "Jack" vs "Port"

### "Port MIDI" (vue utilisateur)
Un **port MIDI** = entrée + sortie sur l'appareil physique.

MidiCore a **4 ports MIDI** :
- Port 1 : MIDI IN 1 + MIDI OUT 1
- Port 2 : MIDI IN 2 + MIDI OUT 2
- Port 3 : MIDI IN 3 + MIDI OUT 3
- Port 4 : MIDI IN 4 + MIDI OUT 4

### "Jack" (vue USB/driver)
Un **Jack** = élément de routage USB MIDI (spec USB).

Chaque port = 4 Jacks internes (External IN/OUT + Embedded IN/OUT).

**Les deux termes coexistent** :
- Marketing/utilisateur : "4 ports MIDI"
- Technique/USB : "16 Jacks MIDI (4 par port)"

---

## Summary / Résumé

✅ **"Jack" = Official USB MIDI terminology** (usb.org spec v1.0) / Terminologie officielle USB MIDI

✅ **Not a MidiCore invention** - universal standard / Pas une invention de MidiCore - standard universel

✅ **1 physical MIDI port = 4 internal USB Jacks** / 1 port MIDI physique = 4 Jacks USB internes

✅ **All USB MIDI devices work this way** (keyboards, interfaces, controllers) / Tous les devices USB MIDI fonctionnent ainsi

✅ **MidiCore 4 ports = 16 Jacks = Standard USB MIDI**

---

## References

- **USB Device Class Definition for MIDI Devices v1.0** (usb.org)
- **MIOS32 USB Implementation**: https://github.com/midibox/mios32
- **Universal Serial Bus Specification 2.0**
- **Wikipedia USB MIDI**: https://en.wikipedia.org/wiki/USB-MIDI

---

**Document Status**: ✅ Complete bilingual reference  
**Last Verified**: 2026-01-25

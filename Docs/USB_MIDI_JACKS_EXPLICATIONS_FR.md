# USB MIDI "Jacks" - Explication Complète (Français)

## Pourquoi des "Jacks" et pas simplement des "Ports" ?

**"Jack"** est la **terminologie officielle** de la spécification **USB Device Class Definition for MIDI Devices v1.0**.

Ce n'est PAS un choix arbitraire - c'est défini dans la spec officielle USB MIDI (usb.org).

---

## Modèle Conceptuel USB MIDI

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

## Flux de Données - Exemple

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

## Références

- **USB Device Class Definition for MIDI Devices v1.0** (usb.org)
- **MIOS32 USB Implementation** : https://github.com/midibox/mios32
- **Universal Serial Bus Specification 2.0**
- **Wikipedia USB MIDI** : https://en.wikipedia.org/wiki/USB-MIDI

---

## En Résumé

✅ **"Jack" = Terminologie officielle USB MIDI** (spec usb.org v1.0)

✅ **Pas une invention de MidiCore** - standard universel

✅ **1 port MIDI physique = 4 Jacks USB internes**

✅ **Tous les devices USB MIDI fonctionnent ainsi** (claviers, interfaces, contrôleurs)

✅ **MidiCore 4 ports = 16 Jacks = Standard USB MIDI**

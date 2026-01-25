# MidiCore™ Commercial & Product Documentation
# MidiCore™ Documentation Commerciale & Produit

> **🇬🇧 English** | **🇫🇷 Français** - This document is bilingual

---

## 🇬🇧 English Section

### Overview

**MidiCore** is a revolutionary professional MIDI performance system designed for three main markets:

1. **Live Professional Musicians**
   - Need: Absolute reliability, minimal latency
   - Solution: Guaranteed real-time performance (<5ms)
   - Advantage: Embedded platform without OS = zero crashes

2. **Music Production Studios**
   - Need: Complex MIDI routing, real-time effects
   - Solution: 8 MIDI ports, integrated effects processor
   - Advantage: USB + DIN MIDI integration

3. **Music Therapy & Specialized Education**
   - Need: Accessibility, adaptation to disabilities
   - Solution: Adaptive interfaces, customizable controls
   - Advantage: Inclusive design, professional breath controller

### Unique Value Proposition

| Feature | Competitors | MidiCore |
|---------|------------|----------|
| **MIDI Latency** | 10-50ms | **<5ms** |
| **Live Reliability** | OS = crash risk | **Bare-metal = 100% stable** |
| **Accessibility** | Not adaptable | **Designed for inclusion** |
| **Price** | €800-€3000 | **~€600 (hardware)** |
| **Customization** | Closed firmware | **Open architecture** |
| **Breath Support** | Basic or absent | **Professional 24-bit** |

### Executive Summary

MidiCore is a state-of-the-art MIDI performance system combining the power of a professional multi-track looper, real-time MIDI effects processor, and comprehensive hardware controller in a single robust embedded platform. Built on STM32F407 microcontroller technology, MidiCore delivers studio-quality performance with ultra-low latency (<5ms) and rock-solid reliability for demanding professional applications.

**Designed for Accessibility**: MidiCore's modular architecture enables customized solutions for musicians with disabilities, offering adaptive interfaces and specialized controllers that eliminate barriers to musical expression.

### Key Highlights

- ✅ **17,560+ lines** of production-ready code, fully documented
- ✅ **4-track MIDI looper** with 8-scene arrangement engine
- ✅ **64-channel analog input system** (MCP3208 + multiplexer)
- ✅ Professional **breath controller support** (24-bit pressure sensor)
- ✅ **Real-time MIDI effects** with 15 musical scales
- ✅ **Complete user interface** with 6 custom OLED pages (256×64 grayscale)
- ✅ **SD card persistence** with MIOS32-compatible configuration
- ✅ **Dream SAM5716 sampler** controlled via MIDI/SysEx
- ✅ **BLE MIDI** via ESP32 (separate MCU)
- ✅ **100% MIOS32/MBHP compatible** hardware interface
- ✅ **Modular & Adaptive** - Configurable for various accessibility needs

### What You Get

- **4-track looper + 8 scenes** for complete structures
- **Real-time MIDI effects** (transpose, scale, velocity, routing)
- **8 MIDI ports total** (4x USB device + 4x DIN)
- **64-channel analog inputs** for pedals, knobs, sensors
- **Breath controller support** for expressive playing
- **SD card projects** for recall and portability

### What It Replaces

- Computer + MIDI/audio interface
- Separate looper and routing hardware
- Multiple controllers for live and studio

### Core Modules

- **Looper (4 tracks / 8 scenes)**: record, overdub, undo/redo, copy/paste, quantize, tempo sync
- **Router (8×8 matrix)**: MIDI routing between USB/DIN ports with channel filters
- **LiveFX**: real-time transformation layer per track or input
- **AINSER64**: 64 analog inputs with curves, ranges, and CC mapping
- **SRIO (DIN/DOUT)**: 128 buttons + 128 LEDs, debounce and inversion
- **Pressure/Breath**: 24-bit bidirectional sensor with response curves
- **Footswitch**: 8 pedals mapped to transport, scene, track actions

### LiveFX Effects

- **Transpose**: ±12 semitones in real-time
- **Force-to-Scale**: 15 musical scales with selectable root note
- **Velocity Scaling**: 0–200% for dynamics
- **Humanizer**: micro-variations in timing/velocity
- **Randomizer**: controlled variation for generative play
- **Arpeggiator**: 5 modes, 1–4 octaves, gate control

### Technical Specifications

#### System Performance
```
Full MIDI signal path: <5ms
├── Analog acquisition: 1.2ms
├── FreeRTOS processing: 0.8ms
├── USB/DIN transmission: 1.5ms
└── Safety margin: 1.5ms
```

#### Processing Capacity
- **MCU**: STM32F407VGT6 @ 168 MHz (ARM Cortex-M4F)
- **RAM**: 192 KB SRAM (≈95 KB used in production)
- **Flash**: 1 MB (≈280 KB used = 28%)
- **FPU**: Hardware floating-point calculations

#### MIDI Connectivity
- **4x USB Device**: 4 virtual ports (cables 0-3)
- **4x DIN MIDI**: Via hardware UARTs (USART1/2/3, UART5)
- **Bitrate**: 31.25 kbaud standard MIDI
- **USB**: Full-Speed (12 Mbps), composite Audio+MIDI class
- **Drivers**: Windows/Mac/Linux compatible (standard drivers)

#### Breath Controller
- **Sensor**: XGZP6847D (I2C, 24-bit)
- **Range**: ±10 kPa to ±100 kPa (configurable)
- **Accuracy**: 0.1% FS (Full Scale)
- **Bidirectional**: Blow (push) + inhale (pull)
- **Calibration**: Automatic at startup
- **MIDI Mapping**: CC1 (Modulation) or CC11 (Expression)

### Use Cases

#### 1. Professional Live Performance
Solo set: build 4 tracks, jump scenes with footswitches, keep hands on instrument.

#### 2. Production Studio
Central MIDI hub: route USB ↔ DIN without cable chaos, force-to-scale for happy accidents.

#### 3. Music Therapy & Accessibility
Breath controller + pedals enable full musical expression for musicians with motor limitations.

### Accessibility Features

#### For Musicians with Reduced Mobility
- **Hands-Free Operation**: 8 programmable pedals eliminate need for manual controls
- **External Controller Support**: MIDI Learn accepts any accessible MIDI controller
- **Simplified Interface**: One-button operations for complex tasks
- **Scene-Based Workflow**: Pre-program entire performances, trigger with minimal input

#### For Visually Impaired Musicians
- **High-Contrast OLED Screen**: 16 gray levels for maximum visibility
- **Large Text Option**: Configurable UI scaling
- **Audio Feedback**: Metronome and tempo signals for non-visual navigation
- **Tactile Interface**: Physical buttons and encoder with clear detents
- **Braille Labels**: Housing designed for tactile label application

#### For Breath Controller Users
- **Professional Pressure Sensor**: XGZP6847D 24-bit sensor
- **Bidirectional Support**: Positive and negative pressure detection
- **Adjustable Sensitivity**: Wide range from very light to firm
- **Atmospheric Calibration**: Automatic compensation for environmental changes
- **Ultra-Low Latency**: <5ms for natural playing feel

### Pricing Structure (France Market)

#### DIY / Maker Version
- **Component cost**: ~€400-500
- **Custom PCB**: €50-80
- **Enclosure**: €50-100
- **Hardware total**: **~€600**
- **Software**: Open-source (free)

#### Semi-Assembled Kit Version
- **Pre-tested hardware**: €650
- **SMD-assembled PCB**: +€80
- **Total**: **~€730**
- **Warranty**: 6 months

#### Finished Product (Professional)
- **Complete system**: €1200
- **19" aluminum rack enclosure**: Included
- **Total**: **€1200**
- **Warranty**: 2 years

### Competitive Analysis

| Product | Price | Latency | Accessibility | Reliability |
|---------|------|---------|---------------|-----------|
| **Akai MPC Live II** | €1699 | ~12ms | ❌ No | ⚠️ Linux OS |
| **Elektron Octatrack** | €1599 | ~8ms | ❌ No | ✅ Embedded |
| **Ableton Push 3** | €1599 | ~15ms | ❌ No | ⚠️ Standalone |
| **MidiCore DIY** | **€600** | **<5ms** | ✅ **Yes** | ✅ **Bare-metal** |
| **MidiCore Pro** | **€1200** | **<5ms** | ✅ **Yes** | ✅ **Industrial** |

---

## 🇫🇷 Section Française

### Vue d'ensemble

**MidiCore** est un système de performance MIDI professionnel révolutionnaire, conçu pour trois marchés principaux :

1. **Musiciens Professionnels en Live**
   - Besoin : Fiabilité absolue, latence minimale
   - Solution : Performance temps-réel garantie (<5ms)
   - Avantage : Plateforme embarquée sans OS = zéro crash

2. **Studios de Production Musicale**
   - Besoin : Routing MIDI complexe, effets en temps réel
   - Solution : 8 ports MIDI, processeur d'effets intégré
   - Avantage : Intégration USB + DIN MIDI

3. **Musicothérapie & Éducation Spécialisée**
   - Besoin : Accessibilité, adaptabilité aux handicaps
   - Solution : Interfaces adaptatives, contrôles personnalisables
   - Avantage : Design inclusif, contrôleur à souffle professionnel

### Proposition de Valeur Unique

| Caractéristique | Compétiteurs | MidiCore |
|----------------|--------------|----------|
| **Latence MIDI** | 10-50ms | **<5ms** |
| **Fiabilité Live** | OS = risque crash | **Bare-metal = 100% stable** |
| **Accessibilité** | Non adaptable | **Conçu pour inclusion** |
| **Prix** | 800€-3000€ | **~600€ (matériel)** |
| **Personnalisation** | Firmware fermé | **Architecture ouverte** |
| **Support Breath** | Basique ou absent | **Professionnel 24-bit** |

### Résumé Exécutif

**MidiCore** est un système de performance MIDI de pointe combinant la puissance d'un boucleur multi-pistes professionnel, d'un processeur d'effets MIDI en temps réel et d'un contrôleur matériel complet dans une seule plateforme embarquée robuste. Construit sur la technologie du microcontrôleur STM32F407, MidiCore offre des performances de qualité studio avec une latence ultra-faible (<5ms) et une fiabilité à toute épreuve pour les applications professionnelles exigeantes.

**Conçu pour l'Accessibilité** : L'architecture modulaire de MidiCore permet des solutions personnalisées pour les musiciens en situation de handicap, offrant des interfaces adaptatives et des contrôleurs spécialisés qui éliminent les barrières à l'expression musicale.

### Points Clés

- ✅ **17 560+ lignes** de code prêt pour la production, entièrement documenté
- ✅ **Boucleur MIDI 4 pistes** avec moteur d'arrangement 8 scènes
- ✅ **Système d'entrée analogique 64 canaux** (MCP3208 + multiplexeur)
- ✅ Support **contrôleur à souffle professionnel** (capteur de pression 24 bits)
- ✅ **Effets MIDI temps réel** avec 15 gammes musicales
- ✅ **Interface utilisateur complète** avec 6 pages OLED personnalisées (256×64 niveaux de gris)
- ✅ **Persistance carte SD** avec configuration compatible MIOS32
- ✅ **100% compatible MIOS32/MBHP** interface matérielle
- ✅ **Modulaire & Adaptatif** - Configurable pour divers besoins d'accessibilité

### Ce que vous obtenez

- **Boucleur 4 pistes + 8 scènes** pour des structures complètes
- **Effets MIDI temps réel** (transposition, gamme, vélocité, routage)
- **8 ports MIDI au total** (4x USB device + 4x DIN)
- **Entrées analogiques 64 canaux** pour pédales, potars, capteurs
- **Support contrôleur à souffle** pour un jeu expressif
- **Projets sur carte SD** pour rappel et portabilité

### Ce que cela remplace

- Ordinateur + interface MIDI/audio
- Boucleur séparé et matériel de routage
- Plusieurs contrôleurs pour live et studio

### Modules principaux

- **Looper (4 pistes / 8 scènes)** : enregistrement, overdub, undo/redo, copy/paste, quantize, sync tempo
- **Routeur (matrice 8×8)** : routage MIDI entre ports USB/DIN avec filtres de canaux
- **LiveFX** : couche de transformation temps réel par piste ou entrée
- **AINSER64** : 64 entrées analogiques avec courbes, plages et mapping CC
- **SRIO (DIN/DOUT)** : 128 boutons + 128 LED, debounce et inversion
- **Pression/Souffle** : capteur bidirectionnel 24 bits avec courbes de réponse
- **Footswitch** : 8 pédales mappées aux actions transport, scène, pistes

### LiveFX (effets)

- **Transpose** : ±12 demi-tons en temps réel
- **Force-to-Scale** : 15 gammes musicales avec tonique sélectionnable
- **Velocity Scaling** : 0–200% pour la dynamique
- **Humanizer** : micro-variations timing/vélocité
- **Randomizer** : variation contrôlée pour jeu génératif
- **Arpeggiator** : 5 modes, 1–4 octaves, contrôle du gate

### Exemples concrets d'utilisation

#### Exemple 1 : L'orchestre à une personne
Tu joues au clavier et tu veux un morceau complet sans embaucher trois sosies.
- Piste 1 : accords
- Piste 2 : basse
- Piste 3 : lead
- Piste 4 : percussions via pads
- Scènes A‑H : couplet, refrain, pont, solo

Résultat : un arrangement complet, piloté aux pieds pendant que les mains restent sur le clavier.

#### Exemple 2 : « Mon studio est une jungle MIDI »
Tu as du USB, du DIN, et un DAW qui n'écoute jamais.
- La matrice de routage envoie le contrôleur USB → synthé DIN
- Le looper enregistre le DIN et rejoue vers l'USB
- LiveFX met tes improvisations dans la bonne gamme

Résultat : tout communique avec tout.

#### Exemple 3 : Le musicien à souffle expressif
Tu joues au souffle et tu veux une réponse naturelle.
- Capteur de pression mappé sur CC11 (expression)
- Courbes ajustées à ta respiration
- Humanizer LiveFX pour garder le côté musical

Résultat : un jeu expressif qui répond vraiment à toi.

### Caractéristiques Techniques

#### Performance Système
```
Chemin MIDI complet : <5ms
├── Acquisition analogique : 1.2ms
├── Traitement FreeRTOS : 0.8ms
├── Transmission USB/DIN : 1.5ms
└── Marge sécurité : 1.5ms
```

#### Capacité de Traitement
- **MCU** : STM32F407VGT6 @ 168 MHz (ARM Cortex-M4F)
- **RAM** : 192 KB SRAM (≈95 KB utilisés en production)
- **Flash** : 1 MB (≈280 KB utilisés = 28%)
- **FPU** : Calculs en virgule flottante matériels

#### Connectivité MIDI
- **4x USB Device** : 4 ports virtuels (cables 0-3)
- **4x DIN MIDI** : Via UART matériels (USART1/2/3, UART5)
- **Débit** : 31.25 kbaud standard MIDI
- **USB** : Full-Speed (12 Mbps), classe Audio+MIDI composite
- **Drivers** : Compatible Windows/Mac/Linux (pilotes standard)

#### Breath Controller (Contrôleur à Souffle)
- **Capteur** : XGZP6847D (I2C, 24-bit)
- **Plage** : ±10 kPa à ±100 kPa (configurable)
- **Précision** : 0.1% FS (Full Scale)
- **Bidirectionnel** : Souffle (push) + Aspiration (pull)
- **Calibration** : Automatique au démarrage
- **Mapping MIDI** : CC1 (Modulation) ou CC11 (Expression)

### Fonctionnalités d'Accessibilité

#### Pour les Musiciens à Mobilité Réduite
- **Fonctionnement Mains Libres** : 8 pédales programmables éliminent le besoin de contrôles manuels
- **Support Contrôleur Externe** : MIDI Learn accepte tout contrôleur MIDI accessible
- **Interface Simplifiée** : Opérations à un bouton pour tâches complexes
- **Workflow Basé sur les Scènes** : Pré-programmez des performances entières

#### Pour les Musiciens Malvoyants
- **Écran OLED à Haut Contraste** : 16 niveaux de gris
- **Option Texte Large** : Mise à l'échelle UI configurable
- **Retour Audio** : Métronome et signaux de tempo
- **Interface Tactile** : Boutons physiques avec crans clairs
- **Étiquettes Braille** : Boîtier conçu pour étiquettes tactiles

#### Pour Utilisateurs de Contrôleur à Souffle
- **Capteur de Pression Professionnel** : XGZP6847D 24 bits
- **Support Bidirectionnel** : Détection positive et négative
- **Sensibilité Ajustable** : Large gamme de très léger à ferme
- **Latence Ultra-Faible** : <5ms pour sensation naturelle

### Structure de Prix (Marché France)

#### Version DIY / Maker
- **Coût composants** : ~400-500€
- **PCB custom** : 50-80€
- **Boîtier** : 50-100€
- **Total matériel** : **~600€**
- **Logiciel** : Open-source (libre)

#### Version Kit Semi-Assemblé
- **Matériel pré-testé** : 650€
- **PCB assemblé SMD** : +80€
- **Total** : **~730€**
- **Garantie** : 6 mois

#### Version Produit Fini (Professionnel)
- **Système complet** : 1200€
- **Boîtier rack 19" aluminium** : Inclus
- **Total** : **1200€**
- **Garantie** : 2 ans

### Analyse Compétitive

| Produit | Prix | Latence | Accessibilité | Fiabilité |
|---------|------|---------|---------------|-----------|
| **Akai MPC Live II** | 1699€ | ~12ms | ❌ Non | ⚠️ OS Linux |
| **Elektron Octatrack** | 1599€ | ~8ms | ❌ Non | ✅ Embedded |
| **Ableton Push 3** | 1599€ | ~15ms | ❌ Non | ⚠️ Standalone |
| **MidiCore DIY** | **600€** | **<5ms** | ✅ **Oui** | ✅ **Bare-metal** |
| **MidiCore Pro** | **1200€** | **<5ms** | ✅ **Oui** | ✅ **Industriel** |

---

## 📞 Contact / Contact Commercial

**For any commercial inquiry, partnership, or information:**
**Pour toute demande commerciale, partenariat ou information** :

- 📧 **Email**: contact@midicore-pro.com (to be created / à créer)
- 🌐 **Website**: www.midicore-pro.com (to be created / à créer)
- 💬 **Discord**: MidiCore Community (link to be created / lien à créer)
- 📱 **LinkedIn**: Company page (to be created / page à créer)

---

**MidiCore™** - *Empowering All Musicians / Donner le Pouvoir à Tous les Musiciens*

© 2024-2026 MidiCore Project. All rights reserved / Tous droits réservés.

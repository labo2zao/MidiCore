# MidiCore™ Système Professionnel de Performance MIDI

<div align="center">

**Boucleur MIDI Multi-Pistes Avancé & Moteur de Traitement en Temps Réel**

*Solution Professionnelle pour Performance Live, Production Studio & Éducation Musicale*

*Conçu pour l'Accessibilité - Donner du Pouvoir aux Musiciens en Situation de Handicap*

[![STM32](https://img.shields.io/badge/STM32-F407VGT6-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f407vg.html)
[![License](https://img.shields.io/badge/license-Commercial-green.svg)](#licence)
[![Version](https://img.shields.io/badge/version-2.0-brightgreen.svg)](#)
[![Accessibilité](https://img.shields.io/badge/Accessibilit%C3%A9-Conception%20Inclusive-purple.svg)](#accessibilité--technologie-musicale-adaptative)

</div>

---

## 🎯 Résumé Exécutif

**MidiCore** est un système de performance MIDI de pointe combinant la puissance d'un boucleur multi-pistes professionnel, d'un processeur d'effets MIDI en temps réel et d'un contrôleur matériel complet dans une seule plateforme embarquée robuste. Construit sur la technologie du microcontrôleur STM32F407, MidiCore offre des performances de qualité studio avec une latence ultra-faible (<5ms) et une fiabilité à toute épreuve pour les applications professionnelles exigeantes.

**Conçu pour l'Accessibilité** : L'architecture modulaire de MidiCore permet des solutions personnalisées pour les musiciens en situation de handicap, offrant des interfaces adaptatives et des contrôleurs spécialisés qui éliminent les barrières à l'expression musicale.

### **Points Clés**

- ✅ **17 560+ lignes** de code prêt pour la production, entièrement documenté
- ✅ **Boucleur MIDI 4 pistes** avec moteur d'arrangement 8 scènes
- ✅ **Système d'entrée analogique 64 canaux** (MCP3208 + multiplexeur)
- ✅ Support **contrôleur à souffle professionnel** (capteur de pression 24 bits)
- ✅ **Effets MIDI temps réel** avec 15 gammes musicales
- ✅ **Interface utilisateur complète** avec 6 pages OLED personnalisées (256×64 niveaux de gris)
- ✅ **Persistance carte SD** avec configuration compatible MIOS32
- ✅ **Framework de test modulaire** pour assurance qualité
- ✅ **100% compatible MIOS32/MBHP** interface matérielle
- ✅ **Modulaire & Adaptatif** - Configurable pour divers besoins d'accessibilité

---

## 🛒 Aperçu pour les Acheteurs

### **Pour qui ?**
- Performeurs live qui exigent une fiabilité absolue sans ordinateur
- Studios voulant un hub MIDI dédié à faible latence
- Éducateurs/therapeutes qui ont besoin d’interfaces adaptables

### **Ce que vous obtenez (en bref)**
- **Boucleur 4 pistes + 8 scènes** pour des structures complètes
- **Effets MIDI temps réel** (transposition, gamme, vélocité, routage)
- **8 ports MIDI au total** (4x USB device + 4x DIN)
- **Entrées analogiques 64 canaux** pour pédales, potars, capteurs
- **Support contrôleur à souffle** pour un jeu expressif
- **Projets sur carte SD** pour rappel et portabilité

### **Ce que cela remplace**
- Ordinateur + interface MIDI/audio
- Boucleur séparé et matériel de routage
- Plusieurs contrôleurs pour live et studio

### **Checklist d’intégration**
- Compatible **Windows/Mac/Linux** via USB MIDI class-compliant
- MIDI DIN pour synthés legacy et matériel de scène
- Carte SD pour sauvegarde et transfert des projets
- Entrées extensibles pour configurations d’accessibilité

---

## 🧩 Détails modules & FX (référence acheteurs)

### **Modules principaux**
- **Looper (4 pistes / 8 scènes)** : enregistrement, overdub, undo/redo, copy/paste, quantize, sync tempo
- **Routeur (matrice 8×8)** : routage MIDI entre ports USB/DIN avec filtres de canaux
- **LiveFX** : couche de transformation temps réel par piste ou entrée
- **AINSER64** : 64 entrées analogiques avec courbes, plages et mapping CC
- **SRIO (DIN/DOUT)** : 128 boutons + 128 LED, debounce et inversion
- **Pression/Souffle** : capteur bidirectionnel 24 bits avec courbes de réponse
- **Footswitch** : 8 pédales mappées aux actions transport, scène, pistes

### **LiveFX (effets) en bref**
- **Transpose** : ±12 demi-tons en temps réel
- **Force-to-Scale** : 15 gammes musicales avec tonique sélectionnable
- **Velocity Scaling** : 0–200% pour la dynamique
- **Humanizer** : micro-variations timing/vélocité
- **Randomizer** : variation contrôlée pour jeu génératif
- **Arpeggiator** : 5 modes, 1–4 octaves, contrôle du gate

---

## 🎛️ « Mais concrètement, je fais quoi avec ? » (exemples réels)

### **Exemple 1 : L’orchestre à une personne**
Tu joues au clavier et tu veux un morceau complet sans embaucher trois sosies.
- Piste 1 : accords
- Piste 2 : basse
- Piste 3 : lead
- Piste 4 : percussions via pads
- Scènes A‑H : couplet, refrain, pont, solo
Résultat : un arrangement complet, piloté aux pieds pendant que les mains restent sur le clavier. Bonus : tu as l’air d’un magicien. 🧙‍♂️

### **Exemple 2 : « Mon studio est une jungle MIDI »**
Tu as du USB, du DIN, et un DAW qui n’écoute jamais.
- La matrice de routage envoie le contrôleur USB → synthé DIN
- Le looper enregistre le DIN et rejoue vers l’USB
- LiveFX met tes improvisations dans la bonne gamme
Résultat : tout communique avec tout, sans sacrifices rituels aux dieux du MIDI.

### **Exemple 3 : Le musicien à souffle expressif**
Tu joues au souffle et tu veux une réponse naturelle.
- Capteur de pression mappé sur CC11 (expression)
- Courbes ajustées à ta respiration
- Humanizer LiveFX pour garder le côté musical
Résultat : un jeu expressif qui répond vraiment à toi.

### **Exemple 4 : « Le mode entraînement sans larmes »**
Tu veux améliorer le timing et avoir un vrai feedback.
- Métronome + entraînement rythmique
- Zones de timing visuelles
- Scènes pour les exercices
Résultat : tu progresses plus vite et tu arrêtes de blâmer le batteur (presque).

---

## 🧪 « Est‑ce que ça marche avec mon matos ? » (check rapide)
- **DAW** : 4 ports MIDI USB (câbles 0–3)
- **Synthés vintage** : 4 ports DIN pour les rigs old‑school
- **Contrôleurs** : MIDI Learn mappe CC/Notes aux actions
- **Carte SD** : sauvegarde des projets, transport facile

Si ton matériel parle MIDI, il y a de grandes chances qu’il s’entende bien avec MidiCore.

---

## ♿ Accessibilité & Technologie Musicale Adaptative

### **Engagement envers la Conception Inclusive**

MidiCore est développé avec un **fort engagement envers l'accessibilité**, reconnaissant que la musique est un langage universel qui devrait être accessible à tous, quelle que soit la capacité physique. L'architecture modulaire du système a été spécifiquement conçue pour soutenir les musiciens en situation de handicap, permettant des adaptations personnalisées pour divers besoins.

### **Capacités Adaptatives**

#### **Pour les Musiciens à Mobilité Réduite**
- **Fonctionnement Mains Libres** : 8 pédales programmables éliminent le besoin de contrôles manuels
- **Support Contrôleur Externe** : Système MIDI Learn accepte tout contrôleur MIDI accessible
- **Interface Simplifiée** : Opérations à un bouton pour tâches complexes
- **Workflow Basé sur les Scènes** : Pré-programmez des performances entières, déclenchement avec entrée minimale
- **Prêt pour Commande Vocale** : MIDI Learn peut mapper des dispositifs voix-vers-MIDI

#### **Pour les Musiciens Malvoyants**
- **Écran OLED à Haut Contraste** : 16 niveaux de gris pour visibilité maximale
- **Option Texte Large** : Mise à l'échelle UI configurable (via ajustement taille de police)
- **Retour Audio** : Métronome et signaux de tempo pour navigation non visuelle
- **Interface Tactile** : Boutons physiques et encodeur avec crans clairs
- **Étiquettes Braille** : Boîtier conçu pour application d'étiquettes tactiles

#### **Pour Utilisateurs de Contrôleur à Souffle (Handicaps Respiratoires)**
- **Capteur de Pression Professionnel** : Capteur XGZP6847D 24 bits
- **Support Bidirectionnel** : Détection de pression positive et négative
- **Sensibilité Ajustable** : Large gamme de très léger à ferme
- **Calibration Atmosphérique** : Compensation automatique des changements environnementaux
- **Plages de Pression Multiples** : Configurable ±10 à ±100 kPa
- **Latence Ultra-Faible** : <5ms pour sensation de jeu naturelle

#### **Pour Différences Membres Supérieurs**
- **Fonctionnement à Une Main** : Toutes les fonctions principales accessibles d'une main
- **Mappage Bouton Personnalisé** : Assignez n'importe quelle fonction à n'importe quel bouton physique
- **Disposition Pédale Adaptive** : Placez les contrôles où accessible
- **Contrôle Proportionnel** : Capteurs de pression peuvent remplacer mouvements complexes des doigts
- **Fonctions Macro** : Un bouton déclenche plusieurs actions

#### **Pour Accessibilité Cognitive**
- **Workflow Visuel** : Grille de scènes montre structure de chanson d'un coup d'œil
- **Statut Codé Couleur** : Indicateurs visuels clairs pour tous les états système
- **Modes Simplifiés** : Masquer fonctionnalités avancées, exposer seulement l'essentiel
- **Navigation Cohérente** : Même modèle sur toutes les pages UI
- **Annuler/Refaire** : Workflow tolérant permet exploration sans crainte

### **Framework d'Adaptation Modulaire**

L'**architecture modulaire** de MidiCore permet aux professionnels de santé, thérapeutes et ingénieurs de créer des adaptations spécialisées :

#### **Méthodes d'Entrée Configurables**
```c
// Activer seulement les modules d'entrée nécessaires
#define MODULE_ENABLE_AINSER64    1  // Capteurs analogiques (pression, position)
#define MODULE_ENABLE_SRIO        1  // Boutons/interrupteurs numériques
#define MODULE_ENABLE_PRESSURE    1  // Contrôleur à souffle
#define MODULE_ENABLE_FOOTSWITCH  1  // Pédales
#define MODULE_ENABLE_USB_HOST    1  // Contrôleurs adaptatifs externes
```

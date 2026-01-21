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


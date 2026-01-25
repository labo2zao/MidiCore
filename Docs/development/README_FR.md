# Documentation de Développement

> 🇬🇧 [English version available](README.md)

Documentation technique, détails d'implémentation et informations de compatibilité pour les développeurs MidiCore.

## Détails d'Implémentation

### Implémentation USB
- **[USB Host MIDI](README_USBH_MIDI.md)** - Implémentation USB Host pour STM32F407
- **[Intégration USB Device](README_USB_DEVICE_INTEGRATION.md)** - Guide d'intégration USB MIDI Device
- **[Implémentation Finale](README_IMPLEMENTATION_FINAL.md)** - Guide de configuration finale USB MIDI

### Infrastructure de Test
- **[Résumé d'Implémentation](IMPLEMENTATION_SUMMARY.md)** - Implémentation de l'infrastructure de test des modules
- **[Correction Mode Production](PRODUCTION_MODE_FIX.md)** - Problèmes critiques de production résolus
- **[Résumé de Solution](SOLUTION_SUMMARY.md)** - Correction de l'erreur Windows 0xC00000E5

## Compatibilité & Portabilité

### Compatibilité MIOS32
- **[Index de Compatibilité](COMPATIBILITY_INDEX.md)** - Index de la documentation de compatibilité MIOS32
- **[Résumé de Compatibilité](COMPATIBILITY_SUMMARY.md)** - Référence rapide (98.95% compatible)
- **[Rapport de Compatibilité des Pilotes](DRIVER_COMPATIBILITY_REPORT.md)** - Analyse détaillée des pilotes
- **[Rapport de Compatibilité LoopA](LOOPA_COMPATIBILITY_REPORT.md)** - Compatibilité des fonctionnalités LoopA

### Portabilité de Plateforme
- **[Guide de Portabilité](README_PORTABILITY.md)** - Guide de migration STM32F4 → STM32H7
  - Considérations mémoire
  - Différences de périphériques
  - Optimisations de performance
  - Liste de contrôle de migration

## Architecture

MidiCore est construit sur une architecture modulaire comprenant:
- **Couche d'Abstraction HAL** - Abstraction matérielle pour la portabilité
- **Tâches FreeRTOS** - Multitâche temps réel
- **Modules de Service** - Looper, UI, Router, Gestionnaire de Patch
- **Couche Pilote** - AINSER64, SRIO, OLED, Carte SD

## Flux de Développement

1. **Comprendre** l'architecture à travers les docs d'implémentation
2. **Vérifier la compatibilité** pour votre plateforme cible
3. **Suivre** le guide de portabilité pour les changements de plateforme
4. **Référencer** l'implémentation USB pour les fonctionnalités USB
5. **Tester** en utilisant l'infrastructure de test

## Documentation Associée

- **[Documentation USB](../usb/)** - Guides USB détaillés et corrections de bugs
- **[Compatibilité MIOS32](../mios32/)** - Guides spécifiques MIOS32
- **[Configuration](../configuration/)** - Configuration du système
- **[Tests](../testing/)** - Procédures de test

## Pour les Contributeurs

Lors de l'ajout de nouvelles fonctionnalités ou de corrections de bugs:
1. Maintenir l'abstraction HAL pour la portabilité
2. Suivre les standards de codage existants
3. Ajouter des tests pour les nouvelles fonctionnalités
4. Mettre à jour les rapports de compatibilité si nécessaire
5. Documenter les changements USB en détail

# Compatibilité MIOS32

> 🇬🇧 [English version available](README.md)

Guides de compatibilité et de migration MIOS32 pour MidiCore.

## Guides Disponibles

### Documentation de Compatibilité
- **[Compatibilité MIOS32](MIOS32_COMPATIBILITY.md)** - Évaluation globale de compatibilité (98.95% compatible)
- **[Comparaison Approfondie MIOS32](MIOS32_DEEP_COMPARISON.md)** - Comparaison détaillée fonctionnalité par fonctionnalité
- **[Analyse des Descripteurs MIOS32](MIOS32_DESCRIPTOR_ANALYSIS.md)** - Analyse de compatibilité des descripteurs USB

### Guides d'Implémentation
- **[Guide d'Implémentation USB MIOS32](MIOS32_USB_IMPLEMENTATION_GUIDE.md)** - Guide d'implémentation USB MIDI
- **[Guide du Mode Dual MIOS32](MIOS32_DUAL_MODE_GUIDE.md)** - Opération USB en mode dual
- **[Commutation Automatique de Style MIOS32](MIOS32_STYLE_AUTO_SWITCHING.md)** - Commutation automatique de mode

## Aperçu de Compatibilité

MidiCore maintient une haute compatibilité avec l'écosystème MIOS32/MIDIbox:

### Compatibilité Matérielle (100%)
- **AINSER64**: Matériel et protocole 100% compatibles
- **SRIO**: 100% compatible (conventions de nommage MBHP)
- **MIDI DIN**: 100% compatible (31.25k bauds, running status)
- **Bootloader USB MIDI**: Protocole SysEx compatible MIOS32

### Compatibilité Logicielle (98.95%)
- **Format de Patch**: Format TXT clé=valeur compatible
- **Looper**: Basé sur LoopA (96 PPQN, quantization)
- **API**: Signatures et comportement de fonctions similaires
- **Configuration**: Système compatible d'activation/désactivation de modules

### Différences Connues
- FreeRTOS vs ordonnanceur de tâches MIOS32 (APIs différentes)
- Couche d'abstraction HAL (spécifique à MidiCore)
- Certaines implémentations de périphériques optimisées pour STM32F4

## Migration depuis MIOS32

1. **Lire** [Compatibilité MIOS32](MIOS32_COMPATIBILITY.md) pour un aperçu
2. **Comparer** les fonctionnalités dans [Comparaison Approfondie](MIOS32_DEEP_COMPARISON.md)
3. **Réviser** l'implémentation USB dans [Guide d'Implémentation USB](MIOS32_USB_IMPLEMENTATION_GUIDE.md)
4. **Vérifier** la compatibilité des descripteurs dans [Analyse des Descripteurs](MIOS32_DESCRIPTOR_ANALYSIS.md)
5. **Tester** votre application avec MidiCore

## Documentation Associée

- **[Développement](../development/)** - Détails d'implémentation technique
- **[Configuration](../configuration/)** - Configuration UART et des modules
- **[Matériel](../hardware/)** - Compatibilité matérielle MBHP
- **[USB](../usb/)** - Implémentation USB MIDI

## Ressources Externes

- [Projet MIOS32](http://www.midibox.org/mios32/)
- [Wiki MIDIbox](http://wiki.midibox.org/)
- [Matériel MBHP](http://www.ucapps.de/mbhp.html)

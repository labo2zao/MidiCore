# Documentation USB

> 🇬🇧 [English version available](README.md)

Guides d'implémentation et de débogage USB MIDI pour MidiCore.

## Guides Disponibles

### Configuration & Installation
- **[Guide de Configuration USB](USB_CONFIGURATION_GUIDE.md)** - Guide complet de configuration USB
- **[Guide Device et Host USB](USB_DEVICE_AND_HOST_GUIDE.md)** - Guide complet device et host
- **[Explication Host et Device USB](USB_HOST_AND_DEVICE_EXPLAINED.md)** - Explication de l'architecture

### Débogage
- **[Guide de Debug USB](USB_DEBUG_GUIDE.md)** - Guide complet de débogage des problèmes USB
- **[Démarrage Rapide Debug UART USB](USB_DEBUG_UART_QUICKSTART.md)** - Configuration rapide du debug via UART

### Intégration CubeMX
- **[Problème OTG CubeMX Expliqué](CUBEMX_OTG_ISSUE_EXPLAINED.md)** - Résolution du problème OTG
- **[Protéger USB Host de CubeMX](PROTECT_USB_HOST_FROM_CUBEMX.md)** - Protection contre la régénération CubeMX
- **[Protection USB MIDI CubeMX](USB_MIDI_CUBEMX_PROTECTION.md)** - Guide de protection spécifique MIDI

### Jacks USB MIDI
- **[Jacks USB MIDI Expliqués](USB_MIDI_JACKS_EXPLAINED.md)** - Configuration des jacks (Anglais)
- **[Explications Jacks USB MIDI](USB_MIDI_JACKS_EXPLICATIONS_FR.md)** - Configuration des jacks (Français)

### Documentation Technique
- **[Structure des Descripteurs USB](USB_DESCRIPTOR_STRUCTURE.txt)** - Détails de la structure des descripteurs
- **[Analyse des Descripteurs USB MIDI](USB_MIDI_DESCRIPTOR_ANALYSIS.md)** - Analyse détaillée des descripteurs
- **[Audit du Protocole USB MIDI](USB_MIDI_PROTOCOL_AUDIT.md)** - Audit de l'implémentation du protocole
- **[Moteur SysEx USB MIDI](USB_MIDI_SYSEX_ENGINE.md)** - Guide d'implémentation SysEx

### Problèmes Connus & Corrections
- **[Bugs de la Bibliothèque USB STM32](STM32_USB_LIBRARY_BUGS.md)** - Problèmes connus de la bibliothèque STM32
- **[Correction Bug Endpoint Bulk USB](USB_BULK_ENDPOINT_BUG_FIX.md)** - Correction du bug endpoint bulk
- **[Correction Bug Taille Descripteur USB](USB_DESCRIPTOR_SIZE_BUG_FIX.md)** - Correction de taille de descripteur
- **[Analyse d'Échec Descripteur Device USB](USB_DEVICE_DESCRIPTOR_FAILURE_ANALYSIS.md)** - Échecs du descripteur device
- **[Correction IAD USB](USB_IAD_FIX.md)** - Correction Interface Association Descriptor
- **[Correction Suppression IAD USB](USB_IAD_REMOVAL_FIX.md)** - Correction de suppression IAD
- **[Correction Bug Header MS USB MIDI](USB_MIDI_MS_HEADER_BUG_FIX.md)** - Correction du bug header MS

## Aperçu USB

MidiCore implémente un support USB MIDI complet:
- **USB MIDI Device**: Le STM32 apparaît comme périphérique MIDI pour l'ordinateur
- **USB MIDI Host**: Connexion de contrôleurs/claviers MIDI USB externes
- **Mode Dual**: Device et Host peuvent fonctionner simultanément (avec support matériel)
- **Compatible MIOS32**: Utilise le protocole USB MIDI standard MIOS32

## Démarrage Rapide

1. **Configurer**: Suivre le [Guide de Configuration USB](USB_CONFIGURATION_GUIDE.md)
2. **Protéger**: Utiliser la [Protection CubeMX](USB_MIDI_CUBEMX_PROTECTION.md) avant régénération
3. **Débugger**: Consulter le [Guide de Debug USB](USB_DEBUG_GUIDE.md) en cas de problème
4. **Corriger**: Référencer les corrections de bugs connus si nécessaire

## Documentation Associée

- **[Développement](../development/)** - Détails d'implémentation USB
- **[MIOS32](../mios32/)** - Compatibilité USB MIOS32
- **[Configuration](../configuration/)** - Configuration du système
- **[Tests](../testing/)** - Procédures de test USB

# Documentation MidiCore

> **✨ La documentation a été récemment réorganisée !** Tous les fichiers de documentation ont été consolidés et organisés dans cette structure de dossiers. Consultez [DOCUMENTATION_REORGANIZATION.md](DOCUMENTATION_REORGANIZATION.md) pour connaître les déplacements effectués et leur nouvelle localisation.

> **🌐 Support bilingue :** La plupart des dossiers incluent maintenant un README_FR.md pour les traductions françaises. Recherchez les indicateurs 🇫🇷/🇬🇧 dans les README des dossiers.

Documentation complète du système de contrôleur MIDI MidiCore.

## 📚 Structure de la documentation

### 🚀 [Bien démarrer](getting-started/) • [🇫🇷 Français](getting-started/README_FR.md)
Guides de démarrage rapide et intégration du projet
- [Guide principal](getting-started/README.md) - Guide d’intégration consolidé
- [Que faire maintenant](getting-started/WHAT_TO_DO_NOW.md) - Plan d’action clair

### 👤 [Guides utilisateur](user-guides/) • [🇫🇷 Français](user-guides/README_FR.md)
Fonctionnalités et guides destinés aux utilisateurs finaux
- [Vue d’ensemble](user-guides/README.md) - Vue d’ensemble des guides utilisateur
- [Guide des pédales](user-guides/FOOTSWITCH_GUIDE.md) - Intégration des pédales
- [Plan des fonctionnalités LoopA](user-guides/LOOPA_FEATURES_PLAN.md) - Fonctionnalités du looper
- [Implémentation de l’UI](user-guides/UI_LOOPA_IMPLEMENTATION.md) - Guide de l’interface
- [Tests des pages UI](user-guides/UI_PAGE_TESTING_GUIDE.md) - Guide de tests UI
- [Système d’automatisation](user-guides/AUTOMATION_SYSTEM.md) - Documentation d’automatisation
- [Analyse des boutons SCS](user-guides/SCS_BUTTONS_ANALYSIS.md) - Analyse du système de boutons

### 🔌 [Matériel](hardware/) • [🇫🇷 Français](hardware/README_FR.md)
Installation matérielle, câblage et informations de brochage
- [Vue d’ensemble](hardware/README.md) - Aperçu de la documentation matérielle
- [Guide de câblage OLED](hardware/OLED_WIRING_GUIDE.md) - Câblage de l’écran OLED (SSD1322/SSD1306 compatible LoopA)
- [Résumé des améliorations OLED](hardware/OLED_IMPROVEMENTS_SUMMARY.md) - Améliorations de l’affichage OLED
- [Historique des correctifs SSD1322](hardware/OLED_SSD1322_FIX_HISTORY.md) - Historique des correctifs SSD1322
- [Référence technique SSD1322](hardware/OLED_SSD1322_TECHNICAL_REFERENCE.md) - Référence technique
- [Test rapide OLED](hardware/OLED_QUICK_TEST.md) - Procédure de test rapide OLED
- [Guide de la page de test OLED](hardware/OLED_TEST_PAGE_GUIDE.md) - Guide des pages de test
- [Dépannage OLED](hardware/OLED_TROUBLESHOOTING.md) - Guide de dépannage
- [Brochage connecteur OLED J1](hardware/J1_OLED_CONNECTOR_PINOUT.md) - Brochage du connecteur
- [Résumé des améliorations UI](hardware/UI_IMPROVEMENTS_SUMMARY.md) - Améliorations de l’UI
- [Améliorations du rendu UI](hardware/UI_RENDERING_IMPROVEMENTS.md) - Améliorations du rendu
- [Disposition UI ergonomique](hardware/UI_LAYOUT_ERGONOMIC.md) - Guide de disposition ergonomique
- [Navigation par touches combinées](hardware/COMBINED_KEY_NAVIGATION.md) - Système de navigation par touches
- [Guide de régénération CubeMX](hardware/CUBEMX_REGENERATION_GUIDE.md) - Protéger le code personnalisé
- [NHD-OLEDSSD1322DISP.pdf](hardware/NHD-OLEDSSD1322DISP.pdf) - Fiche technique OLED
- [SSD1322.pdf](hardware/SSD1322%20(4).pdf) - Fiche technique du contrôleur SSD1322

### ⚙️ [Configuration](configuration/) • [🇫🇷 Français](configuration/README_FR.md)
Configuration et paramétrage du système
- [Vue d’ensemble](configuration/README.md) - Aperçu de la configuration
- [Configuration des modules](configuration/README_MODULE_CONFIG.md) - Activer/désactiver des modules
- [Configuration UART MIOS32](configuration/README_MIOS32_UART_CONFIG.md) - UART et débogage
- [Configuration SPI](configuration/SPI_CONFIGURATION_REFERENCE.md) - Paramètres SPI
- [Régénération CubeMX](configuration/CUBEMX_REGENERATION_GUIDE.md) - Protéger le code personnalisé
- [Protection FreeRTOS](configuration/FREERTOS_PROTECTION_GUIDE.md) - Protéger les tâches de CubeMX

### 🧪 [Tests](testing/) • [🇫🇷 Français](testing/README_FR.md)
Procédures de test et validation
- [Vue d’ensemble](testing/README.md) - Aperçu de la documentation de test
- [Démarrage rapide des tests](testing/TESTING_QUICKSTART.md) - Exemples de test rapide
- [Guide de test des modules](testing/README_MODULE_TESTING.md) - Guide complet de test
- [Protocole de test](testing/TESTING_PROTOCOL.md) - Procédures complètes (300+ tests)
- [Protocole de test OLED](testing/OLED_TEST_PROTOCOL.md) - Validation de l’écran OLED (15 modes de test)
- [Exécution des tests](testing/TEST_EXECUTION.md) - Détails d’exécution
- [Rapport de validation des tests](testing/TEST_VALIDATION_REPORT.md) - Résultats de validation
- [Checklist de debug](testing/FINAL_DEBUG_CHECKLIST.md) - Vérification systématique
- [Phase 2 Complete Module Test All](testing/PHASE_2_COMPLETE_MODULE_TEST_ALL.md) - Tests phase 2
- [Phase 3 Advanced Features](testing/PHASE_3_ADVANCED_FEATURES.md) - Tests phase 3 (depuis Docs/testing/)
- [Phase 3 Complete Advanced Features](testing/PHASE_3_COMPLETE_ADVANCED_FEATURES.md) - Tests phase 3 (déplacé depuis la racine)
- [README Tests](testing/README_TESTS.md) - Documentation des tests
- [README USB MIDI Test](testing/README_USB_MIDI_TEST.md) - Tests USB MIDI
- [Guide de test du contrôleur de souffle](testing/BREATH_CONTROLLER_TEST_GUIDE.md) - Tests du contrôleur de souffle
- [Exemples MIDI DIN](testing/MIDI_DIN_EXAMPLES.md) - Exemples de tests MIDI DIN
- [Test MIDI DIN LiveFX](testing/MIDI_DIN_LIVEFX_TEST.md) - Tests LiveFX
- [Nouvelles fonctionnalités MIDI DIN](testing/MIDI_DIN_NEW_FEATURES.md) - Tests des nouvelles fonctionnalités
- [Module Test All](testing/MODULE_TEST_ALL.md) - Tests complets des modules
- [Feuille de route Module Test MIDI DIN](testing/MODULE_TEST_MIDI_DIN_ALL_PHASES_ROADMAP.md) - Feuille de route des tests
- [Résumé Module Test MIDI DIN](testing/MODULE_TEST_MIDI_DIN_SUMMARY.md) - Résumé des tests
- [Module Test Patch SD](testing/MODULE_TEST_PATCH_SD.md) - Tests Patch SD
- [Module Test Router Output](testing/MODULE_TEST_ROUTER_OUTPUT.md) - Tests du routeur
- [Guide d’implémentation Phase B](testing/PHASE_B_IMPLEMENTATION_GUIDE.md) - Guide Phase B
- [Démarrage rapide Patch SD](testing/QUICKSTART_PATCH_SD.md) - Démarrage rapide Patch SD

### 🔧 [Développement](development/) • [🇫🇷 Français](development/README_FR.md)
Documentation technique et détails d’implémentation
- [Vue d’ensemble](development/README.md) - Aperçu de la documentation de développement
- [Implémentation du bootloader](development/BOOTLOADER_IMPLEMENTATION.md) - Bootloader USB MIDI
- [Vérification du bootloader](development/BOOTLOADER_VERIFICATION.md) - Guide de vérification
- [README Bootloader](development/README_BOOTLOADER.md) - Documentation principale du bootloader
- [README Bootloader RAM](development/README_BOOTLOADER_RAM.md) - Bootloader en RAM
- [Modes de build](development/BUILD_MODES.md) - Modes de configuration
- [Résumé complet de l’implémentation](development/COMPLETE_IMPLEMENTATION_SUMMARY.md) - Implémentation complète
- [Implémentation terminée](development/IMPLEMENTATION_COMPLETE.md) - Statut d’implémentation
- [Résumé d’implémentation](development/IMPLEMENTATION_SUMMARY.md) - Infrastructure de test
- [Résumé d’implémentation Patch SD](development/IMPLEMENTATION_SUMMARY_PATCH_SD.md) - Implémentation Patch SD
- [Utilitaires de production](development/PRODUCTION_UTILITIES.md) - Outils de production
- [Correctif de débordement RAM](development/RAM_OVERFLOW_FIX.md) - Résolution de débordement RAM
- [USB Host MIDI](development/README_USBH_MIDI.md) - Implémentation USB Host
- [Intégration USB Device](development/README_USB_DEVICE_INTEGRATION.md) - Configuration USB Device
- [Guide de portabilité](development/README_PORTABILITY.md) - Migration STM32F4/H7
- [Index de compatibilité](development/COMPATIBILITY_INDEX.md) - Index de compatibilité MIOS32
- [Résumé de compatibilité](development/COMPATIBILITY_SUMMARY.md) - Référence rapide
- [Compatibilité des drivers](development/DRIVER_COMPATIBILITY_REPORT.md) - Analyse des drivers
- [Compatibilité LoopA](development/LOOPA_COMPATIBILITY_REPORT.md) - Fonctionnalités LoopA
- [Correctif Production Mode](development/PRODUCTION_MODE_FIX.md) - Correctifs critiques
- [Résumé de solution](development/SOLUTION_SUMMARY.md) - Correctif d’erreur Windows
- [Implémentation finale](development/README_IMPLEMENTATION_FINAL.md) - Config USB MIDI finale
- [Détails des modules](development/Modules_MidiCore_Detail_par_Module.txt) - Architecture détaillée (français)

### 🔌 [Documentation USB](usb/) • [🇫🇷 Français](usb/README_FR.md)
Implémentation USB MIDI et débogage
- [Vue d’ensemble](usb/README.md) - Aperçu de la documentation USB
- [Guide de configuration USB](usb/USB_CONFIGURATION_GUIDE.md) - Configuration USB
- [Guide USB Device et Host](usb/USB_DEVICE_AND_HOST_GUIDE.md) - Guide complet
- [USB Host expliqué](usb/USB_HOST_AND_DEVICE_EXPLAINED.md) - Explication de l’architecture
- [Guide de debug USB](usb/USB_DEBUG_GUIDE.md) - Débogage des problèmes USB
- [USB Debug UART Quickstart](usb/USB_DEBUG_UART_QUICKSTART.md) - Mise en route du debug UART
- [Problème OTG CubeMX](usb/CUBEMX_OTG_ISSUE_EXPLAINED.md) - Résolution du problème OTG
- [Protéger USB Host](usb/PROTECT_USB_HOST_FROM_CUBEMX.md) - Protection contre régénération
- [Bugs de la bibliothèque USB STM32](usb/STM32_USB_LIBRARY_BUGS.md) - Problèmes connus
- [Protection USB MIDI CubeMX](usb/USB_MIDI_CUBEMX_PROTECTION.md) - Guide de protection
- [USB MIDI Jacks Explained](usb/USB_MIDI_JACKS_EXPLAINED.md) - Configuration des jacks (EN)
- [USB MIDI Jacks Explications](usb/USB_MIDI_JACKS_EXPLICATIONS_FR.md) - Configuration des jacks (FR)

**Correctifs :**
- [Correctif Bulk Endpoint](usb/USB_BULK_ENDPOINT_BUG_FIX.md)
- [Correctif taille des descripteurs](usb/USB_DESCRIPTOR_SIZE_BUG_FIX.md)
- [Échec du descripteur de périphérique](usb/USB_DEVICE_DESCRIPTOR_FAILURE_ANALYSIS.md)
- [Correctif IAD](usb/USB_IAD_FIX.md)
- [Correctif de suppression IAD](usb/USB_IAD_REMOVAL_FIX.md)
- [Correctif MS Header](usb/USB_MIDI_MS_HEADER_BUG_FIX.md)

**Documents techniques :**
- [Structure des descripteurs USB](usb/USB_DESCRIPTOR_STRUCTURE.txt) - Détails de la structure
- [Analyse des descripteurs USB MIDI](usb/USB_MIDI_DESCRIPTOR_ANALYSIS.md) - Analyse des descripteurs
- [Audit du protocole USB MIDI](usb/USB_MIDI_PROTOCOL_AUDIT.md) - Audit du protocole
- [Moteur SysEx USB MIDI](usb/USB_MIDI_SYSEX_ENGINE.md) - Implémentation SysEx

### 🔄 [Compatibilité MIOS32](mios32/) • [🇫🇷 Français](mios32/README_FR.md)
Compatibilité MIOS32 et guides de migration
- [Vue d’ensemble](mios32/README.md) - Aperçu de la compatibilité MIOS32
- [Compatibilité MIOS32](mios32/MIOS32_COMPATIBILITY.md) - Compatibilité globale
- [Comparaison approfondie MIOS32](mios32/MIOS32_DEEP_COMPARISON.md) - Analyse détaillée
- [Guide double mode MIOS32](mios32/MIOS32_DUAL_MODE_GUIDE.md) - USB double mode
- [Guide d’implémentation USB MIOS32](mios32/MIOS32_USB_IMPLEMENTATION_GUIDE.md) - Guide USB
- [Auto-basculement style MIOS32](mios32/MIOS32_STYLE_AUTO_SWITCHING.md) - Basculement de mode
- [Analyse des descripteurs MIOS32](mios32/MIOS32_DESCRIPTOR_ANALYSIS.md) - Détails des descripteurs

### 💼 [Commercial](commercial/) • [🇫🇷 Français](commercial/README_COMMERCIAL_FR.md)
Documentation et présentations commerciales
- [README Commercial](commercial/README_COMMERCIAL.md) - Vue d’ensemble professionnelle (EN)
- [README Commercial FR](commercial/README_COMMERCIAL_FR.md) - Vue d’ensemble professionnelle (FR)
- [Product Presentation](commercial/PRESENTATION_PRODUIT_EN.md) - Présentation produit (EN)
- [Présentation produit](commercial/PRESENTATION_PRODUIT.md) - Présentation produit (FR)

---

## Liens rapides

- **Nouveau sur MidiCore ?** → Commencer par [Bien démarrer](getting-started/)
- **Besoin de configurer ?** → Voir [Configuration](configuration/)
- **Envie de tester ?** → Consulter [Démarrage rapide des tests](testing/TESTING_QUICKSTART.md)
- **Problèmes USB ?** → Parcourir la [Documentation USB](usb/)
- **Vous venez de MIOS32 ?** → Lire [Compatibilité MIOS32](mios32/)

## Ressources supplémentaires

- **README principal** : [../README.md](../README.md)
- **Détails des modules** : [development/Modules_MidiCore_Detail_par_Module.txt](development/Modules_MidiCore_Detail_par_Module.txt)
- **Code source** : [../App/](../App/), [../Services/](../Services/), [../Hal/](../Hal/)

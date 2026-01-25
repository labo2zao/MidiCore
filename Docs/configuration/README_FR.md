# Configuration

> 🇬🇧 [English version available](README.md)

Guides de configuration et de mise en place du système MidiCore.

## Guides Disponibles

### Configuration des Modules
- **[Configuration des Modules](README_MODULE_CONFIG.md)** - Activer/désactiver les modules et configurer les fonctionnalités

### Configuration UART & Série
- **[Configuration UART MIOS32](README_MIOS32_UART_CONFIG.md)** - Configuration UART et debug pour la compatibilité MIOS32

### Configuration Matérielle
- **[Référence Configuration SPI](SPI_CONFIGURATION_REFERENCE.md)** - Paramètres et configuration SPI
- **[Guide de Régénération CubeMX](CUBEMX_REGENERATION_GUIDE.md)** - Protéger le code personnalisé de la régénération CubeMX

### Protection du Système
- **[Guide de Protection FreeRTOS](FREERTOS_PROTECTION_GUIDE.md)** - Protéger les tâches FreeRTOS de la régénération CubeMX

## Aperçu de la Configuration

MidiCore utilise un système de configuration modulaire qui permet de:
- Activer/désactiver les modules matériels (AINSER64, SRIO, OLED, etc.)
- Configurer les interfaces de communication (UART, SPI, I2C)
- Protéger le code personnalisé de la régénération CubeMX
- Configurer le mappage UART compatible MIOS32

## Démarrage Rapide

1. **Configuration des Modules**: Éditer `Config/module_config.h` pour activer les modules désirés
2. **Configuration des Broches**: Configurer les broches dans CubeMX selon les exigences des modules
3. **Protection de Régénération**: Suivre les guides pour protéger le code personnalisé avant de régénérer avec CubeMX

## Documentation Associée

- **[Démarrage](../getting-started/)** - Démarrage rapide et intégration
- **[Matériel](../hardware/)** - Configuration et câblage matériel
- **[Développement](../development/)** - Détails d'implémentation technique

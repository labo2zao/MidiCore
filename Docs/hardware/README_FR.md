# Matériel

> 🇬🇧 [English version available](README.md)

Configuration matérielle, guides de câblage et informations de brochage pour MidiCore.

## Guides Disponibles

### Écran OLED
- **[Guide de Câblage OLED](OLED_WIRING_GUIDE.md)** - Câblage de l'écran pour afficheurs SSD1322/SSD1306 compatibles LoopA
- **[Test Rapide OLED](OLED_QUICK_TEST.md)** - Procédure de test rapide OLED
- **[Guide de Page de Test OLED](OLED_TEST_PAGE_GUIDE.md)** - Guide détaillé des pages de test
- **[Dépannage OLED](OLED_TROUBLESHOOTING.md)** - Problèmes courants et solutions
- **[Référence Technique OLED SSD1322](OLED_SSD1322_TECHNICAL_REFERENCE.md)** - Détails techniques
- **[Historique des Corrections OLED SSD1322](OLED_SSD1322_FIX_HISTORY.md)** - Corrections de bugs et améliorations
- **[Résumé des Améliorations OLED](OLED_IMPROVEMENTS_SUMMARY.md)** - Résumé des améliorations de l'affichage

### Connecteurs & Brochages
- **[Brochage Connecteur OLED J1](J1_OLED_CONNECTOR_PINOUT.md)** - Détails du brochage du connecteur OLED

### Interface Utilisateur
- **[Résumé des Améliorations UI](UI_IMPROVEMENTS_SUMMARY.md)** - Améliorations et enrichissements de l'UI
- **[Améliorations du Rendu UI](UI_RENDERING_IMPROVEMENTS.md)** - Optimisations du rendu
- **[Ergonomie de la Disposition UI](UI_LAYOUT_ERGONOMIC.md)** - Conception ergonomique de la disposition
- **[Navigation par Touches Combinées](COMBINED_KEY_NAVIGATION.md)** - Système de navigation par touches

### Configuration
- **[Guide de Régénération CubeMX](CUBEMX_REGENERATION_GUIDE.md)** - Protéger la configuration matérielle

### Fiches Techniques
- **[NHD-OLEDSSD1322DISP.pdf](NHD-OLEDSSD1322DISP.pdf)** - Fiche technique de l'écran OLED
- **[SSD1322.pdf](SSD1322%20(4).pdf)** - Fiche technique du contrôleur SSD1322

## Aperçu du Matériel

MidiCore supporte divers modules matériels:
- **Écrans OLED**: SSD1322 (256×64 niveaux de gris), SSD1306 (128×64 monochrome)
- **Entrée Analogique**: AINSER64 (64 canaux via MCP3208 + multiplexeur)
- **E/S Numériques**: SRIO (registres à décalage pour boutons/LEDs)
- **MIDI**: E/S DIN 4 ports, USB MIDI Device/Host
- **Stockage**: Carte SD via SDIO ou SPI

## Démarrage Rapide

1. **Câbler l'Écran**: Suivre le [Guide de Câblage OLED](OLED_WIRING_GUIDE.md)
2. **Tester l'Écran**: Utiliser le [Test Rapide OLED](OLED_QUICK_TEST.md)
3. **Configurer**: Configurer les broches dans CubeMX
4. **Dépanner**: Consulter le [Dépannage OLED](OLED_TROUBLESHOOTING.md) si nécessaire

## Documentation Associée

- **[Configuration](../configuration/)** - Configuration du système
- **[Démarrage](../getting-started/)** - Guides d'intégration
- **[Tests](../testing/)** - Procédures de test matériel

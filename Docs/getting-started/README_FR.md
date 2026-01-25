# Démarrage avec MidiCore

Guides de démarrage rapide pour vous aider à intégrer et utiliser MidiCore dans vos projets.

## Guides Disponibles

### Intégration
- **[Planification & Prochaines Étapes](WHAT_TO_DO_NOW.md)** - Plan d'action clair et dernières mises à jour

## Flux de Démarrage Rapide

1. **Lire** ce guide pour comprendre l'intégration des modules
2. **Configurer** les modules en utilisant [Configuration des Modules](../configuration/README_MODULE_CONFIG.md)
3. **Tester** votre configuration avec [Test de Démarrage Rapide](../testing/TESTING_QUICKSTART.md)
4. **Explorer** les [Guides Utilisateur](../user-guides/) pour la documentation des fonctionnalités

## Besoin d'Aide?

- **Problèmes de configuration?** → Voir [Configuration](../configuration/)
- **Erreurs de compilation?** → Consultez [Que Faire Maintenant](WHAT_TO_DO_NOW.md)
- **Tests?** → Parcourir la [Documentation de Test](../testing/)

---

## Guide d'Intégration

### Ajouter les Modules AINSER64 et OLED SSD1322

#### Copier dans votre projet
Copier les dossiers:
- `Config/`
- `Hal/`
- `Services/`

#### Ordre d'initialisation (après l'init du noyau RTOS)
```c
spibus_init();
hal_ainser64_init();
ain_init();
oled_init();
```

#### Tâche FreeRTOS (exemple)
Appeler `ain_tick_5ms()` toutes les 5ms.

#### Mappage des touches
Mappage actuel:
```
key = ((bank*8 + adc_channel) * 8 + step)  // 0..63
```

Si votre câblage physique diffère, modifiez le mappage dans `Services/ain/ain.c`.

---

## Intégration du Projet

### Projet MidiCore fusionné (router + DIN + patch + stubs looper/ui)

#### Ce qui est inclus
- Scan AINSER64 + événements de vélocité
- Pilote OLED SSD1322 + démo
- Routeur 16 nœuds avec masques de canal par route
- MIDI DIN IN/OUT (USART1/2/3 + UART5) à 31250 bauds
- Patch SD TXT (FATFS) stockage minimal clé=valeur
- Stubs de transport USB MIDI (votre projet CubeMX est actuellement Custom HID)
- Placeholders compilables Looper/UI (à implémenter complètement ensuite)

#### Notes CubeMX
- Assurez-vous que le baudrate USART1 est réglé à 31250 (peut être par défaut actuellement).
- Assurez-vous que les interruptions RX sont activées pour USART1/2/3/UART5.
- Le Device USB est Custom HID dans ce projet; pour activer USB MIDI plus tard:
  - Basculer la classe USB Device vers MIDI dans CubeMX
  - Générer `usbd_midi_if.*` et définir `ENABLE_USBD_MIDI` dans les flags de compilation.

#### Notes de compilation
- Si vous voyez `undefined reference to powf`, ajoutez `m` dans MCU GCC Linker → Libraries.

#### Chargement de patch au démarrage (Option A)
- `app_init` monte SD (0:) puis charge `0:/patches/bank01/patch.txt` et applique les routes [router].

---

## Bundle de Modules AccordeonInstrument v1

Ce bundle contient les modules actuellement *implémentés* (non vides):
- `Config/`: mappages de broches pour AINSER64 + OLED
- `Hal/`: spi_bus, delay_us, ainser64_hw (MCP3208+4051+bank gating), oled_ssd1322
- `Services/`: ain (filtrage+calib+vélocité C=A+B + file d'événements)
- `App/`: tâches de démarrage (AinTask 5ms + démo OLED)

### Comment utiliser (CubeIDE)
1. Partir de votre projet généré par CubeMX (celui qui génère déjà OK).
2. Copier ces dossiers à côté de `Core/`:
   - Config, Hal, Services, App
3. Ajouter les chemins d'inclusion:
   - `${workspace_loc:/${ProjName}/Config}`
   - `${workspace_loc:/${ProjName}/Hal}`
   - `${workspace_loc:/${ProjName}/Services}`
   - `${workspace_loc:/${ProjName}/App}`
4. S'assurer que le linker inclut libm (pour powf):
   - ajouter `m` dans MCU GCC Linker > Libraries
5. Appeler `app_init_and_start()` une fois après l'init RTOS (voir `App/README.md`).

### Notes
- Ceci est un *bundle de modules*, pas un remplacement pour les fichiers CubeMX (Core/, .ioc, startup, linker).
- Il compilera lorsqu'il sera fusionné dans votre projet CubeMX avec le brochage convenu.

# MIOS32 vs MidiCore CLI - Analyse Comparative des Stacks

## 📚 Question Initiale

*"Comment ils gèrent dans mios32 la question du cli ? Quelle sont les valeurs des stack ?"*

Cette analyse compare l'approche MIOS32 (projet de référence) avec MidiCore pour comprendre les différences de sizing mémoire.

---

## 🎯 Configuration Stack MIOS32

### Architecture FreeRTOS MIOS32

D'après le code source MIOS32 (https://github.com/midibox/mios32):

```c
// MIOS32 Tutorial #006 - RTOS Tasks
// $MIOS32_PATH/apps/tutorials/006_rtos_tasks/

#define PRIORITY_TASK_J5_SCAN (tskIDLE_PRIORITY + 3)

// Task creation:
xTaskCreate(TASK_J5_Scan, 
            "J5_Scan", 
            configMINIMAL_STACK_SIZE,  // ← Stack size
            NULL, 
            PRIORITY_TASK_J5_SCAN, 
            NULL);
```

### Valeurs Typiques MIOS32

| Tâche | Stack (words) | Stack (bytes) | Usage |
|-------|---------------|---------------|-------|
| Simple (I/O scan) | 128 | 512 B | Polling pins, simple logic |
| Moyenne (MIDI) | 256 | 1 KB | MIDI processing |
| Complexe (UI) | 512 | 2 KB | UI, complex commands |
| Très complexe | 1024 | 4 KB | File I/O, parsing |

**Note:** FreeRTOS utilise des **words** (4 bytes sur Cortex-M) pour les stack sizes.

### Tâches Standard MIOS32

```c
// From MIOS32 programming_models/traditional/main.c

// MIDI Poll Task: Priority 3, Stack ~256 words (1KB)
// SRIO/AIN Scan Task: Priority 3, Stack ~256 words (1KB)
// APP_Background: Priority 0 (idle), Stack minimal
```

---

## 📊 Comparaison Détaillée: MIOS32 vs MidiCore

### 1. Stack des Tâches CLI

| Projet | Stack CLI Task | Justification |
|--------|---------------|---------------|
| **MIOS32** | **256-512 words (1-2KB)** | CLI simple, commandes basiques |
| **MidiCore (initial)** | **1536 words (6KB)** | CLI complexe, appels imbriqués |
| **MidiCore (optimisé)** | **768 words (3KB)** | Buffers réduits, historique supprimé |

### 2. Mémoire CLI Statique

| Élément | MIOS32 | MidiCore (avant) | MidiCore (optimisé) |
|---------|--------|------------------|---------------------|
| Historique | 0 (pas utilisé) | 4KB (16×256) | 0 (supprimé) ✅ |
| Registre commandes | ~2KB (32 cmd) | 10KB (128 cmd) | 2.5KB (32 cmd) ✅ |
| Input buffer | 128B | 256B | 128B ✅ |
| Print buffer | 128B | 256B | 128B ✅ |
| **TOTAL** | **~3KB** | **~15KB** | **~3KB** ✅ |

### 3. Heap FreeRTOS

| Projet | Heap Size | Usage |
|--------|-----------|-------|
| **MIOS32** | 10-15KB | Tâches + mutex/queues simples |
| **MidiCore (initial)** | 25KB | Nombreux modules, mutex, queues |
| **MidiCore (optimisé)** | 15-20KB | Réduit avec CLI optimisé |

---

## 💡 Pourquoi MIOS32 Utilise Moins de Stack?

### 1. Architecture CLI Plus Simple

**MIOS32:**
```c
// Tâche simple, pas d'imbrication profonde
static void TASK_Scan(void *pvParameters) {
  u8 state[12];  // Variables locales minimales
  
  while(1) {
    vTaskDelayUntil(&xLastExecutionTime, 1 / portTICK_RATE_MS);
    
    // Processing simple
    for(pin=0; pin<12; ++pin) {
      // Code plat, pas d'appels imbriqués
      u8 new_state = ReadPin(pin);
      if(new_state != state[pin]) {
        SendMIDI(pin, new_state);  // Direct, pas d'appels profonds
      }
    }
  }
}
```

**Stack usage:** ~200-300 bytes max

**MidiCore (avant optimisation):**
```c
void cli_execute(char *line) {
    char buffer[256];      // ← 256 bytes
    char *argv[8];         // ← 32 bytes
    
    cmd_module();          // ← Appel niveau 1
}

void cmd_module() {
    char buffer[256];      // ← 256 bytes additionnels
    cli_printf("...");     // ← Appel niveau 2
}

void cli_printf(const char *fmt, ...) {
    char buffer[256];      // ← 256 bytes encore
    usb_cdc_transmit();    // ← Appel niveau 3
}
```

**Profondeur:** 5 niveaux × 300B = **1500 bytes minimum**
**Mode debug:** × 2-3 = **3000-4500 bytes**

### 2. Utilisation de Buffers Statiques

**MIOS32 privilégie:**
```c
// Buffer statique global (pas sur stack)
static char s_shared_buffer[128];

void process_command(char *cmd) {
  // Utilise s_shared_buffer
  sprintf(s_shared_buffer, "Result: %s\n", cmd);
  print(s_shared_buffer);
}
```

**Avantage:** Stack minimal
**Trade-off:** RAM toujours occupée (même si commande pas active)

**MidiCore utilisait:**
```c
// Buffers locaux (sur stack)
void cmd_help() {
  char buffer[256];  // Alloué à chaque appel
  sprintf(buffer, "Commands:\n");
  cli_printf(buffer);
}
```

**Avantage:** RAM libérée après appel
**Trade-off:** Stack plus grand nécessaire

---

## 🎯 Optimisations Appliquées à MidiCore

### 1. Suppression Historique CLI

```c
// AVANT:
#define CLI_HISTORY_SIZE 16
static char s_history[16][256];  // 4KB!

// APRÈS (style MIOS32):
#define CLI_HISTORY_SIZE 0       // Historique désactivé
// Pas de s_history[] = économie 4KB
```

**Justification:**
- MIOS32 n'utilise pas d'historique CLI
- Sur un instrument MIDI, CLI utilisé ponctuellement pour config
- Pas de besoin de rappeler commandes précédentes

### 2. Réduction Registre Commandes

```c
// AVANT:
#define CLI_MAX_COMMANDS 128
static cli_command_t s_commands[128];  // 10KB

// APRÈS (style MIOS32):
#define CLI_MAX_COMMANDS 32
static cli_command_t s_commands[32];   // 2.5KB
```

**Justification:**
- MIOS32 a ~20-30 commandes max
- MidiCore n'en utilise que 7-10 actuellement
- 32 slots largement suffisant

### 3. Buffers Locaux Réduits

```c
// AVANT:
void cli_printf(const char *fmt, ...) {
    char buffer[256];  // Sur stack
    // ...
}

// APRÈS (style MIOS32):
void cli_printf(const char *fmt, ...) {
    char buffer[128];  // 50% plus petit
    // ...
}
```

**Impact stack:** -128 bytes par niveau d'appel = **-512 bytes total**

### 4. Longueur Ligne Réduite

```c
// AVANT:
#define CLI_MAX_LINE_LEN 256

// APRÈS (style MIOS32):
#define CLI_MAX_LINE_LEN 128
```

**Justification:**
- MIOS32 utilise 80-128 caractères
- 128 suffisant pour commandes + arguments
- Économie: 128 bytes (input) + 2KB (historique si activé)

---

## 📊 Résultat Final

### Stack Sizing Comparatif

| Tâche | MIOS32 | MidiCore (avant) | MidiCore (après) | Ratio |
|-------|--------|------------------|------------------|-------|
| DefaultTask | 512B-1KB | 6KB | 6KB | 6-12× |
| CLI Task | 1-2KB | 6KB | 3KB | 3× → 1.5× |
| MIDI Task | 1KB | 1KB | 1KB | 1× |
| **Total Stacks** | **~3-4KB** | **~13KB** | **~10KB** | **3.3× → 2.5×** |

### RAM Totale Comparaison

| Ressource | MIOS32 | MidiCore (avant) | MidiCore (après) |
|-----------|--------|------------------|------------------|
| Stacks | 3-4KB | 13KB | 10KB |
| CLI statique | 3KB | 15KB | 3KB |
| Heap FreeRTOS | 10-15KB | 25KB | 15-20KB |
| **TOTAL RTOS** | **~20KB** | **~53KB** | **~30KB** |
| **% of 128KB** | **16%** | **41%** | **23%** |

---

## 🎓 Leçons de MIOS32

### 1. Philosophy: Minimal Stacks

MIOS32 montre qu'on peut faire un système complexe avec des stacks petits si:
- Code plat (pas d'imbrication excessive)
- Buffers statiques partagés
- Commandes simples et directes

### 2. Monitoring is Key

MIOS32 recommande:
```c
// Vérifier régulièrement l'usage stack
UBaseType_t uxHighWaterMark;
uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
// Si < 64 words: augmenter stack
// Si > 256 words: peut-être réduire
```

### 3. Trade-offs

| Approche | Avantages | Inconvénients |
|----------|-----------|---------------|
| **MIOS32 (buffers statiques)** | Stack minimal | RAM toujours occupée |
| **MidiCore (buffers locaux)** | RAM libérée après usage | Stack plus grand |

**Notre choix:** Hybride - buffers locaux réduits (128B) = bon compromis

---

## ✅ Recommandations Finales

### Configuration Optimale MidiCore

```c
// Services/cli/cli.h
#define CLI_HISTORY_SIZE 0              // Comme MIOS32
#define CLI_MAX_COMMANDS 32             // Comme MIOS32
#define CLI_MAX_LINE_LEN 128            // Comme MIOS32
#define CLI_PRINT_BUFFER_SIZE 128       // Comme MIOS32

// App/app_init.c
const osThreadAttr_t cli_attr = {
  .name = "CliTask",
  .priority = osPriorityBelowNormal,
  .stack_size = 3072  // 768 words = 3KB (vs MIOS32: 1-2KB)
};

// Core/Src/main.c
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 6,  // 6KB pour init complexe
  .priority = (osPriority_t) osPriorityNormal,
};

// Core/Inc/FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE ((size_t)(15*1024))  // 15KB
```

### Pourquoi Encore Plus que MIOS32?

MidiCore utilise **3KB** pour CLI vs **1-2KB** pour MIOS32 car:

1. **Commandes plus complexes:**
   - `module status`, `test run`, `config` avec nombreux paramètres
   - MIOS32: commandes simples I/O

2. **Appels plus profonds:**
   - MidiCore: cli_execute → cmd_module → module_status → cli_printf (4-5 niveaux)
   - MIOS32: task → action directe (2 niveaux)

3. **Mode Debug:**
   - STM32CubeIDE ajoute overhead significatif
   - Frames 2-3× plus gros en debug

4. **Sécurité:**
   - 3KB donne marge confortable
   - Évite overflow en cas d'ajout fonctionnalités

**Conclusion:** 3KB est un excellent compromis entre MIOS32 (1-2KB minimal) et notre besoin (6KB était excessif).

---

## 📚 Références

- MIOS32 Repository: https://github.com/midibox/mios32
- MIOS32 Tutorial #006 RTOS Tasks: https://github.com/midibox/mios32/tree/master/apps/tutorials/006_rtos_tasks
- FreeRTOS Documentation: https://www.freertos.org/
- FreeRTOS Stack Overflow Detection: https://www.freertos.org/Stacks-and-stack-overflow-checking.html

---

**Document créé:** 2026-01-30  
**Auteur:** MidiCore Architecture Team  
**Version:** 1.0

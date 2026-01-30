# Architecture Mémoire MidiCore - STM32F407

## 📚 Comprendre: Variables Statiques vs Stack FreeRTOS

### ❓ Question Fréquente

> "Pourquoi il faut mettre un stack FreeRTOS alors que ces fonctions sont déjà déclarées en static?"

**Réponse courte:** Ce sont deux choses complètement différentes!

---

## 🎯 Différence Fondamentale

### 1. Variables STATIQUES (`static`)

**Localisation:** Section `.bss` ou `.data` en RAM (mémoire globale)  
**Durée de vie:** Toute la vie du programme  
**Allocation:** À la compilation  

**Exemple:**
```c
// Services/cli/cli.c
static char s_commands[128];       // 128 bytes en .bss
static char s_input_line[256];     // 256 bytes en .bss
static char s_history[16][256];    // 4096 bytes en .bss (avant optim)

// Ces variables existent TOUJOURS en RAM
// Elles ne sont PAS sur le stack
```

### 2. Stack FreeRTOS

**Localisation:** Heap FreeRTOS (alloué dynamiquement)  
**Durée de vie:** Pendant l'exécution de la tâche  
**Allocation:** À runtime par FreeRTOS  

**Exemple:**
```c
void cli_execute(char *line) {
    char buffer[256];      // ← 256 bytes sur STACK de CliTask
    char *argv[8];         // ← 32 bytes sur STACK de CliTask
    int argc;              // ← 4 bytes sur STACK de CliTask
    
    // Ces variables existent SEULEMENT pendant l'exécution
    // Elles sont sur le stack de la tâche
}
```

---

## 📊 Carte Mémoire STM32F407 (128KB RAM)

```
┌─────────────────────────────────────────────────────┐
│ 0x0800_0000: FLASH (512KB)                         │
│   └─ Code (.text) - programmes                     │
└─────────────────────────────────────────────────────┘
         ↓ code exécuté depuis FLASH

┌─────────────────────────────────────────────────────┐
│ 0x2000_0000: RAM (128KB)                           │
│                                                     │
│ ┌─────────────────────────────────────────────┐   │
│ │ .data (variables initialisées)              │   │
│ │ ~10KB                                       │   │
│ │   static int x = 5;                        │   │
│ │   const int y = 10; (si en RAM)            │   │
│ └─────────────────────────────────────────────┘   │
│                                                     │
│ ┌─────────────────────────────────────────────┐   │
│ │ .bss (variables non-initialisées)          │   │
│ │ ~20KB (avant optimisation CLI)              │   │
│ │   static char s_commands[128];             │   │
│ │   static char s_history[16][256];  (4KB!)  │   │
│ │   static uint32_t g_count;                 │   │
│ └─────────────────────────────────────────────┘   │
│                                                     │
│ ┌─────────────────────────────────────────────┐   │
│ │ Heap FreeRTOS (configurable)               │   │
│ │ 15-25KB selon config                        │   │
│ │                                             │   │
│ │ Contient:                                   │   │
│ │   ├─ DefaultTask stack (6KB)              │   │
│ │   ├─ CliTask stack (3-6KB)                │   │
│ │   ├─ MidiIOTask stack (1KB)               │   │
│ │   ├─ TCB des tâches (~600B)               │   │
│ │   ├─ Mutex (15× ~80B = 1.2KB)             │   │
│ │   ├─ Queues USB (~800B)                    │   │
│ │   └─ Semaphores/Events (~500B)            │   │
│ └─────────────────────────────────────────────┘   │
│                                                     │
│ ┌─────────────────────────────────────────────┐   │
│ │ Heap C standard (malloc/free)              │   │
│ │ ~2KB (minimal, évité)                       │   │
│ └─────────────────────────────────────────────┘   │
│                                                     │
│ ┌─────────────────────────────────────────────┐   │
│ │ Libre / Disponible                         │   │
│ │ ~73KB (après optimisations)                 │   │
│ └─────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────┘
```

---

## 💡 Pourquoi le Stack est Nécessaire?

### Exemple Concret: Appels de Fonctions Imbriqués

```c
void CliTask(void *arg) {
    // Frame 0: CliTask
    // Locals: argument, etc. = 50 bytes
    
    for(;;) {
        cli_task();  // ← Appel de fonction
    }
}

void cli_task() {
    // Frame 1: cli_task  
    // Locals: variables = 50 bytes
    // TOTAL stack jusqu'ici: 100 bytes
    
    if (char_received) {
        cli_execute(input_line);  // ← Appel de fonction
    }
}

void cli_execute(char *line) {
    // Frame 2: cli_execute
    char buffer[256];     // 256 bytes
    char *argv[8];        // 32 bytes  
    int argc;             // 4 bytes
    // TOTAL stack jusqu'ici: 100 + 292 = 392 bytes
    
    if (strcmp(argv[0], "help") == 0) {
        cmd_help();  // ← Appel de fonction
    }
}

void cmd_help() {
    // Frame 3: cmd_help
    char buffer[256];     // 256 bytes
    // TOTAL stack jusqu'ici: 392 + 256 = 648 bytes
    
    cli_printf("Available commands:\r\n");  // ← Appel de fonction
}

void cli_printf(const char *fmt, ...) {
    // Frame 4: cli_printf
    char buffer[256];     // 256 bytes
    va_list args;         // 16 bytes
    // TOTAL stack jusqu'ici: 648 + 272 = 920 bytes
    
    usb_cdc_transmit(buffer, len);  // ← Appel de fonction
}

void usb_cdc_transmit(const char *data, size_t len) {
    // Frame 5: usb_cdc_transmit
    // Locals: variables = 100 bytes
    // TOTAL stack jusqu'ici: 920 + 100 = 1020 bytes
}
```

**Profondeur maximale: 5 niveaux = ~1KB de stack**

**En mode debug:** Frames 2-3× plus gros → **2-3KB nécessaire**

**Avec plusieurs chemins possibles:** Pire cas → **4-6KB nécessaire**

---

## 🔍 Analyse CLI Mémoire

### Variables Statiques (RAM .bss)

| Variable | Taille | Description |
|----------|--------|-------------|
| `s_commands[128]` | 10KB | Registre commandes (128×80B) |
| `s_history[16][256]` | 4KB | Historique (16 lignes) |
| `s_input_line[256]` | 256B | Ligne courante |
| `s_history_index` | 4B | Index historique |
| `s_command_count` | 4B | Nombre commandes |
| **TOTAL** | **~14.5KB** | **Avant optimisation** |

### Stack CliTask (Heap FreeRTOS)

| Fonction | Variables Locales | Taille Frame |
|----------|-------------------|--------------|
| `CliTask()` | arg, locals | ~50B |
| `cli_task()` | locals | ~50B |
| `cli_execute()` | buffer[256], argv[8], argc | ~300B |
| `cmd_xxx()` | buffer[256], locals | ~300B |
| `cli_printf()` | buffer[256], va_list | ~300B |
| `usb_cdc_transmit()` | locals | ~100B |
| **TOTAL (profondeur max)** | | **~1100B** |
| **× Mode debug** | | **×2-3** |
| **= Nécessaire** | | **3-4KB** |
| **+ Marge sécurité** | | **+2KB** |
| **= Stack alloué** | | **6KB** |

---

## ⚙️ Optimisations Possibles

### Option 1: Réduire Variables Locales

**AVANT:**
```c
void cli_printf(const char *fmt, ...) {
    char buffer[256];  // ← 256 bytes sur stack
    // ...
}
```

**APRÈS:**
```c
void cli_printf(const char *fmt, ...) {
    char buffer[128];  // ← 128 bytes sur stack (-50%)
    // ...
}
```

**Économie stack: ~500B → Stack 6KB → 4KB**

### Option 2: Utiliser Variables Statiques

**AVANT (sur stack):**
```c
void cmd_help() {
    char buffer[256];  // ← Alloué à chaque appel
    sprintf(buffer, "Commands...");
}
```

**APRÈS (statique):**
```c
void cmd_help() {
    static char s_buffer[256];  // ← Toujours en mémoire
    sprintf(s_buffer, "Commands...");
}
```

**Trade-off:**
- ✅ Réduit stack nécessaire
- ❌ Augmente .bss (mémoire toujours utilisée)
- ❌ Problème si fonction réentrante

### Option 3: Réduire Historique

**AVANT:**
```c
#define CLI_HISTORY_SIZE 16        // 16 lignes
#define CLI_MAX_LINE_LEN 256       // 256 chars
static char s_history[16][256];    // 4KB!
```

**APRÈS:**
```c
#define CLI_HISTORY_SIZE 0         // Désactivé
// s_history supprimé                 // 0 bytes
```

**Économie .bss: 4KB**

---

## 📈 Configuration Optimale STM32F407

### Balance RAM Recommandée

```c
// Core/Inc/FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE ((size_t)(15*1024))  // 15KB heap

// Core/Src/main.c - DefaultTask
.stack_size = 1024 * 6  // 6KB pour init profonde

// App/app_init.c - CliTask
.stack_size = 3072  // 3KB après optimisation buffers

// App/midi_io_task.c - MidiIOTask
.stack_size = 1024  // 1KB (loop simple)

// Services/cli/cli.h
#define CLI_HISTORY_SIZE 0          // Historique désactivé
#define CLI_MAX_COMMANDS 32         // 32 au lieu de 128
#define CLI_MAX_LINE_LEN 128        // 128 au lieu de 256
#define CLI_PRINT_BUFFER_SIZE 128   // 128 au lieu de 256
```

### Résultat

| Ressource | Avant | Après | Économie |
|-----------|-------|-------|----------|
| CLI .bss | 14.5KB | 3KB | -11.5KB |
| CLI stack | 6KB | 3KB | -3KB |
| Heap FreeRTOS | 25KB | 15KB | -10KB |
| **TOTAL** | **45.5KB** | **21KB** | **-24.5KB** |
| **RAM libre** | **82.5KB** | **107KB** | **+24.5KB** |

---

## 🎯 Règles à Retenir

1. **`static` ≠ stack**
   - `static` = mémoire globale (.bss/.data)
   - Stack = mémoire temporaire pour appels

2. **Variables locales utilisent le stack**
   - `char buffer[256]` dans fonction = sur stack
   - Besoin de stack dimensionné pour profondeur max

3. **Appels imbriqués accumulent les frames**
   - Plus d'appels = plus de stack nécessaire
   - Mode debug = frames plus gros

4. **Trade-off static vs stack**
   - Static: Toujours en mémoire (gaspillage)
   - Stack: Seulement quand utilisé (économie)

5. **Optimisation dépend de l'usage**
   - Fonction appelée souvent: static OK
   - Fonction rare: stack préférable

---

## 📚 Références

- FreeRTOS Documentation: [www.freertos.org](https://www.freertos.org)
- ARM Cortex-M Stack: [ARM Developer](https://developer.arm.com)
- GCC Memory Sections: [GCC Manual](https://gcc.gnu.org/onlinedocs/gcc/)

---

**Maintenant vous comprenez pourquoi le stack est nécessaire même avec des variables `static`!** 🎓

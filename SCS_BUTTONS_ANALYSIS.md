# MidiCore - Analyse et Extension des Boutons SCS

## Date
2026-01-12

## Contexte
**Demande**: Augmenter le nombre de boutons SCS pour optimiser la navigation dans l'UI du looper

---

## Configuration Actuelle

### Système Input Existant

**Fichier**: `Services/input/input.h`

```c
// Button logical IDs: 1..9 (utilisés par UI)
void input_feed_button(uint16_t phys_id, uint8_t pressed);
void input_feed_encoder(uint16_t phys_id, int8_t delta);
```

**Système flexible**:
- Support N boutons physiques (phys_id: 0..N-1)
- Mapping physique → logique configurable
- Debouncing: 20ms (configurable)
- Shift layer: bouton 10 (hold 500ms)

### UI Buttons Actuels

**Fichier**: `Services/ui/ui.c`

```c
void ui_on_button(uint8_t id, uint8_t pressed) {
  if (pressed && id == 5) {
    // Cycle pages: LOOP → TIME → PIANO → LOOP
  }
  // Dispatch to page handlers
}
```

**Bouton 5 seulement** utilisé pour cycle pages.

### UI Actions (Encodeurs)

**Fichier**: `Services/ui/ui_actions.h`

```c
typedef enum {
  UI_ACT_NONE = 0,
  // Navigation
  UI_ACT_PATCH_PREV,
  UI_ACT_PATCH_NEXT,
  UI_ACT_BANK_PREV,
  UI_ACT_BANK_NEXT,
  UI_ACT_LOAD_APPLY,
  // UI editing
  UI_ACT_UI_PREV_PAGE,
  UI_ACT_UI_NEXT_PAGE,
  UI_ACT_CURSOR_LEFT,
  UI_ACT_CURSOR_RIGHT,
  UI_ACT_ZOOM_OUT,
  UI_ACT_ZOOM_IN,
  UI_ACT_QUANTIZE,
  UI_ACT_DELETE,
  UI_ACT_TOGGLE_CHORD_MODE,
  UI_ACT_TOGGLE_AUTO_LOOP,
} ui_action_t;
```

**2 encodeurs** configurables avec mappings:
- CW/CCW
- Shift + CW/CCW
- Button press
- Shift + Button press

**Total**: 12 actions par encodeur (6 avec shift)

---

## Analyse MIDIbox NG SCS Standard

### Configuration SCS Classique

**Hardware Standard**:
- 1 Rotary Encoder (détented)
- 6 Boutons dédiés:
  1. **EXIT** - Retour/sortie menu
  2. **LEFT** - Navigation gauche
  3. **RIGHT** - Navigation droite
  4. **UP** - Navigation haut
  5. **DOWN** - Navigation bas
  6. **SELECT** - Confirmation/entrée

**Total**: 7 contrôles (1 encoder + 6 boutons)

### MIDIbox NG Extended SCS

Certaines configs utilisent:
- **SHIFT** button (dedicated)
- **Soft buttons** F1-F4 (context-sensitive)
- **Footswitches** (2-4 pédales)

**Total étendu**: 10-14 contrôles

---

## Proposition MidiCore SCS Extended

### Configuration Optimale pour Looper

#### Groupe 1: Navigation SCS (6 boutons)
1. **EXIT/BACK** - Retour, annuler
2. **LEFT** - Curseur gauche, page précédente
3. **RIGHT** - Curseur droite, page suivante
4. **UP** - Paramètre +, zoom in, track précédent
5. **DOWN** - Paramètre -, zoom out, track suivant
6. **SELECT/ENTER** - Confirmer, play/pause

#### Groupe 2: Looper Quick Access (6 boutons)
7. **REC** - Record on/off
8. **PLAY** - Play/pause/stop
9. **MUTE** - Mute track actuel
10. **CLEAR** - Clear track/events
11. **UNDO** - Undo last action
12. **SHIFT** - Layer shift (hold)

#### Groupe 3: Transport (4 boutons optionnels)
13. **REWIND** - Début de boucle
14. **FORWARD** - Fin de boucle
15. **STEP_BACK** - Step backward (footswitch compatible)
16. **STEP_FWD** - Step forward (footswitch compatible)

#### Encodeurs (2 existants)
- **ENC1**: Navigation générale, curseur
- **ENC2**: Paramètres, valeurs

**Total**: 12-16 boutons + 2 encodeurs

---

## Mapping Recommandé

### Boutons Logiques (ID)

```c
// SCS Core (1-6)
#define UI_BTN_EXIT      1   // EXIT/BACK
#define UI_BTN_LEFT      2   // Navigation gauche
#define UI_BTN_RIGHT     3   // Navigation droite
#define UI_BTN_UP        4   // Navigation haut
#define UI_BTN_DOWN      5   // Navigation bas
#define UI_BTN_SELECT    6   // SELECT/ENTER

// Looper Quick Access (7-12)
#define UI_BTN_REC       7   // Record
#define UI_BTN_PLAY      8   // Play/Pause
#define UI_BTN_MUTE      9   // Mute track
#define UI_BTN_CLEAR    10   // Clear
#define UI_BTN_UNDO     11   // Undo
#define UI_BTN_SHIFT    12   // Shift layer

// Transport Extended (13-16) - Optionnel
#define UI_BTN_REWIND   13   // <<
#define UI_BTN_FORWARD  14   // >>
#define UI_BTN_STEP_BCK 15   // |< step back
#define UI_BTN_STEP_FWD 16   // >| step forward
```

### Actions par Page

#### Looper Overview Page

| Bouton | Action | Shift + Action |
|--------|--------|----------------|
| LEFT | Track précédent | Scene précédente |
| RIGHT | Track suivant | Scene suivante |
| UP | Volume + | Transpose + |
| DOWN | Volume - | Transpose - |
| SELECT | Solo track | Arm all |
| REC | Record toggle | Record all tracks |
| PLAY | Play/pause | Stop all |
| MUTE | Mute track | Mute all |
| CLEAR | Clear track | Clear all |
| UNDO | Undo | Redo |

#### Looper Timeline Page

| Bouton | Action | Shift + Action |
|--------|--------|----------------|
| LEFT | Cursor left | Page left |
| RIGHT | Cursor right | Page right |
| UP | Zoom in | Zoom in 2x |
| DOWN | Zoom out | Zoom out 2x |
| SELECT | Place marker | Delete marker |
| STEP_BCK | Step backward | Jump to start |
| STEP_FWD | Step forward | Jump to end |
| REC | Record mode | Overdub mode |

#### Looper Piano Roll Page

| Bouton | Action | Shift + Action |
|--------|--------|----------------|
| LEFT | Note left | Octave down |
| RIGHT | Note right | Octave up |
| UP | Velocity + | Velocity +10 |
| DOWN | Velocity - | Velocity -10 |
| SELECT | Insert note | Delete note |
| CLEAR | Clear selection | Clear all notes |

#### Song Mode Page (NEW)

| Bouton | Action | Shift + Action |
|--------|--------|----------------|
| LEFT | Scene précédente | - |
| RIGHT | Scene suivante | - |
| UP | Track précédent | - |
| DOWN | Track suivant | - |
| SELECT | Toggle clip | Edit clip |
| PLAY | Play chain | Loop scene |
| REC | Record scene | - |

#### Config Editor Page (NEW)

| Bouton | Action | Shift + Action |
|--------|--------|----------------|
| LEFT | Cursor left | - |
| RIGHT | Cursor right | - |
| UP | Previous param | Previous section |
| DOWN | Next param | Next section |
| SELECT | Edit value | - |
| EXIT | Cancel/back | Save & exit |

---

## Implémentation Technique

### Étape 1: Extension UI Actions

**Fichier**: `Services/ui/ui_actions.h`

```c
typedef enum {
  UI_ACT_NONE = 0,
  
  // Existants...
  
  // NEW: Looper specific
  UI_ACT_REC_TOGGLE,
  UI_ACT_PLAY_PAUSE,
  UI_ACT_STOP_ALL,
  UI_ACT_MUTE_TRACK,
  UI_ACT_MUTE_ALL,
  UI_ACT_CLEAR_TRACK,
  UI_ACT_CLEAR_ALL,
  UI_ACT_UNDO,
  UI_ACT_REDO,
  UI_ACT_SOLO_TRACK,
  UI_ACT_ARM_TRACK,
  UI_ACT_ARM_ALL,
  
  // NEW: Transport
  UI_ACT_JUMP_START,
  UI_ACT_JUMP_END,
  UI_ACT_STEP_BACKWARD,
  UI_ACT_STEP_FORWARD,
  
  // NEW: Page navigation
  UI_ACT_PREV_TRACK,
  UI_ACT_NEXT_TRACK,
  UI_ACT_PREV_SCENE,
  UI_ACT_NEXT_SCENE,
  
  UI_ACT_COUNT
} ui_action_t;
```

### Étape 2: Extension Button Handler

**Fichier**: `Services/ui/ui.c`

```c
void ui_on_button(uint8_t id, uint8_t pressed) {
  uint8_t shift = input_shift_active();
  
  // SCS navigation buttons (1-6)
  switch (id) {
    case UI_BTN_EXIT:
      if (pressed) ui_handle_exit(shift);
      break;
    case UI_BTN_LEFT:
      if (pressed) ui_handle_left(shift);
      break;
    case UI_BTN_RIGHT:
      if (pressed) ui_handle_right(shift);
      break;
    case UI_BTN_UP:
      if (pressed) ui_handle_up(shift);
      break;
    case UI_BTN_DOWN:
      if (pressed) ui_handle_down(shift);
      break;
    case UI_BTN_SELECT:
      if (pressed) ui_handle_select(shift);
      break;
      
    // Looper quick access (7-12)
    case UI_BTN_REC:
      if (pressed) ui_handle_rec(shift);
      break;
    case UI_BTN_PLAY:
      if (pressed) ui_handle_play(shift);
      break;
    case UI_BTN_MUTE:
      if (pressed) ui_handle_mute(shift);
      break;
    case UI_BTN_CLEAR:
      if (pressed) ui_handle_clear(shift);
      break;
    case UI_BTN_UNDO:
      if (pressed) ui_handle_undo(shift);
      break;
    case UI_BTN_SHIFT:
      // Handled by input module
      break;
      
    // Transport extended (13-16)
    case UI_BTN_REWIND:
      if (pressed) ui_handle_rewind(shift);
      break;
    case UI_BTN_FORWARD:
      if (pressed) ui_handle_forward(shift);
      break;
    case UI_BTN_STEP_BCK:
      if (pressed) looper_step_backward(ui_current_track());
      break;
    case UI_BTN_STEP_FWD:
      if (pressed) looper_step_forward(ui_current_track());
      break;
      
    default:
      // Dispatch to page handlers for custom buttons
      ui_page_dispatch_button(id, pressed, shift);
      break;
  }
}
```

### Étape 3: Configuration File

**Fichier**: `/cfg/ui_buttons.ngc` (nouveau)

```ini
# MidiCore UI Buttons Configuration
# Format: LOGICAL_ID = PHYSICAL_DIN_BIT

# SCS Core Navigation
UI_BTN_EXIT    = 0
UI_BTN_LEFT    = 1
UI_BTN_RIGHT   = 2
UI_BTN_UP      = 3
UI_BTN_DOWN    = 4
UI_BTN_SELECT  = 5

# Looper Quick Access
UI_BTN_REC     = 8
UI_BTN_PLAY    = 9
UI_BTN_MUTE    = 10
UI_BTN_CLEAR   = 11
UI_BTN_UNDO    = 12
UI_BTN_SHIFT   = 13

# Transport (Optional)
UI_BTN_REWIND  = 16
UI_BTN_FORWARD = 17
UI_BTN_STEP_BCK = 18
UI_BTN_STEP_FWD = 19

# Footswitches
FOOTSWITCH_1 = 18  # Map to STEP_BCK
FOOTSWITCH_2 = 19  # Map to STEP_FWD
```

---

## Avantages de cette Configuration

### 1. Navigation Optimale
- **SCS standard**: 6 boutons familiers (EXIT, 4 directions, SELECT)
- **Context-aware**: Actions changent selon page active
- **Shift layer**: Double capacité (12 actions SCS de base)

### 2. Workflow Looper
- **Quick access**: REC, PLAY, MUTE, CLEAR à portée immédiate
- **Transport**: UNDO, step backward/forward
- **No menu diving**: Actions principales en 1 bouton

### 3. Extensibilité
- **Base**: 6 boutons (SCS minimal)
- **Standard**: 12 boutons (SCS + looper)
- **Extended**: 16 boutons (SCS + looper + transport)
- **Footswitches**: 2-4 pédales (step playback, rec, play)

### 4. Compatibilité MIOS32
- Layout inspiré MIDIbox NG SCS
- Mapping physique flexible (DIN bits)
- Configuration via .ngc files

---

## Migration Path

### Phase 1: SCS Core (6 boutons)
- Implémenter navigation de base
- EXIT, LEFT, RIGHT, UP, DOWN, SELECT
- Context-sensitive per page

### Phase 2: Looper Quick (6 boutons)
- Ajouter REC, PLAY, MUTE, CLEAR, UNDO, SHIFT
- Actions looper immédiates
- Shift layer actif

### Phase 3: Transport (4 boutons optionnels)
- REWIND, FORWARD pour jump
- STEP_BCK, STEP_FWD pour step playback
- Footswitch mapping

### Phase 4: Configuration
- Fichier `/cfg/ui_buttons.ngc`
- Parser et loader
- Hot-reload possible

---

## Recommandation Finale

**Configuration Optimale**: **12 boutons (SCS Extended)**
- 6 boutons SCS standard (navigation universelle)
- 6 boutons looper (workflow optimisé)
- 2 encodeurs (paramètres, curseur)
- 2-4 footswitches optionnels (step playback, rec)

**Total contrôles**: 12-16 boutons + 2 encoders + 2-4 footswitches

Cette config offre:
- ✅ Navigation SCS légère et rapide
- ✅ Workflow looper optimisé (no menu diving)
- ✅ Extensible (transport optionnel)
- ✅ Compatible MIOS32/MBHP
- ✅ Configurable via .ngc files

---

**Prêt pour implémentation!** 🚀

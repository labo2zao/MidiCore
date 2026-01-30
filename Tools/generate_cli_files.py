#!/usr/bin/env python3
"""
Generate CLI integration files for all MidiCore modules.
This script creates boilerplate CLI files following the established patterns.
"""

import os

# Module definitions: (name, category, description, has_per_track, parameters)
MODULES = [
    # MIDI Converter
    ("midi_converter", "EFFECT", "Convert between MIDI message types", True, [
        ("enabled", "bool", "Enable converter"),
        ("mode", "enum", "Conversion mode", ["CC_TO_AT", "AT_TO_CC", "PB_TO_CC", "CC_TO_PB", "VEL_TO_CC", "CC_TO_CC", "NOTE_TO_CC", "CC_TO_NOTE"]),
        ("source_cc", "int", "Source CC number (0-127)", 0, 127),
        ("dest_cc", "int", "Destination CC number (0-127)", 0, 127),
        ("scale", "int", "Scale factor (0-200%)", 0, 200),
        ("offset", "int", "Offset value (-127 to 127)", -127, 127),
        ("invert", "bool", "Invert values"),
    ]),
    
    # Quantizer
    ("quantizer", "EFFECT", "Timing quantizer for MIDI notes", True, [
        ("enabled", "bool", "Enable quantization"),
        ("resolution", "enum", "Grid resolution", ["1_4", "1_8", "1_16", "1_32", "1_8T", "1_16T", "1_4_DOT", "1_8_DOT"]),
        ("strength", "int", "Quantization strength (0-100%)", 0, 100),
        ("lookahead", "int", "Lookahead window (ms)", 0, 500),
        ("swing", "int", "Swing amount (0-100%)", 0, 100),
    ]),
    
    # Harmonizer
    ("harmonizer", "EFFECT", "MIDI harmonizer - adds harmony notes", True, [
        ("enabled", "bool", "Enable harmonizer"),
        ("voice1_interval", "enum", "Voice 1 interval", ["UNISON", "THIRD_UP", "THIRD_DOWN", "FIFTH_UP", "FIFTH_DOWN", "OCTAVE_UP", "OCTAVE_DOWN"]),
        ("voice1_enabled", "bool", "Enable voice 1"),
        ("voice2_interval", "enum", "Voice 2 interval", ["UNISON", "THIRD_UP", "THIRD_DOWN", "FIFTH_UP", "FIFTH_DOWN", "OCTAVE_UP", "OCTAVE_DOWN"]),
        ("voice2_enabled", "bool", "Enable voice 2"),
    ]),
    
    # Velocity Compressor
    ("velocity_compressor", "EFFECT", "Velocity dynamics compression", True, [
        ("enabled", "bool", "Enable compressor"),
        ("threshold", "int", "Compression threshold (1-127)", 1, 127),
        ("ratio", "enum", "Compression ratio", ["1_1", "2_1", "3_1", "4_1", "8_1", "INF_1"]),
        ("makeup_gain", "int", "Output gain (0-127)", 0, 127),
        ("knee", "enum", "Knee type", ["HARD", "SOFT"]),
    ]),
    
    # CC Smoother
    ("cc_smoother", "EFFECT", "MIDI CC smoother - eliminate zipper noise", True, [
        ("enabled", "bool", "Enable smoothing"),
        ("mode", "enum", "Smoothing mode", ["OFF", "LIGHT", "MEDIUM", "HEAVY", "CUSTOM"]),
        ("amount", "int", "Smoothing amount (0-255)", 0, 255),
        ("attack", "int", "Attack time (ms)", 0, 1000),
        ("release", "int", "Release time (ms)", 0, 1000),
    ]),
    
    # Envelope CC
    ("envelope_cc", "EFFECT", "ADSR envelope to CC output", True, [
        ("enabled", "bool", "Enable envelope"),
        ("channel", "int", "Output channel (0-15)", 0, 15),
        ("cc_number", "int", "CC to modulate (0-127)", 0, 127),
        ("attack_ms", "int", "Attack time (0-5000ms)", 0, 5000),
        ("decay_ms", "int", "Decay time (0-5000ms)", 0, 5000),
        ("sustain", "int", "Sustain level (0-127)", 0, 127),
        ("release_ms", "int", "Release time (0-5000ms)", 0, 5000),
    ]),
    
    # LFO
    ("lfo", "EFFECT", "Low Frequency Oscillator for modulation", True, [
        ("enabled", "bool", "Enable LFO"),
        ("waveform", "enum", "Waveform", ["SINE", "TRIANGLE", "SQUARE", "SAW_UP", "SAW_DOWN", "RANDOM"]),
        ("rate_hz", "int", "LFO rate (0.01-10Hz * 100)", 1, 1000),
        ("depth", "int", "Modulation depth (0-127)", 0, 127),
        ("target", "enum", "Modulation target", ["CC", "PITCH", "VELOCITY", "TIMING"]),
    ]),
    
    # Channelizer
    ("channelizer", "EFFECT", "Intelligent channel mapping and voice management", True, [
        ("enabled", "bool", "Enable channelizer"),
        ("mode", "enum", "Operating mode", ["BYPASS", "FORCE", "REMAP", "ROTATE", "ZONE"]),
        ("force_channel", "int", "Force to channel (0-15)", 0, 15),
        ("voice_limit", "int", "Max voices (1-16)", 1, 16),
    ]),
    
    # Chord
    ("chord", "EFFECT", "Chord trigger - single note to chord", True, [
        ("enabled", "bool", "Enable chord trigger"),
        ("type", "enum", "Chord type", ["MAJOR", "MINOR", "DIM", "AUG", "MAJ7", "MIN7", "DOM7", "SUS2", "SUS4"]),
        ("inversion", "int", "Chord inversion (0-3)", 0, 3),
        ("voicing", "enum", "Voicing", ["CLOSE", "SPREAD", "DROP2", "DROP3"]),
    ]),
    
    # Scale
    ("scale", "EFFECT", "Scale quantization", False, [
        ("scale_type", "enum", "Scale type", ["CHROMATIC", "MAJOR", "MINOR_NAT", "MINOR_HAR", "MINOR_MEL", "DORIAN", "PHRYGIAN", "LYDIAN", "MIXOLYDIAN", "LOCRIAN", "PENTATONIC_MAJ", "PENTATONIC_MIN", "BLUES", "WHOLE_TONE"]),
        ("root_note", "int", "Root note (0-11, C=0)", 0, 11),
    ]),
    
    # Legato
    ("legato", "EFFECT", "Legato/mono/priority handling", True, [
        ("enabled", "bool", "Enable legato mode"),
        ("priority", "enum", "Note priority", ["LAST", "HIGHEST", "LOWEST", "FIRST"]),
        ("retrigger", "enum", "Retrigger mode", ["OFF", "ON"]),
        ("glide_time", "int", "Portamento time (0-2000ms)", 0, 2000),
        ("mono_mode", "bool", "Mono mode"),
    ]),
    
    # Note Repeat
    ("note_repeat", "EFFECT", "Note repeat/ratchet/stutter (MPC-style)", True, [
        ("enabled", "bool", "Enable repeat"),
        ("rate", "enum", "Repeat rate", ["1_4", "1_8", "1_16", "1_32", "1_8T", "1_16T", "1_32T"]),
        ("gate", "int", "Gate length (1-100%)", 1, 100),
        ("velocity_decay", "int", "Velocity decay (0-100%)", 0, 100),
    ]),
    
    # Note Stabilizer
    ("note_stabilizer", "EFFECT", "Stabilize note timing and velocity", True, [
        ("enabled", "bool", "Enable stabilizer"),
        ("min_duration_ms", "int", "Min note duration (10-500ms)", 10, 500),
        ("retrigger_delay_ms", "int", "Retrigger delay (10-1000ms)", 10, 1000),
        ("neighbor_range", "int", "Neighbor semitones (0-12)", 0, 12),
    ]),
    
    # Strum
    ("strum", "EFFECT", "Guitar-style strum effect", True, [
        ("enabled", "bool", "Enable strum"),
        ("time", "int", "Strum time (0-200ms)", 0, 200),
        ("direction", "enum", "Direction", ["UP", "DOWN", "UP_DOWN", "RANDOM"]),
        ("velocity_ramp", "bool", "Velocity ramp"),
    ]),
    
    # Swing
    ("swing", "EFFECT", "Swing/groove timing", True, [
        ("enabled", "bool", "Enable swing"),
        ("amount", "int", "Swing amount (0-100%, 50=straight)", 0, 100),
        ("resolution", "enum", "Resolution", ["8TH", "16TH", "32ND"]),
        ("groove", "enum", "Groove preset", ["STRAIGHT", "SWING", "SHUFFLE", "HALF_TIME", "DOUBLE_TIME"]),
    ]),
    
    # Gate Time
    ("gate_time", "EFFECT", "Note length/gate time control", True, [
        ("enabled", "bool", "Enable gate control"),
        ("mode", "enum", "Mode", ["FIXED", "PERCENT", "ADD_SUBTRACT"]),
        ("value", "int", "Gate value (depends on mode)", 0, 1000),
    ]),
    
    # Humanize
    ("humanize", "EFFECT", "Humanize timing and velocity", False, [
        ("time_amount", "int", "Timing variation (0-100%)", 0, 100),
        ("velocity_amount", "int", "Velocity variation (0-100%)", 0, 100),
    ]),
    
    # Bellows Expression
    ("bellows_expression", "ACCORDION", "Bellows pressure sensor", False, [
        ("curve", "enum", "Expression curve", ["LINEAR", "EXPONENTIAL", "LOGARITHMIC", "S_CURVE"]),
        ("min_pa", "int", "Minimum pressure (Pa)", 0, 5000),
        ("max_pa", "int", "Maximum pressure (Pa)", 0, 5000),
        ("bidirectional", "bool", "Push/pull detection"),
        ("expression_cc", "int", "Expression CC (0-127)", 0, 127),
    ]),
    
    # Bellows Shake
    ("bellows_shake", "ACCORDION", "Tremolo from bellows shaking", False, [
        ("enabled", "bool", "Enable shake detection"),
        ("sensitivity", "int", "Detection sensitivity (0-100)", 0, 100),
        ("depth", "int", "Tremolo depth (0-127)", 0, 127),
        ("target", "enum", "Target", ["MOD_WHEEL", "VOLUME", "FILTER", "BOTH"]),
    ]),
    
    # Bass Chord System
    ("bass_chord_system", "ACCORDION", "Stradella bass for accordion", True, [
        ("layout", "enum", "Bass layout", ["STRADELLA_120", "STRADELLA_96", "STRADELLA_72", "STRADELLA_48", "FREE_BASS"]),
        ("base_note", "int", "Starting note (0-127)", 0, 127),
        ("octave_doubling", "bool", "Enable octave doubling"),
    ]),
    
    # Register Coupling
    ("register_coupling", "ACCORDION", "Accordion register switching", True, [
        ("register", "enum", "Current register", ["MASTER", "MUSETTE", "BANDONEON", "VIOLIN", "CLARINET", "BASSOON", "PICCOLO", "ORGAN", "OBOE", "FLUTE"]),
        ("smooth_transition", "bool", "Smooth register transition"),
        ("transition_time", "int", "Transition time (ms)", 0, 1000),
    ]),
    
    # Assist Hold
    ("assist_hold", "ACCESSIBILITY", "Auto-hold for motor disabilities", False, [
        ("mode", "enum", "Hold mode", ["OFF", "PERMANENT", "TIMED"]),
        ("duration_ms", "int", "Hold duration (ms, timed mode)", 0, 10000),
        ("velocity_threshold", "int", "Min velocity to hold (1-127)", 1, 127),
    ]),
    
    # Musette Detune
    ("musette_detune", "ACCORDION", "Classic accordion musette/chorus", True, [
        ("style", "enum", "Tuning style", ["DRY", "SCOTTISH", "AMERICAN", "FRENCH", "ITALIAN", "CUSTOM"]),
        ("detune_cents", "int", "Detune amount (cents)", 0, 50),
    ]),
    
    # LiveFX
    ("livefx", "EFFECT", "Live FX system for real-time control", True, [
        ("enabled", "bool", "Enable live FX"),
        ("transpose", "int", "Transpose semitones (-12 to +12)", -12, 12),
        ("velocity_scale", "int", "Velocity scale (0-200%)", 0, 200),
        ("force_scale", "bool", "Force to scale"),
    ]),
    
    # AIN
    ("ain", "INPUT", "Analog input (Hall sensor keyboard)", False, [
        ("enable", "bool", "Enable AIN scanning"),
        ("velocity_enable", "bool", "Enable velocity sensing"),
        ("scan_ms", "int", "Scan interval (ms)", 1, 50),
        ("deadband", "int", "ADC deadband", 0, 100),
    ]),
    
    # Watchdog
    ("watchdog", "SYSTEM", "System watchdog and health monitoring", False, []),
    
    # Config
    ("config", "SYSTEM", "Global system configuration", False, [
        ("srio_enable", "bool", "Enable SRIO subsystem"),
        ("srio_din_enable", "bool", "Enable DIN scanning"),
        ("srio_dout_enable", "bool", "Enable DOUT output"),
    ]),
]

def generate_cli_file(module_name, category, description, has_per_track, parameters):
    """Generate CLI integration file for a module."""
    
    header_name = f"{module_name}.h"
    cli_filename = f"{module_name}_cli.c"
    
    # Start building the file content
    content = f'''/**
 * @file {cli_filename}
 * @brief CLI integration for {module_name} module
 * 
 * {description}
 */

#include "Services/{module_name}/{header_name}"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

'''
    
    if not parameters:
        content += f'''// =============================================================================
// MODULE DESCRIPTOR (No configurable parameters)
// =============================================================================

static int {module_name}_cli_enable(uint8_t track) {{
  (void)track;
  // Module has no enable/disable function
  return 0;
}}

static int {module_name}_cli_disable(uint8_t track) {{
  (void)track;
  return 0;
}}

static int {module_name}_cli_get_status(uint8_t track) {{
  (void)track;
  return MODULE_STATUS_ENABLED;
}}

static module_descriptor_t s_{module_name}_descriptor = {{
  .name = "{module_name}",
  .description = "{description}",
  .category = MODULE_CATEGORY_{category},
  .init = {module_name}_init,
  .enable = {module_name}_cli_enable,
  .disable = {module_name}_cli_disable,
  .get_status = {module_name}_cli_get_status,
  .has_per_track_state = {1 if has_per_track else 0},
  .is_global = {0 if has_per_track else 1}
}};

int {module_name}_register_cli(void) {{
  return module_registry_register(&s_{module_name}_descriptor);
}}
'''
        return content
    
    # Generate parameter wrappers
    content += "// =============================================================================\n"
    content += "// PARAMETER WRAPPERS\n"
    content += "// =============================================================================\n\n"
    
    enum_strings = []
    
    for param in parameters:
        param_name = param[0]
        param_type = param[1]
        
        if param_type == "bool":
            if has_per_track:
                content += f"DEFINE_PARAM_BOOL_TRACK({module_name}, {param_name}, {module_name}_get_{param_name}, {module_name}_set_{param_name})\n\n"
            else:
                content += f"DEFINE_PARAM_BOOL({module_name}, {param_name}, {module_name}_get_{param_name}, {module_name}_set_{param_name})\n\n"
        
        elif param_type == "int":
            if has_per_track:
                content += f"DEFINE_PARAM_INT_TRACK({module_name}, {param_name}, {module_name}_get_{param_name}, {module_name}_set_{param_name})\n\n"
            else:
                content += f"DEFINE_PARAM_INT({module_name}, {param_name}, {module_name}_get_{param_name}, {module_name}_set_{param_name})\n\n"
        
        elif param_type == "enum":
            enum_values = param[3]
            enum_strings.append((param_name, enum_values))
            
            # Generate custom getter/setter for enum
            content += f'''static int {module_name}_param_get_{param_name}(uint8_t track, param_value_t* out) {{
  {'(void)track;' if not has_per_track else ''}
  out->int_val = {module_name}_get_{param_name}({('track' if has_per_track else '')});
  return 0;
}}

static int {module_name}_param_set_{param_name}(uint8_t track, const param_value_t* val) {{
  {'(void)track;' if not has_per_track else ''}
  if (val->int_val < 0 || val->int_val >= {len(enum_values)}) return -1;
  {module_name}_set_{param_name}({('track, ' if has_per_track else '')}(uint8_t)val->int_val);
  return 0;
}}

'''
    
    # Generate module control wrappers
    content += "// =============================================================================\n"
    content += "// MODULE CONTROL WRAPPERS\n"
    content += "// =============================================================================\n\n"
    
    # Find if there's an 'enabled' parameter
    has_enabled = any(p[0] == "enabled" for p in parameters)
    
    if has_enabled:
        if has_per_track:
            content += f"DEFINE_MODULE_CONTROL_TRACK({module_name}, {module_name}_set_enabled, {module_name}_is_enabled)\n\n"
        else:
            content += f"DEFINE_MODULE_CONTROL_GLOBAL({module_name}, {module_name}_set_enabled, {module_name}_is_enabled)\n\n"
    else:
        # No enable/disable
        content += f'''static int {module_name}_cli_enable(uint8_t track) {{
  {'(void)track;' if not has_per_track else ''}
  return 0;
}}

static int {module_name}_cli_disable(uint8_t track) {{
  {'(void)track;' if not has_per_track else ''}
  return 0;
}}

static int {module_name}_cli_get_status(uint8_t track) {{
  {'(void)track;' if not has_per_track else ''}
  return MODULE_STATUS_ENABLED;
}}

'''
    
    # Generate enum strings
    if enum_strings:
        content += "// =============================================================================\n"
        content += "// ENUM STRINGS\n"
        content += "// =============================================================================\n\n"
        
        for enum_name, enum_values in enum_strings:
            content += f"static const char* s_{enum_name}_names[] = {{\n"
            for val in enum_values:
                content += f'  "{val}",\n'
            content += "};\n\n"
    
    # Generate module descriptor
    content += "// =============================================================================\n"
    content += "// MODULE DESCRIPTOR\n"
    content += "// =============================================================================\n\n"
    
    content += f'''static module_descriptor_t s_{module_name}_descriptor = {{
  .name = "{module_name}",
  .description = "{description}",
  .category = MODULE_CATEGORY_{category},
  .init = {module_name}_init,
  .enable = {module_name}_cli_enable,
  .disable = {module_name}_cli_disable,
  .get_status = {module_name}_cli_get_status,
  .has_per_track_state = {1 if has_per_track else 0},
  .is_global = {0 if has_per_track else 1}
}};

'''
    
    # Generate parameter setup function
    content += "// =============================================================================\n"
    content += "// PARAMETER SETUP\n"
    content += "// =============================================================================\n\n"
    
    content += f"static void setup_{module_name}_parameters(void) {{\n"
    content += "  module_param_t params[] = {\n"
    
    for param in parameters:
        param_name = param[0]
        param_type = param[1]
        param_desc = param[2]
        
        if param_type == "bool":
            content += f"    PARAM_BOOL({module_name}, {param_name}, \"{param_desc}\"),\n"
        elif param_type == "int":
            min_val = param[3]
            max_val = param[4]
            content += f"    PARAM_INT({module_name}, {param_name}, \"{param_desc}\", {min_val}, {max_val}),\n"
        elif param_type == "enum":
            enum_values = param[3]
            content += f'''    {{
      .name = "{param_name}",
      .description = "{param_desc}",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = {len(enum_values) - 1},
      .enum_values = s_{param_name}_names,
      .enum_count = {len(enum_values)},
      .read_only = 0,
      .get_value = {module_name}_param_get_{param_name},
      .set_value = {module_name}_param_set_{param_name}
    }},
'''
    
    content += "  };\n"
    content += f"  \n"
    content += f"  s_{module_name}_descriptor.param_count = sizeof(params) / sizeof(params[0]);\n"
    content += f"  memcpy(s_{module_name}_descriptor.params, params, sizeof(params));\n"
    content += "}\n\n"
    
    # Generate registration function
    content += "// =============================================================================\n"
    content += "// REGISTRATION\n"
    content += "// =============================================================================\n\n"
    
    content += f'''int {module_name}_register_cli(void) {{
  setup_{module_name}_parameters();
  return module_registry_register(&s_{module_name}_descriptor);
}}
'''
    
    return content

def main():
    """Generate all CLI files."""
    cli_dir = "/home/runner/work/MidiCore/MidiCore/Services/cli"
    
    print(f"Generating CLI files in {cli_dir}...")
    
    for module_name, category, description, has_per_track, parameters in MODULES:
        cli_file = os.path.join(cli_dir, f"{module_name}_cli.c")
        
        # Check if file already exists
        if os.path.exists(cli_file):
            print(f"Skipping {module_name}_cli.c (already exists)")
            continue
        
        content = generate_cli_file(module_name, category, description, has_per_track, parameters)
        
        with open(cli_file, 'w') as f:
            f.write(content)
        
        print(f"Created {module_name}_cli.c")
    
    print("Done!")

if __name__ == "__main__":
    main()

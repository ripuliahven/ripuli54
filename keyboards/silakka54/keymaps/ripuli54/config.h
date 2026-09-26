/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

// 9 typing layers (0-8) for Vial, plus 3 fixed MIDI layers (9-11, see
// keymap.c) that live above what's exposed for regular keyboard design.
#ifdef MIDI_ENABLE
#    define DYNAMIC_KEYMAP_LAYER_COUNT 12
#else
#    define DYNAMIC_KEYMAP_LAYER_COUNT 9
#endif
#define VIAL_KEYBOARD_UID {0x07, 0xBC, 0x7D, 0x8A, 0x7E, 0x4A, 0x67, 0xEC}
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

// Enables the MI_* note/octave/sustain keycodes (not just raw MIDI plumbing)
#ifdef MIDI_ENABLE
#    define MIDI_ADVANCED
#endif

// Disables the "tap 5 times to lock" behavior for one-shot mods/layers --
// QMK only locks when this is >1, so 1 keeps pure tap-once-applies-to-next
// semantics with no accidental locking.
#define ONESHOT_TAP_TOGGLE 1

// Queued one-shot mods never expire; only a non-mod key or Esc clears them.
#define ONESHOT_TIMEOUT 0

// Debounce window (default 5 ms); longer filters chatter on worn switches.
#define DEBOUNCE 10

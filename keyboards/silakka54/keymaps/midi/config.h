/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

// 9 typing layers (0-8) for Vial, plus 3 fixed MIDI layers (9-11, see
// keymap.c) that live above what's exposed for regular keyboard design.
#define DYNAMIC_KEYMAP_LAYER_COUNT 12
#define VIAL_KEYBOARD_UID {0x07, 0xBC, 0x7D, 0x8A, 0x7E, 0x4A, 0x67, 0xEC}
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

// Enables the MI_* note/octave/sustain keycodes (not just raw MIDI plumbing)
#define MIDI_ADVANCED

// Disables the "tap 5 times to lock" behavior for one-shot mods/layers --
// QMK only locks when this is >1, so 1 keeps pure tap-once-applies-to-next
// semantics with no accidental locking.
#define ONESHOT_TAP_TOGGLE 1

// Debounce window (default 5 ms); longer filters chatter on worn switches.
#define DEBOUNCE 10

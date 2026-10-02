// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

// All custom keycodes belong to the MIDI modules.
#ifdef MIDI_ENABLE

// Piano grid: 3x6 keys per hand.
#define HAND_GRID_SIZE 18
#define PIANO_GRID_SIZE (HAND_GRID_SIZE * 2)
#define NUM_SCALES 7 // entries in SCALES[] (piano)

// Drums live in QK_KB (0x7E00-0x7E3F), which Vial names via customKeycodes
// in vial.json -- keep the two lists in the same order.
enum drum_keycodes {
    DRM_KICK = QK_KB_0,    // 36 Bass Drum 1
    DRM_SNARE,             // 38 Acoustic Snare
    DRM_RIM,               // 37 Side Stick
    DRM_CLAP,              // 39 Hand Clap
    DRM_HHCL,              // 42 Closed Hi-Hat
    DRM_HHOP,              // 46 Open Hi-Hat
    DRM_HHPD,              // 44 Pedal Hi-Hat
    DRM_TOML,              // 45 Low Tom
    DRM_TOMM,              // 47 Mid Tom
    DRM_TOMH,              // 50 High Tom
    DRM_CRASH,             // 49 Crash Cymbal 1
    DRM_RIDE,              // 51 Ride Cymbal 1
    DRM_CRASH2,            // 57 Crash Cymbal 2
    DRM_SPLASH,            // 55 Splash Cymbal
    DRM_COWBELL,           // 56 Cowbell
    DRM_CHINA,             // 52 Chinese Cymbal
    DRM_RIDEBELL,          // 53 Ride Bell
    DRM_CHOKE,             // silences all currently-ringing cymbals
};
_Static_assert((uint16_t)DRM_CHOKE <= (uint16_t)QK_KB_MAX, "drum keycodes overflow Vial's custom range");

// Piano stays in QK_USER: positional, so Vial shows these as hex.
enum piano_keycodes {
    SC_KEY0 = SAFE_RANGE,                     // reserves SC_KEY0 .. SC_KEY0+PIANO_GRID_SIZE-1
    SC_SCALE0 = SC_KEY0 + PIANO_GRID_SIZE,     // reserves SC_SCALE0 .. SC_SCALE0+NUM_SCALES-1 (index into SCALES[])
    SC_ROOT0 = SC_SCALE0 + NUM_SCALES,        // reserves SC_ROOT0 .. SC_ROOT0+11 (see ROOT_SEMITONES)
};

#endif // MIDI_ENABLE

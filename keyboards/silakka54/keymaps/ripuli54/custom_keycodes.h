// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

// Piano grid: 3x6 keys per hand.
#define HAND_GRID_SIZE 18
#define PIANO_GRID_SIZE (HAND_GRID_SIZE * 2)
#define NUM_SCALES 7 // entries in SCALES[] (piano)

enum custom_keycodes {
    DRM_KICK = SAFE_RANGE, // 36 Bass Drum 1
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
    SC_KEY0,                                  // reserves SC_KEY0 .. SC_KEY0+PIANO_GRID_SIZE-1
    SC_SCALE0 = SC_KEY0 + PIANO_GRID_SIZE,     // reserves SC_SCALE0 .. SC_SCALE0+NUM_SCALES-1 (index into SCALES[])
    SC_ROOT0 = SC_SCALE0 + NUM_SCALES,        // reserves SC_ROOT0 .. SC_ROOT0+11 (see ROOT_SEMITONES)
};

// Shorthand for the piano grid so the LAYOUT() calls stay readable:
// L(n)/R(n) is scale-degree slot n (0-17) of the left/right hand.
#define L(n) (SC_KEY0 + (n))
#define R(n) (SC_KEY0 + HAND_GRID_SIZE + (n))

// Root-select shorthand: ROOT(n) is the n-th key of the top row, left to
// right across both hands (see ROOT_SEMITONES for the note each one picks).
#define ROOT(n) (SC_ROOT0 + (n))

// Scale-select shorthand: SCALE(n) picks SCALES[n] directly (0=major,
// 1=minor, 2=major penta, 3=minor penta, 4=chromatic).
#define SCALE(n) (SC_SCALE0 + (n))

// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "qmk_midi.h"

// Layers 2-8 are left undefined below (blank/KC_NO) -- free for regular
// keyboard layers, designed entirely in Vial. MIDI gets pushed to the top
// of the layer stack (9-11) so it doesn't eat into that range.
enum layers {
    _BASE = 0,
    _FN,
    _PIANO = 9,
    _PIANO_SCALE,
    _DRUM,
};

// General MIDI percussion note numbers (channel 10 / index 9)
#define DRUM_MIDI_CHANNEL 9
#define DRUM_VELOCITY 110

// Piano layer: all 36 grid keys share ONE continuous run of SCALE DEGREES
// (signed -- can go negative) around a single root note (PIANO_ROOT_NOTE).
// PIANO_KEY_DEGREES[] below gives each physical key's degree, in LAYOUT()
// slot order (left hand slots 0-17, then right hand slots 18-35); degree 0
// is the root, negative degrees sit below it, wrapping to the next octave
// every SCALE_LENGTHS[n] steps either direction. The top/number row picks
// the root directly (it plays nothing else in this layer); hold a thumb key
// to turn that same row into the scale picker instead (see _PIANO_SCALE).
#define HAND_GRID_SIZE 18
#define PIANO_GRID_SIZE (HAND_GRID_SIZE * 2)
#define PIANO_ROOT_NOTE 60 // C4 -- lands on R(0), the right hand's bottom-left key
#define PIANO_MIDI_CHANNEL 0
#define PIANO_VELOCITY 100

static const int8_t SCALE_MAJOR[]     = {0, 2, 4, 5, 7, 9, 11};
static const int8_t SCALE_MINOR[]     = {0, 2, 3, 5, 7, 8, 10};
static const int8_t SCALE_HARM_MINOR[] = {0, 2, 3, 5, 7, 8, 11};
static const int8_t SCALE_MEL_MINOR[]  = {0, 2, 3, 5, 7, 9, 11};
static const int8_t SCALE_MAJ_PENTA[] = {0, 2, 4, 7, 9};
static const int8_t SCALE_MIN_PENTA[] = {0, 3, 5, 7, 10};
static const int8_t SCALE_CHROMATIC[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static const int8_t *const SCALES[]        = {SCALE_MAJOR, SCALE_MINOR, SCALE_HARM_MINOR, SCALE_MEL_MINOR, SCALE_MAJ_PENTA, SCALE_MIN_PENTA, SCALE_CHROMATIC};
static const uint8_t        SCALE_LENGTHS[] = {7, 7, 7, 7, 5, 5, 12};
#define NUM_SCALES 7

// Root note picked via the _PIANO top row, in physical left-to-right key
// order (A, A#, B, C, C#, D, D#, E, F, F#, G, G#); values are semitones added
// relative to the C the hand base notes are defined against.
static const int8_t ROOT_SEMITONES[12] = {9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8};

// Signed scale-degree for each physical piano key, in LAYOUT() slot order
// (left hand slots 0-17, then right hand slots 18-35). Degree 0 is the root;
// this default is one continuous run from -18 (left hand, bottom-left) to
// +17 (right hand, top-right), landing the root on the right hand's
// bottom-left key. Regenerate from the layout editor for a different split.
static const int8_t PIANO_KEY_DEGREES[PIANO_GRID_SIZE] = {
    -6, -5, -4, -3, -2, -1, -12, -11, -10, -9, -8, -7, -18, -17, -16, -15, -14, -13,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
};

static uint8_t current_scale             = 0; // index into SCALES/SCALE_LENGTHS
static int8_t  root_offset               = 0; // semitones added to every note, wraps 0-11
static uint8_t piano_note_status[PIANO_GRID_SIZE];

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

void keyboard_post_init_user(void) {
    for (uint8_t i = 0; i < PIANO_GRID_SIZE; i++) {
        piano_note_status[i] = 0xFF;
    }
}

// Noteoff fires on actual key release (real, natural hold duration) rather
// than an artificial fixed delay, which proved unreliable (too short some
// fraction of the time, causing random silent drops regardless of routing).
// Several physical keys can map to the same drum note (e.g. the left-thumb
// kick and every kick in the grid), so a hold-count per note ensures a
// noteoff only fires once no key mapped to it is still held -- one key's
// release can't cut off another key's still-sounding hit.
static uint8_t drum_hold_count[128];

static void drum_hit(uint8_t note, bool pressed) {
    if (pressed) {
        midi_send_noteon(&midi_device, DRUM_MIDI_CHANNEL, note, DRUM_VELOCITY);
        drum_hold_count[note]++;
    } else {
        if (drum_hold_count[note] > 0) {
            drum_hold_count[note]--;
        }
        if (drum_hold_count[note] == 0) {
            midi_send_noteoff(&midi_device, DRUM_MIDI_CHANNEL, note, 0);
        }
    }
}

// Cymbals a choke key should silence -- the "ringy" percussion, not the
// short/decisive hits (kick, snare, toms, hi-hat, rim, clap).
static const uint8_t CHOKE_NOTES[] = {49, 51, 57, 55, 52, 53}; // crash, ride, crash2, splash, china, ride bell

static void drum_choke(void) {
    for (uint8_t i = 0; i < sizeof(CHOKE_NOTES); i++) {
        uint8_t note = CHOKE_NOTES[i];
        if (drum_hold_count[note] > 0) {
            drum_hold_count[note] = 0;
            midi_send_noteoff(&midi_device, DRUM_MIDI_CHANNEL, note, 0);
        }
    }
}

static void piano_scale_key(uint8_t grid_index, bool pressed) {
    if (pressed) {
        int16_t degree_raw = PIANO_KEY_DEGREES[grid_index];
        int16_t scale_len  = SCALE_LENGTHS[current_scale];
        int16_t degree     = degree_raw % scale_len;
        if (degree < 0) degree += scale_len;
        int16_t octave     = (degree_raw - degree) / scale_len; // exact: degree_raw - degree is a multiple of scale_len
        int16_t note       = PIANO_ROOT_NOTE + root_offset + 12 * octave + SCALES[current_scale][degree];
        if (note < 0) note = 0;
        if (note > 127) note = 127;
        piano_note_status[grid_index] = (uint8_t)note;
        midi_send_noteon(&midi_device, PIANO_MIDI_CHANNEL, (uint8_t)note, PIANO_VELOCITY);
    } else if (piano_note_status[grid_index] != 0xFF) {
        midi_send_noteoff(&midi_device, PIANO_MIDI_CHANNEL, piano_note_status[grid_index], 0);
        piano_note_status[grid_index] = 0xFF;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DRM_KICK:  drum_hit(36, record->event.pressed); return false;
        case DRM_SNARE: drum_hit(38, record->event.pressed); return false;
        case DRM_RIM:   drum_hit(37, record->event.pressed); return false;
        case DRM_CLAP:  drum_hit(39, record->event.pressed); return false;
        case DRM_HHCL:  drum_hit(42, record->event.pressed); return false;
        case DRM_HHOP:  drum_hit(46, record->event.pressed); return false;
        case DRM_HHPD:  drum_hit(44, record->event.pressed); return false;
        case DRM_TOML:  drum_hit(45, record->event.pressed); return false;
        case DRM_TOMM:  drum_hit(47, record->event.pressed); return false;
        case DRM_TOMH:  drum_hit(50, record->event.pressed); return false;
        case DRM_CRASH: drum_hit(49, record->event.pressed); return false;
        case DRM_RIDE:  drum_hit(51, record->event.pressed); return false;
        case DRM_CRASH2:   drum_hit(57, record->event.pressed); return false;
        case DRM_SPLASH:   drum_hit(55, record->event.pressed); return false;
        case DRM_COWBELL:  drum_hit(56, record->event.pressed); return false;
        case DRM_CHINA:    drum_hit(52, record->event.pressed); return false;
        case DRM_RIDEBELL: drum_hit(53, record->event.pressed); return false;
        case DRM_CHOKE:
            if (record->event.pressed) drum_choke();
            return false;

        case SC_KEY0 ... SC_KEY0 + PIANO_GRID_SIZE - 1:
            piano_scale_key(keycode - SC_KEY0, record->event.pressed);
            return false;

        case SC_SCALE0 ... SC_SCALE0 + NUM_SCALES - 1:
            if (record->event.pressed) current_scale = keycode - SC_SCALE0;
            return false;

        case SC_ROOT0 ... SC_ROOT0 + 11:
            if (record->event.pressed) root_offset = ROOT_SEMITONES[keycode - SC_ROOT0];
            return false;
    }
    return true;
}

// Shorthand for the piano grid so the LAYOUT() calls below stay readable:
// L(n)/R(n) is scale-degree slot n (0-17) of the left/right hand.
#define L(n) (SC_KEY0 + (n))
#define R(n) (SC_KEY0 + HAND_GRID_SIZE + (n))

// Root-select shorthand: ROOT(n) is the n-th key of the top row, left to
// right across both hands (see ROOT_SEMITONES for the note each one picks).
#define ROOT(n) (SC_ROOT0 + (n))

// Scale-select shorthand: SCALE(n) picks SCALES[n] directly (0=major,
// 1=minor, 2=major penta, 3=minor penta, 4=chromatic).
#define SCALE(n) (SC_SCALE0 + (n))

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                               KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                               KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
        KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                               KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                               KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                                            KC_LGUI, MO(_FN), KC_SPC,           KC_ENT,  KC_RCTL,  KC_RALT
    ),

    // Fn layer: hold Fn for normal Fn-keys/navigation; tap Fn+Space or Fn+Enter
    // to latch into the Piano or Drum MIDI layers.
    [_FN] = LAYOUT(
        KC_GRV,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                              KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_PGUP, KC_PGDN, KC_HOME, KC_END,  KC_DEL,  KC_F12,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_LBRC, KC_RBRC,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                                            KC_TRNS, KC_TRNS, TG(_PIANO),      TG(_DRUM), EE_CLR,  QK_BOOT
    ),

    // Piano layer: see PIANO_KEY_DEGREES above for which key plays which
    // degree -- by default the root sits at R(0) (right hand bottom-left),
    // with the left hand continuing downward into negative degrees. The top
    // row picks the root directly (tap, no hold needed).
    // Left thumb: RCtrl=hold for scale picker, RAlt=momentary Drum overlay
    // so you can drop a kick/snare without leaving the layer.
    [_PIANO] = LAYOUT(
        ROOT(0), ROOT(1), ROOT(2), ROOT(3), ROOT(4), ROOT(5),                             ROOT(6), ROOT(7), ROOT(8), ROOT(9), ROOT(10),ROOT(11),
        L(12),   L(13),   L(14),   L(15),   L(16),   L(17),                               R(12),   R(13),   R(14),   R(15),   R(16),   R(17),
        L(6),    L(7),    L(8),    L(9),    L(10),   L(11),                               R(6),    R(7),    R(8),    R(9),    R(10),   R(11),
        L(0),    L(1),    L(2),    L(3),    L(4),    L(5),                                R(0),    R(1),    R(2),    R(3),    R(4),    R(5),
                                            TO(_BASE), KC_TRNS, KC_TRNS,         MO(_PIANO_SCALE), KC_TRNS, MO(_DRUM)
    ),

    // Scale picker overlay: hold the _PIANO right-thumb key, then tap one of
    // the first 7 top-row keys to jump straight to that scale -- major,
    // minor, harmonic minor, melodic minor, major penta, minor penta,
    // chromatic, left to right. The rest of the top row stays transparent,
    // so those keys keep picking the root even while this is held.
    [_PIANO_SCALE] = LAYOUT(
        SCALE(0),SCALE(1),SCALE(2),SCALE(3),SCALE(4),SCALE(5),                             SCALE(6),KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                              KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                              KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                              KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                                            KC_TRNS, KC_TRNS, KC_TRNS,        KC_TRNS, KC_TRNS, KC_TRNS
    ),

    // Drum layer: the core kit (hi-hats/rim/clap/toms/crash/ride/kick) is
    // repeated on the bottom 3 rows so any finger/row reach hits the same
    // sound per column. The top row swaps in the less-frequently-used extra
    // cymbals/percussion instead of duplicating hi-hats, since it's the
    // farthest row to reach for fast rolls. Thumbs carry the two most
    // frequent hits (kick/snare) plus an all-notes-off panic key.
    [_DRUM] = LAYOUT(
        DRM_HHCL, DRM_CRASH2, DRM_SPLASH, DRM_CHINA, DRM_COWBELL, DRM_TOML,                DRM_TOML, DRM_TOMM, DRM_TOMH, DRM_CRASH, DRM_RIDE, DRM_KICK,
        DRM_HHCL, DRM_HHCL,   DRM_HHOP,   DRM_HHPD,  DRM_RIM,     DRM_TOMM,                DRM_TOMM, DRM_TOMM, DRM_TOMH, DRM_CRASH, DRM_RIDE, DRM_KICK,
        DRM_HHCL, DRM_HHCL,   DRM_HHOP,   DRM_HHPD,  DRM_RIM,     DRM_TOMH,                DRM_TOMH, DRM_TOMM, DRM_TOMH, DRM_CRASH, DRM_RIDE, DRM_KICK,
        DRM_HHCL, DRM_HHCL,   DRM_HHOP,   DRM_HHPD,  DRM_RIM,     DRM_KICK,                DRM_KICK, DRM_TOMM, DRM_TOMH, DRM_CRASH, DRM_RIDE, DRM_KICK,
                                            TO(_BASE), KC_TRNS, DRM_SNARE,      DRM_SNARE, MI_AOFF,  DRM_CHOKE
    )
};

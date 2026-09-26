// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "custom_keycodes.h"
#include "drums.h"
#include "osm.h"
#include "piano.h"

// Layers 2-8 are left undefined below (blank/KC_NO) -- free for regular
// keyboard layers, designed entirely in Vial. MIDI gets pushed to the top
// of the layer stack (9-11) so it doesn't eat into that range.
enum layers {
    _BASE = 0,
    _FN,
#ifdef MIDI_ENABLE
    _PIANO = 9,
    _PIANO_SCALE,
    _DRUM,
#endif
};

// Fn thumb keys that latch into the MIDI layers; plain Fn without MIDI.
#ifdef MIDI_ENABLE
#    define FN_PIANO TG(_PIANO)
#    define FN_DRUM TG(_DRUM)
#else
#    define FN_PIANO KC_TRNS
#    define FN_DRUM KC_TRNS
#endif

// Each module returns false once it has handled the key.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_osm(keycode, record)) {
        return false;
    }
#ifdef MIDI_ENABLE
    if (!process_record_drums(keycode, record)) {
        return false;
    }
    if (!process_record_piano(keycode, record)) {
        return false;
    }
#endif
    return true;
}

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
                                            KC_TRNS, KC_TRNS, FN_PIANO,        FN_DRUM,   EE_CLR,  QK_BOOT
    ),

#ifdef MIDI_ENABLE
    // Piano layer: see PIANO_KEY_DEGREES in piano.c for which key plays which
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
    ),
#endif
};

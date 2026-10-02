// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "custom_keycodes.h"
#include "drums.h"
#include "osm.h"
#include "piano.h"

// Default layers (keymaps[]) are generated from vial/ripuli54.vil by
// build.sh. Layers 0-8 are for typing; 9-11 are Piano, Piano scale picker,
// and Drums (MIDI builds only).
#include "keymap_vil.h"

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

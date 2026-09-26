// SPDX-License-Identifier: GPL-2.0-or-later

#include "drums.h"
#include "custom_keycodes.h"
#include "qmk_midi.h"

// General MIDI percussion note numbers (channel 10 / index 9)
#define DRUM_MIDI_CHANNEL 9
#define DRUM_VELOCITY 110

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

bool process_record_drums(uint16_t keycode, keyrecord_t *record) {
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
    }
    return true;
}

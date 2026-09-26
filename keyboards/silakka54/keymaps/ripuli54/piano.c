// SPDX-License-Identifier: GPL-2.0-or-later

#include "piano.h"
#include "custom_keycodes.h"
#include "qmk_midi.h"

// Piano layer: all 36 grid keys share ONE continuous run of SCALE DEGREES
// (signed -- can go negative) around a single root note (PIANO_ROOT_NOTE).
// PIANO_KEY_DEGREES[] below gives each physical key's degree, in LAYOUT()
// slot order (left hand slots 0-17, then right hand slots 18-35); degree 0
// is the root, negative degrees sit below it, wrapping to the next octave
// every SCALE_LENGTHS[n] steps either direction. The top/number row picks
// the root directly (it plays nothing else in this layer); hold a thumb key
// to turn that same row into the scale picker instead (see _PIANO_SCALE).
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
static uint8_t piano_note_status[PIANO_GRID_SIZE] = {[0 ... PIANO_GRID_SIZE - 1] = 0xFF}; // 0xFF = not sounding

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

bool process_record_piano(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
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

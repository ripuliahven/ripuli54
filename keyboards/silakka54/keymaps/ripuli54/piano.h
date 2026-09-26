// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

// Handles the SC_* keycodes; returns false if the key was handled here.
bool process_record_piano(uint16_t keycode, keyrecord_t *record);

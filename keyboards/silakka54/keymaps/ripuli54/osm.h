// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

// Returns false if the key was fully handled here.
bool process_record_osm(uint16_t keycode, keyrecord_t *record);

// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

// Handles the DRM_* keycodes; returns false if the key was handled here.
bool process_record_drums(uint16_t keycode, keyrecord_t *record);

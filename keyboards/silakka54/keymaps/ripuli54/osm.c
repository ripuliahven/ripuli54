// SPDX-License-Identifier: GPL-2.0-or-later

#include "osm.h"

bool process_record_osm(uint16_t keycode, keyrecord_t *record) {
    // Escape cancels a pending one-shot mod (OSM) instead of applying it,
    // so tapping a tap-mod and changing your mind just needs a tap of Esc.
    // Esc itself still sends normally, just with the mod already dropped.
    if (keycode == KC_ESC && record->event.pressed) {
        clear_oneshot_mods();
        clear_oneshot_locked_mods();
    }

    // OSM keys always queue their mod, whether tapped or held, so chording
    // several of them stacks instead of dropping the ones that became holds.
    // The mods stay queued until a non-mod key uses them (or Esc clears them).
    // While held they also act as normal mods, but only get registered once
    // another key is pressed, so a released chord never sends a lone GUI tap.
    static uint8_t osm_held;
    static uint8_t osm_registered;
    if (IS_QK_ONE_SHOT_MOD(keycode)) {
        uint8_t mods = QK_ONE_SHOT_MOD_GET_MODS(keycode);
        // 5-bit mod format: bit 4 flags right-hand mods
        mods = mods & 0x10 ? (mods & 0x0F) << 4 : mods;
        if (record->event.pressed && record->tap.count >= 2) {
            // Double tap: plain mod, so e.g. GUI alone still opens the menu
            del_oneshot_mods(mods);
            register_mods(mods);
            osm_registered |= mods;
        } else if (record->event.pressed) {
            osm_held |= mods;
            add_oneshot_mods(mods);
        } else {
            osm_held &= ~mods;
            if (osm_registered & mods) {
                unregister_mods(osm_registered & mods);
                osm_registered &= ~mods;
            }
        }
        return false;
    }
    if (record->event.pressed && osm_held && !IS_QK_MOMENTARY(keycode)) {
        register_mods(osm_held);
        osm_registered |= osm_held;
    }
    return true;
}

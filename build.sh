#!/usr/bin/env bash
# Builds the silakka54 "midi" keymap into a flashable .uf2.
# Fetches the pinned vial-qmk-silakka54 firmware fork under ./build,
# links this repo's keymap into it, and runs `qmk compile`.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QMK_REPO_URL="https://github.com/RAvenGEr/vial-qmk-silakka54.git"
# Pinned so everyone builds against the same QMK; bump deliberately.
QMK_REV="02a98ccd7104a9fe1600fced4881dec0060b4416"
QMK_DIR="$SCRIPT_DIR/build/vial-qmk-silakka54"
KEYMAP_SRC="$SCRIPT_DIR/keyboards/silakka54/keymaps/midi"
KEYMAP_DST="$QMK_DIR/keyboards/silakka54/keymaps/midi"

if ! command -v qmk >/dev/null 2>&1; then
    echo "error: qmk CLI not found on PATH (pip install qmk)" >&2
    exit 1
fi

if [ ! -d "$QMK_DIR/.git" ]; then
    git init -q "$QMK_DIR"
    git -C "$QMK_DIR" remote add origin "$QMK_REPO_URL"
fi

if [ "$(git -C "$QMK_DIR" rev-parse -q --verify HEAD || true)" != "$QMK_REV" ]; then
    echo "==> Checking out vial-qmk-silakka54 @ ${QMK_REV:0:10} into build/..."
    git -C "$QMK_DIR" fetch --depth 1 origin "$QMK_REV"
    # --force drops the build_id.py patch; it's reapplied below.
    git -C "$QMK_DIR" checkout -q --force FETCH_HEAD
fi

echo "==> Fetching required submodules..."
git -C "$QMK_DIR" submodule update --init --depth 1 \
    lib/pico-sdk lib/chibios lib/chibios-contrib lib/lufa lib/printf

# Vial validates its EEPROM keymap storage against a BUILD_ID magic that
# util/build_id.py otherwise randomizes on every single compile -- meaning a
# stock build wipes Vial's saved keymap back to firmware defaults on every
# reflash, not just ones with an actual layout change. Pin it to a fixed
# value so the EEPROM only resets when you deliberately clear it (EE_CLR or
# Vial's "Reset EEPROM").
BUILD_ID_SCRIPT="$QMK_DIR/util/build_id.py"
if ! grep -q "0x52495031" "$BUILD_ID_SCRIPT" 2>/dev/null; then
    echo "==> Pinning BUILD_ID so reflashing doesn't wipe Vial's saved keymap..."
    cat > "$BUILD_ID_SCRIPT" <<'EOF'
def main():
    # Fixed (not random) so Vial's EEPROM magic stays stable across
    # reflashes -- see ripuli54/build.sh, which patches this in on clone.
    print("#define BUILD_ID ((uint32_t)0x{:08X})".format(0x52495031))


if __name__ == "__main__":
    main()
EOF
fi

# Symlinked (not copied) so the build and clangd always see the live files.
if [ ! -L "$KEYMAP_DST" ]; then
    echo "==> Linking midi keymap into the QMK tree..."
    rm -rf "$KEYMAP_DST"
    ln -s ../../../../../keyboards/silakka54/keymaps/midi "$KEYMAP_DST"
fi

# clangd database. Only regenerated when flag inputs change, and before
# compiling since generation wipes .build/.
CDB="$SCRIPT_DIR/compile_commands.json"
if [ ! -f "$CDB" ] || [ -n "$(find "$KEYMAP_SRC/rules.mk" "$KEYMAP_SRC/config.h" \
        "$SCRIPT_DIR/build.sh" "$SCRIPT_DIR/util/compiledb.py" -newer "$CDB")" ]; then
    echo "==> Generating compile_commands.json..."
    (cd "$QMK_DIR" && qmk generate-compilation-database -kb silakka54 -km midi)
    python3 "$SCRIPT_DIR/util/compiledb.py" "$QMK_DIR" "$SCRIPT_DIR" "$KEYMAP_SRC"
fi

echo "==> Compiling..."
(cd "$QMK_DIR" && qmk compile -kb silakka54 -km midi)

UF2="$QMK_DIR/.build/silakka54_midi.uf2"
if [ -f "$UF2" ]; then
    echo "==> Build succeeded: ${UF2#"$SCRIPT_DIR"/}"
else
    echo "error: build finished but .uf2 not found at expected path: $UF2" >&2
    exit 1
fi

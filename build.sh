#!/usr/bin/env bash
# Builds the silakka54 "midi" keymap into a flashable .uf2.
# Clones/updates the vial-qmk-silakka54 firmware fork under ./build,
# drops this repo's keymap into it, and runs `qmk compile`.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QMK_REPO_URL="https://github.com/RAvenGEr/vial-qmk-silakka54.git"
QMK_DIR="$SCRIPT_DIR/build/vial-qmk-silakka54"
KEYMAP_SRC="$SCRIPT_DIR/keyboards/silakka54/keymaps/midi"
KEYMAP_DST="$QMK_DIR/keyboards/silakka54/keymaps/midi"

if ! command -v qmk >/dev/null 2>&1; then
    echo "error: qmk CLI not found on PATH (pip install qmk)" >&2
    exit 1
fi

if [ ! -d "$QMK_DIR" ]; then
    echo "==> Cloning vial-qmk-silakka54 into build/..."
    git clone "$QMK_REPO_URL" "$QMK_DIR"
fi

echo "==> Fetching required submodules..."
git -C "$QMK_DIR" submodule update --init --depth 1 \
    lib/pico-sdk lib/chibios lib/chibios-contrib lib/lufa lib/printf

echo "==> Copying midi keymap..."
rm -rf "$KEYMAP_DST"
cp -r "$KEYMAP_SRC" "$KEYMAP_DST"

echo "==> Compiling..."
(cd "$QMK_DIR" && qmk compile -kb silakka54 -km midi)

UF2="$QMK_DIR/.build/silakka54_midi.uf2"
if [ -f "$UF2" ]; then
    echo "==> Build succeeded: ${UF2#"$SCRIPT_DIR"/}"
else
    echo "error: build finished but .uf2 not found at expected path: $UF2" >&2
    exit 1
fi

#!/usr/bin/env python3
"""Update the keymap's default layers from a Vial .vil save file.

Usage: util/vil2keymap.py [file.vil [qmk_dir [out.h]]]
Defaults: vial/ripuli54.vil, build/vial-qmk-silakka54, the keymap's
keymap_vil.h. Run by hand after saving a layout you want as the default;
an EEPROM reset restores whatever was last written. Vial saves some old
QMK names (KC_BSPACE...), translated via vial-qmk's own alias list.
"""
import json
import re
import sys
from pathlib import Path

# Layers from here up are the MIDI ones (see config.h).
FIRST_MIDI_LAYER = 9

REPO = Path(__file__).resolve().parent.parent
DEFAULTS = [
    REPO / "vial/ripuli54.vil",
    REPO / "build/vial-qmk-silakka54",
    REPO / "keyboards/silakka54/keymaps/ripuli54/keymap_vil.h",
]
args = [Path(a) for a in sys.argv[1:4]]
vil_path, qmk_dir, out_path = args + DEFAULTS[len(args):]

alias_header = qmk_dir / "quantum/vial_ensure_keycode.h"
if not alias_header.exists():
    sys.exit(f"error: {alias_header} not found -- run ./build.sh once first")

aliases = {}
for line in alias_header.read_text().splitlines():
    m = re.match(r"#define\s+(\w+)\s+(\w+)\s*$", line)
    if m and m.group(1) != "kc":
        aliases[m.group(1)] = m.group(2)


def to_c(kc):
    if kc == -1:  # no key at this matrix position
        return "KC_NO"
    return re.sub(r"\w+", lambda m: aliases.get(m.group(0), m.group(0)), kc)


layers = json.loads(vil_path.read_text())["layout"]

out = [
    f"// Generated from {vil_path.name} by util/vil2keymap.py -- do not edit.",
    "// Raw matrix order: rows 0-4 left half, 5-9 right half.",
    "",
    "#pragma once",
    "",
    "const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {",
]
for i, layer in enumerate(layers):
    if i == FIRST_MIDI_LAYER:
        out.append("#ifdef MIDI_ENABLE")
    out.append(f"    [{i}] = {{")
    for row in layer:
        out.append("        {" + ", ".join(to_c(k) for k in row) + "},")
    out.append("    },")
if len(layers) > FIRST_MIDI_LAYER:
    out.append("#endif // MIDI_ENABLE")
out.append("};")
text = "\n".join(out) + "\n"

if out_path.exists() and out_path.read_text() == text:
    print(f"{out_path.name} already matches {vil_path.name}")
else:
    out_path.write_text(text)
    print(f"Updated {out_path.name} from {vil_path.name}")

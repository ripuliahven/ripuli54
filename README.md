# ripuli54

MIDI keymap for the Silakka54 split keyboard, built on [vial-qmk](https://github.com/RAvenGEr/vial-qmk-silakka54)
(the PandaKB Silakka54 fork of Vial-QMK). Play piano (with scale/root pickers)
or a drum kit straight off the keyboard over USB-MIDI.

This repo holds only the keymap itself, not a full QMK fork. QMK is a build-time
dependency — see below.

## What's here

```
keyboards/silakka54/keymaps/midi/
    keymap.c    -- all the logic: piano scale-degree engine, drum hit/choke
                   handling, root/scale pickers, custom keycodes, layers
    config.h    -- Vial UID/unlock combo, dynamic layer count, MIDI_ADVANCED
    rules.mk    -- feature flags (MIDI_ENABLE, VIAL_ENABLE) and debounce type
    vial.json   -- physical layout geometry for the Vial GUI (copied from
                   the stock silakka54 vial keymap, unmodified)
```

## Building

```bash
git clone https://github.com/RAvenGEr/vial-qmk-silakka54.git
cd vial-qmk-silakka54
git submodule update --init --depth 1 lib/pico-sdk lib/chibios lib/chibios-contrib lib/lufa lib/printf

cp -r /path/to/ripuli54/keyboards/silakka54/keymaps/midi keyboards/silakka54/keymaps/

qmk compile -kb silakka54 -km midi
```

This produces `.build/silakka54_midi.uf2`.

## Flashing

Each half of the split keyboard is flashed independently, over whichever half
is plugged into USB (there's no hardware-defined handedness on this board --
whichever half is connected becomes "left" in the layout):

1. Put that half into bootloader mode. An `RPI-RP2` mass-storage drive
   should appear.
2. Copy the `.uf2` file onto it: `cp .build/silakka54_midi.uf2 /media/<you>/RPI-RP2/`
3. It unmounts itself once the flash completes.
4. Repeat for the other half.

After flashing a structural change (a layer added/removed, or anything that
shifts layer indices), press `EE_CLR` once on the keyboard -- Vial's dynamic
keymap lives in EEPROM and needs to resync with the new compiled layout.

## Playing it

The keyboard sends raw MIDI over USB; you need a synth listening on the other
end. [fluidsynth](https://www.fluidsynth.org/) works reliably (unlike some
DAWs' soundfont players, which can smear together rapid same-note retriggers
like double-bass drum rolls):

```bash
fluidsynth -s -i -a pulseaudio -m alsa_seq -o synth.polyphony=256 \
    /usr/share/sounds/sf2/FluidR3_GM.sf2 < /dev/null > /tmp/fluidsynth.log 2>&1 &
```

Then connect the keyboard's MIDI output to it (needed again after every
reflash or fluidsynth restart, since the ALSA client re-enumerates):

```bash
aconnect silakka54 "FLUID Synth"
```

To boost fluidsynth's own volume live without restarting it (which would
drop the connection above):

```bash
printf 'gain 3\n' | nc localhost 9800
```

## Layers

- **Base**: normal typing layer.
- **Fn**: nav/F-keys; also holds `EE_CLR`, `QK_BOOT`, and toggles into Piano/Drum.
- **Piano**: 3x6 grid per hand, each key a *signed scale degree* relative to
  one shared root (`PIANO_KEY_DEGREES[]` in `keymap.c`), not a fixed note --
  degree 0 is the root, negative goes below it. The top row picks the root
  directly (A-G#, no hold needed); holding the right-thumb key turns that
  same row into a scale picker instead (major/minor/harmonic minor/melodic
  minor/major & minor pentatonic/chromatic).
- **Drum**: a General MIDI kit across channel 10, with a choke key to kill
  ringing cymbals on demand.

See the comments in `keymap.c` for the exact per-key layout -- it's been
reshuffled a few times and is the source of truth.

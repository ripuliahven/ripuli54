VIA_ENABLE = yes
VIAL_ENABLE = yes
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
MIDI_ENABLE = yes

# Default (sym_defer_g) debounces the whole half's matrix as one unit, so fast
# repeated hits on two keys sharing a half (e.g. stacked drum pads) can get
# merged or dropped. Per-key debounce avoids that. Presses are eager (sent
# instantly, for drumming), but releases are deferred until the key has been
# stable for DEBOUNCE ms, so a bouncy switch can't release and re-press.
DEBOUNCE_TYPE = asym_eager_defer_pk

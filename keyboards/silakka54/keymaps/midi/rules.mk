VIA_ENABLE = yes
VIAL_ENABLE = yes
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
MIDI_ENABLE = yes

# Default (sym_defer_g) debounces the whole half's matrix as one unit, so fast
# repeated hits on two keys sharing a half (e.g. stacked drum pads) can get
# merged or dropped. Per-key eager debounce reports each key immediately and
# only filters bounce on that specific key, which matters for fast drumming.
DEBOUNCE_TYPE = sym_eager_pk

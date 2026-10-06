# Regenerate secrets.h from the environment on every build invocation.
KEYMAP_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
$(shell $(KEYMAP_DIR)/gen_secrets.sh)

VIA_ENABLE = no
TAP_DANCE_ENABLE = yes
ENCODER_MAP_ENABLE = yes
RGB_MATRIX_ENABLE = no
RGBLIGHT_ENABLE = no
LTO_ENABLE = yes

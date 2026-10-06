// Copyright 2022 Sergey Vlasov (@sigprof)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "macros_config.h"
#include "macros.h"

// Keymap layout matrix
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    MACROPAD_LAYOUTS
};

#if defined(ENCODER_MAP_ENABLE)
// Encoder mapping per layer
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    ENCODER_LAYOUTS
};
#endif

// Record handler for custom macros
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_custom_macros(keycode, record)) {
        return false;
    }
    return true;
}

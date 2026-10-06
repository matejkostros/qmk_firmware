#pragma once

/*
 * ============================================================================
 * WINRY315 USER CONFIGURATION
 * Single source of truth for:
 *   1. Custom Macro Payloads
 *   2. Tap Dance Actions & Comprehensive Reference
 *   3. Layers & Physical Layouts
 *   4. Rotary Encoders
 * ============================================================================
 */

// ----------------------------------------------------------------------------
// 1. MACROS (String payloads stored in Flash ROM)
// Format: M_DEF(NAME, "keystroke payload")
//
// Special characters:
//   - Enter:      \n
//   - Tab:        \t
//   - Quotes:     \"
//   - Backslash:  \\ (double backslash)
// Modifier chords:
//   - Ctrl + key: SS_LCTRL("c")
//   - Alt + key:  SS_LALT("x")
//   - Win + key:  SS_LGUI("r")
// Non-printable:
//   - Escape:     SS_TAP(X_ESCAPE)
//   - Function:   SS_TAP(X_F1) through SS_TAP(X_F12)
// Delays:
//   - Pause:      SS_DELAY(100)
//
// NOTE:
//   - M_PW01: Dynamically generates a fresh 18-char password on each press.
//   - M_PW02: Pastes the last generated password from PW01 (persisted in EEPROM).
// ----------------------------------------------------------------------------
// Static passwords come from the environment at build time (WINRY_PW03/04).
// If unset, these expand to "" and the keys send nothing.
#define MACRO_LIST \
    /* Static fallback passwords for Row 3 on _DEV */ \
    M_DEF(M_PW03, USER_PW03) \
    M_DEF(M_PW04, USER_PW04)

// ----------------------------------------------------------------------------
// 2. TAP DANCE REFERENCE & CONFIGURATION
//
// QMK Tap Dance supports 5 distinct operational patterns:
//
// 1. TWO-WAY STRINGS: TD_STRING(NAME, "single tap", "double tap")
//    - Sends first string on 1 tap, second string on 2 taps.
//
// 2. TWO-WAY KEYCODES: TD_KEY(NAME, kc_single_tap, kc_double_tap)
//    - Sends first keycode on 1 tap, second keycode on 2 taps.
//
// 3. LAYER TOGGLE: ACTION_TAP_DANCE_LAYER_TOGGLE(kc, layer)
//    - Single tap sends keycode; double tap toggles layer on/off.
//
// 4. LAYER MOVE: ACTION_TAP_DANCE_LAYER_MOVE(kc, layer)
//    - Single tap sends keycode; double tap permanently switches to layer.
//
// 5. ADVANCED 5-WAY STATE MACHINE (ACTION_TAP_DANCE_FN_ADVANCED):
//    By implementing finished/reset callbacks, a single key can differentiate:
//      - SINGLE_TAP  : Tap key once.
//      - SINGLE_HOLD : Hold key down without releasing (e.g. acts as a modifier).
//      - DOUBLE_TAP  : Tap key twice.
//      - DOUBLE_HOLD : Tap once, then hold down (e.g. repeated fire).
//      - TRIPLE_TAP  : Tap key 3 times (e.g. emergency escape or nuclear reset).
//
//    State Machine Recipe:
//      enum { SINGLE_TAP, SINGLE_HOLD, DOUBLE_TAP, DOUBLE_HOLD, TRIPLE_TAP };
//      uint8_t cur_dance(tap_dance_state_t *state) {
//          if (state->count == 1) return state->interrupted || !state->pressed ? SINGLE_TAP : SINGLE_HOLD;
//          if (state->count == 2) return state->interrupted || !state->pressed ? DOUBLE_TAP : DOUBLE_HOLD;
//          return TRIPLE_TAP;
//      }
// ----------------------------------------------------------------------------
#define TAP_DANCE_LIST \
    /* Row 1 - Position 1: Status & Overview */ \
    TD_STRING(TD_GIT_STATUS, "git s\n", "git ss\n") \
    \
    /* Row 1 - Position 2: Log & History */ \
    TD_STRING(TD_GIT_LOG,    "git log --oneline -20\n", "git graph-ocean\n") \
    \
    /* Row 1 - Position 3: Rebase */ \
    TD_STRING(TD_GIT_SYNC,   "git pull --rebase --autostash\n", "git fetch origin && git rebase origin/master\n") \
    \
    /* Row 1 - Position 4: Search (places cursor between quotes via left arrow) */ \
    TD_STRING(TD_GIT_SEARCH, "git log -i -S \"\"" SS_TAP(X_LEFT), "git search \"\"" SS_TAP(X_LEFT)) \
    \
    /* Row 1 - Position 5: Soft/Hard Reset (no Enter for safety) */ \
    TD_STRING(TD_GIT_RESET,  "git reset --soft HEAD~1", "git reset --hard @{u}") \
    \
    /* Row 3 - Position 1: Esc / CapsLock toggle */ \
    TD_KEY(TD_ESC_CAPS,      KC_ESC, KC_CAPS) \
    \
    /* Screenshot Tap Dance (Alternative single-key mapping) */ \
    TD_KEY(TD_SCREENSHOT,    G(KC_PSCR), G(S(KC_PSCR)))

// ----------------------------------------------------------------------------
// 3. LAYERS
// ----------------------------------------------------------------------------
enum custom_layers {
    _BASE = 0,
    _DEV,
};

// ----------------------------------------------------------------------------
// 4. PHYSICAL LAYOUT (3 Knobs + 3x5 Keys)
// ----------------------------------------------------------------------------
#define MACROPAD_LAYOUTS \
    [_BASE] = LAYOUT_top( \
        /* Knob Presses:  Left              Center            Right */ \
                          KC_F15,      KC_MUTE,          KC_MPLY, \
        \
        /* Row 1 (Git) */ TD(TD_GIT_STATUS),TD(TD_GIT_LOG),   TD(TD_GIT_SYNC),  TD(TD_GIT_SEARCH),TD(TD_GIT_RESET), \
        \
        /* Row 2 (Linux)  Prev Panel        New Kitty Panel   Lock System       Rename Kitty      Next Panel */ \
                          C(S(KC_TAB)),     C(A(KC_T)),       G(KC_L),          C(A(S(KC_T))),    C(KC_TAB), \
        \
        /* Row 3          Esc / CapsLock    Meta+Print(Window)Meta+Shift+Print  Space             Hold for _DEV */ \
                          TD(TD_ESC_CAPS),  G(KC_PSCR),       G(S(KC_PSCR)),    KC_SPC,           MO(_DEV) \
    ), \
    \
    [_DEV] = LAYOUT_top( \
        /* Knob Presses:  Left              Center            Right (transparent to _BASE) */ \
                          KC_TRNS,          KC_TRNS,          KC_TRNS, \
        \
        /* Row 1 (Transparent to Row 1 of _BASE) */ \
                          KC_TRNS,          KC_TRNS,          KC_TRNS,          KC_TRNS,          KC_TRNS, \
        \
        /* Row 2 (Function Keys F1 - F5) */ \
                          KC_F1,            KC_F2,            KC_F3,            KC_F4,            KC_F5, \
        \
        /* Row 3          PW01 (Generate)   PW02 (Paste Last) PW03 (Static)     PW04 (Static)     Layer Return */ \
                          M_PW01,           M_PW02,           M_PW03,           M_PW04,           KC_TRNS \
    )

// ----------------------------------------------------------------------------
// 5. ROTARY ENCODERS (Per Layer)
// Format: ENCODER_CCW_CW(ccw_keycode, cw_keycode)
// Knobs:  [0] = Left Knob, [1] = Center Knob, [2] = Right Knob
// ----------------------------------------------------------------------------
#define ENCODER_LAYOUTS \
    [_BASE] = { \
        ENCODER_CCW_CW(KC_F13, KC_F14),         /* Left Knob: CCW = F13, CW = F14 */ \
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU),       /* Center Knob: Volume Down / Up */ \
        ENCODER_CCW_CW(KC_MPRV, KC_MNXT)        /* Right Knob: Prev / Next Track */ \
    }, \
    [_DEV] = { \
        ENCODER_CCW_CW(KC_TRNS, KC_TRNS),       /* Left Knob: Inherit from _BASE */ \
        ENCODER_CCW_CW(KC_TRNS, KC_TRNS),       /* Center Knob: Inherit from _BASE */ \
        ENCODER_CCW_CW(KC_TRNS, KC_TRNS)        /* Right Knob: Inherit from _BASE */ \
    }

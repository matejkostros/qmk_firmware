#pragma once

#include QMK_KEYBOARD_H
#include <avr/eeprom.h>
#include <string.h>

// Secrets are injected at build time from environment variables (see
// keymaps/default/gen_secrets.sh). Missing header or unset vars => empty strings.
#if defined(__has_include)
#  if __has_include("secrets.h")
#    include "secrets.h"
#  endif
#endif
#ifndef USER_PW03
#  define USER_PW03 ""
#endif
#ifndef USER_PW04
#  define USER_PW04 ""
#endif
#ifndef USER_PW_FALLBACK
#  define USER_PW_FALLBACK ""
#endif

// ============================================================================
// 1. Dynamic Password Generator & Last-Password Replay Buffer
// ============================================================================
// Stored in the final 24 bytes of ATmega32U4 1KB EEPROM:
//   Offset 1000 - 1019: Last generated password (19 bytes: 18 chars + null)
//   Offset 1020 - 1023: PRNG entropy seed (4 bytes)
#define EEPROM_LAST_PW_ADDR ((void *)(1000))
#define EEPROM_SEED_ADDR    ((uint32_t *)(1020))

static uint32_t persistent_prng_state = 0;
static bool     prng_initialized      = false;
static char     last_password[19]     = "";
static bool     last_pw_loaded        = false;

static inline uint32_t step_prng(void) {
    persistent_prng_state ^= persistent_prng_state << 13;
    persistent_prng_state ^= persistent_prng_state >> 17;
    persistent_prng_state ^= persistent_prng_state << 5;
    persistent_prng_state = persistent_prng_state * 1664525UL + 1013904223UL;
    return persistent_prng_state;
}

static void send_dynamic_password(void) {
    // 1. Load persistent seed on first invocation
    if (!prng_initialized) {
        persistent_prng_state = eeprom_read_dword(EEPROM_SEED_ADDR);
        if (persistent_prng_state == 0xFFFFFFFF || persistent_prng_state == 0) {
            persistent_prng_state = 0x8A5B3C4D;
        }
        prng_initialized = true;
    }

    // 2. Mix physical press jitter (16MHz counter TCNT1 + microsecond timer32)
    uint32_t press_jitter = ((uint32_t)timer_read32() << 16) ^ TCNT1;
    persistent_prng_state ^= press_jitter;
    step_prng();

    // 3. Pools
    static const char lower[]  = "abcdefghijklmnopqrstuvwxyz";
    static const char upper[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char digits[] = "0123456789";
    static const char syms[]   = "!@#$%^&*-_=+?";
    static const char all[]    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*-_=+?";

    char pwd[19];
    // Guarantee at least 1 of each character class
    pwd[0] = lower[step_prng() % 26];
    pwd[1] = upper[step_prng() % 26];
    pwd[2] = digits[step_prng() % 10];
    pwd[3] = syms[step_prng() % 13];

    // Fill remaining 14 characters
    for (uint8_t i = 4; i < 18; i++) {
        pwd[i] = all[step_prng() % (sizeof(all) - 1)];
    }

    // Shuffle characters (Fisher-Yates) using ratcheted state
    for (uint8_t i = 17; i > 0; i--) {
        uint8_t j = step_prng() % (i + 1);
        char tmp = pwd[i];
        pwd[i] = pwd[j];
        pwd[j] = tmp;
    }

    // Ensure no consecutive identical characters
    for (uint8_t i = 1; i < 18; i++) {
        if (pwd[i] == pwd[i - 1]) {
            pwd[i] = all[step_prng() % (sizeof(all) - 1)];
            if (pwd[i] == pwd[i - 1]) {
                pwd[i] = (pwd[i] == 'X') ? 'Y' : 'X';
            }
        }
    }
    pwd[18] = '\0';

    // 4. Save seed & copy into last-password buffer (RAM + EEPROM)
    eeprom_update_dword(EEPROM_SEED_ADDR, persistent_prng_state);

    memcpy(last_password, pwd, sizeof(pwd));
    last_pw_loaded = true;
    eeprom_update_block(pwd, EEPROM_LAST_PW_ADDR, 19);

    send_string(pwd);
}

static void send_last_password(void) {
    if (!last_pw_loaded) {
        eeprom_read_block(last_password, EEPROM_LAST_PW_ADDR, 19);
        last_password[18] = '\0';
        // Fallback if virgin/blank EEPROM (empty unless WINRY_PW_FALLBACK is set)
        if ((uint8_t)last_password[0] == 0xFF || last_password[0] == '\0') {
            strncpy(last_password, USER_PW_FALLBACK, sizeof(last_password) - 1);
            last_password[sizeof(last_password) - 1] = '\0';
        }
        last_pw_loaded = true;
    }
    if (last_password[0] == '\0') {
        return;
    }
    send_string(last_password);
}

// ============================================================================
// 2. Enums for Custom Macro Keycodes
// ============================================================================
enum custom_macro_keycodes {
    CUSTOM_MACROS_START = QK_USER_0,
    M_CTRL_MUTE,
    M_PW_DYN,
    M_PW_REPLAY,
#define M_DEF(name, str) name,
    MACRO_LIST
#undef M_DEF
};

#define M_PW01 M_PW_DYN
#define M_PW02 M_PW_REPLAY

// ============================================================================
// 3. Tap Dance Engine & Code Generation
// ============================================================================
#if defined(TAP_DANCE_ENABLE) && defined(TAP_DANCE_LIST)

enum custom_tap_dance_indexes {
#define TD_STRING(name, s1, s2) name,
#define TD_KEY(name, k1, k2)    name,
    TAP_DANCE_LIST
#undef TD_STRING
#undef TD_KEY
    TD_COUNT
};

#define TD_STRING(name, s1, s2)                                            \
    static void td_fn_##name(tap_dance_state_t *state, void *user_data) { \
        if (state->count == 1) {                                           \
            send_string_P(PSTR(s1));                                       \
        } else if (state->count >= 2) {                                    \
            send_string_P(PSTR(s2));                                       \
        }                                                                  \
    }
#define TD_KEY(name, k1, k2)
TAP_DANCE_LIST
#undef TD_STRING
#undef TD_KEY

tap_dance_action_t tap_dance_actions[] = {
#define TD_STRING(name, s1, s2) [name] = ACTION_TAP_DANCE_FN(td_fn_##name),
#define TD_KEY(name, k1, k2)    [name] = ACTION_TAP_DANCE_DOUBLE(k1, k2),
    TAP_DANCE_LIST
#undef TD_STRING
#undef TD_KEY
};

#endif // TAP_DANCE_ENABLE

// ============================================================================
// 4. Macro Dispatch Engine
// ============================================================================
static inline bool process_custom_macros(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }

    switch (keycode) {
        case M_CTRL_MUTE:
            register_mods(MOD_BIT(KC_LCTL));
            tap_code16(KC_MUTE);
            unregister_mods(MOD_BIT(KC_LCTL));
            return false;

        case M_PW_DYN:
            send_dynamic_password();
            return false;

        case M_PW_REPLAY:
            send_last_password();
            return false;

#define M_DEF(name, str)              \
        case name: {                  \
            send_string_P(PSTR(str)); \
            return false;             \
        }
        MACRO_LIST
#undef M_DEF

        default:
            return true;
    }
}

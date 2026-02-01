#include "ir_nec.h"
#include "timebase.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// ===== PIN IR: D2 / PD2 / INT0 =====
#define IR_DDR   DDRD
#define IR_PORT  PORTD
#define IR_PINR  PIND
#define IR_BIT   PD2

// ===== Progi czasowe NEC (us) =====
#define LEAD_MARK_MIN   8000
#define LEAD_MARK_MAX  10000

#define LEAD_SPACE_MIN  3500
#define LEAD_SPACE_MAX  5500

#define REPEAT_SPACE_MIN 1750
#define REPEAT_SPACE_MAX 2750

#define BIT_MARK_MIN     350
#define BIT_MARK_MAX     750

#define ZERO_SPACE_MIN   350
#define ZERO_SPACE_MAX   750

#define ONE_SPACE_MIN   1300
#define ONE_SPACE_MAX   2100

#define GAP_RESET_US    12000

typedef enum {
    S_IDLE = 0,
    S_GOT_LEAD_MARK,
    S_EXPECT_BIT_MARK,
    S_EXPECT_BIT_SPACE,
} state_t;

static volatile state_t g_state = S_IDLE;
static volatile uint32_t g_last_ticks = 0;
static volatile uint8_t  g_last_level = 1; // TSOP: idle HIGH

static volatile uint8_t  g_bit = 0;
static volatile uint32_t g_data = 0;

static volatile bool     g_have = false;
static volatile bool     g_have_repeat = false;
static volatile uint32_t g_raw = 0;

static volatile uint16_t g_last_addr = 0;
static volatile uint8_t  g_last_cmd = 0;
static volatile bool     g_have_last = false;

static inline bool between_u32(uint32_t v, uint32_t lo, uint32_t hi) {
    return (v >= lo) && (v <= hi);
}

ISR(INT0_vect) {
    const uint32_t now = timebase_ticks();
    const uint32_t dt_ticks = now - g_last_ticks;
    g_last_ticks = now;

    const uint32_t dt_us = dt_ticks >> 1; // 0.5us / tick

    const uint8_t level_now = (IR_PINR & (1 << IR_BIT)) ? 1 : 0;
    const uint8_t prev_level = g_last_level;
    g_last_level = level_now;

    // reset stanu gdy długi gap
    if (dt_us > GAP_RESET_US) {
        g_state = S_IDLE;
        g_bit = 0;
        g_data = 0;
        return;
    }

    switch (g_state) {
        case S_IDLE:
            // czekamy na lead MARK: LOW ~9ms
            if (prev_level == 0 && between_u32(dt_us, LEAD_MARK_MIN, LEAD_MARK_MAX)) {
                g_state = S_GOT_LEAD_MARK;
            }
            break;

        case S_GOT_LEAD_MARK:
            // lead SPACE: HIGH ~4.5ms lub repeat ~2.25ms
            if (prev_level == 1 && between_u32(dt_us, LEAD_SPACE_MIN, LEAD_SPACE_MAX)) {
                g_state = S_EXPECT_BIT_MARK;
                g_bit = 0;
                g_data = 0;
            } else if (prev_level == 1 && between_u32(dt_us, REPEAT_SPACE_MIN, REPEAT_SPACE_MAX)) {
                g_have_repeat = true;
                g_state = S_IDLE;
            } else {
                g_state = S_IDLE;
            }
            break;

        case S_EXPECT_BIT_MARK:
            // bit MARK: LOW ~560us
            if (prev_level == 0 && between_u32(dt_us, BIT_MARK_MIN, BIT_MARK_MAX)) {
                g_state = S_EXPECT_BIT_SPACE;
            } else {
                g_state = S_IDLE;
            }
            break;

        case S_EXPECT_BIT_SPACE:
            // bit SPACE: HIGH 560us (0) lub 1690us (1)
            if (prev_level == 1) {
                uint8_t bitval;
                if (between_u32(dt_us, ZERO_SPACE_MIN, ZERO_SPACE_MAX)) bitval = 0;
                else if (between_u32(dt_us, ONE_SPACE_MIN, ONE_SPACE_MAX)) bitval = 1;
                else {
                    g_state = S_IDLE;
                    break;
                }

                if (bitval) g_data |= (1UL << g_bit);
                g_bit++;

                if (g_bit >= 32) {
                    g_raw = g_data;
                    g_have = true;
                    g_state = S_IDLE;
                } else {
                    g_state = S_EXPECT_BIT_MARK;
                }
            } else {
                g_state = S_IDLE;
            }
            break;
    }
}

void ir_nec_init(void) {
    // pin jako input + pull-up
    IR_DDR  &= ~(1 << IR_BIT);
    IR_PORT |=  (1 << IR_BIT);

    g_last_level = (IR_PINR & (1 << IR_BIT)) ? 1 : 0;
    g_last_ticks = timebase_ticks();

    // INT0 na dowolną zmianę
    EICRA = (1 << ISC00);
    EIMSK = (1 << INT0);
}

static nec_frame_t decode(uint32_t raw) {
    nec_frame_t f = {0};
    const uint8_t a0  = (uint8_t)(raw & 0xFF);
    const uint8_t a1  = (uint8_t)((raw >> 8) & 0xFF);
    const uint8_t cmd = (uint8_t)((raw >> 16) & 0xFF);
    const uint8_t inv = (uint8_t)((raw >> 24) & 0xFF);

    f.command = cmd;
    f.valid = ((uint8_t)(cmd ^ inv) == 0xFF);

    // klasyczny NEC: addr, ~addr; extended: 16-bit addr
    if ((uint8_t)(a0 ^ a1) == 0xFF) f.address = a0;
    else f.address = (uint16_t)a0 | ((uint16_t)a1 << 8);
    return f;
}

bool ir_nec_read(nec_frame_t *out) {
    if (!out) return false;

    bool have = false;
    bool rep = false;
    uint32_t raw = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (g_have) {
            raw = g_raw;
            g_have = false;
            have = true;
        } else if (g_have_repeat) {
            g_have_repeat = false;
            rep = true;
        }
    }

    if (have) {
        nec_frame_t f = decode(raw);
        f.repeat = false;

        if (f.valid) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                g_last_addr = f.address;
                g_last_cmd  = f.command;
                g_have_last = true;
            }
        }

        *out = f;
        return true;
    }

    if (rep) {
        nec_frame_t f = {0};
        bool have_last;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            have_last = g_have_last;
            f.address = g_last_addr;
            f.command = g_last_cmd;
        }
        if (!have_last) return false;
        f.repeat = true;
        f.valid = true;
        *out = f;
        return true;
    }

    return false;
}

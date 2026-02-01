#include "motor_control.h"

#include "pwm_control.h"
#include "timebase.h"

// 1 tick = 0.5us (Timer0 preskaler 8)
#define TICKS_PER_MS 2000u

typedef struct {
    int8_t current; // -100..+100
    int8_t target;  // -100..+100
} motor_side_t;

static motor_side_t g_left = {0, 0};
static motor_side_t g_right = {0, 0};

static uint32_t g_last_step_ticks = 0;
static uint32_t g_last_cmd_ticks = 0;

static inline uint8_t abs8(int8_t v) {
    return (uint8_t)(v < 0 ? -v : v);
}

static inline int8_t sign8(int8_t v) {
    return (v > 0) - (v < 0);
}

static inline int8_t clamp100(int16_t v) {
    if (v > 100) return 100;
    if (v < -100) return -100;
    return (int8_t)v;
}

static void apply_side_left(int8_t value) {
    if (MOTOR_LEFT_INVERT) value = -value;
    // Bezpieczne ustawienie: najpierw oba kanały 0, potem jeden
    pwm_set_left_fwd(0);
    pwm_set_left_rev(0);
    if (value > 0) {
        pwm_set_left_fwd(abs8(value));
    } else if (value < 0) {
        pwm_set_left_rev(abs8(value));
    }
}

static void apply_side_right(int8_t value) {
    if (MOTOR_RIGHT_INVERT) value = -value;
    pwm_set_right_fwd(0);
    pwm_set_right_rev(0);
    if (value > 0) {
        pwm_set_right_fwd(abs8(value));
    } else if (value < 0) {
        pwm_set_right_rev(abs8(value));
    }
}

static int8_t ramp_step(int8_t current, int8_t target) {
    // Zasada: jeżeli zmieniamy znak (przód<->tył), to najpierw dojedź do 0,
    // dopiero potem w drugą stronę.
    if (current == target) return current;

    const int8_t s_cur = sign8(current);
    const int8_t s_tgt = sign8(target);

    // Jeśli jest zmiana kierunku i jeszcze nie jesteśmy na 0 -> schodzimy do 0.
    if (s_cur != 0 && s_tgt != 0 && s_cur != s_tgt) {
        target = 0;
    }

    int16_t diff = (int16_t)target - (int16_t)current;
    if (diff > (int16_t)MOTOR_RAMP_STEP_PERCENT) diff = MOTOR_RAMP_STEP_PERCENT;
    if (diff < -(int16_t)MOTOR_RAMP_STEP_PERCENT) diff = -(int16_t)MOTOR_RAMP_STEP_PERCENT;
    return clamp100((int16_t)current + diff);
}

void motor_init(void) {
    g_left.current = g_left.target = 0;
    g_right.current = g_right.target = 0;
    g_last_step_ticks = timebase_ticks();
    g_last_cmd_ticks = g_last_step_ticks;

    // Start: wszystko wyłączone (coast)
    apply_side_left(0);
    apply_side_right(0);
}

void motor_set_target(int8_t left_percent, int8_t right_percent) {
    g_left.target = clamp100(left_percent);
    g_right.target = clamp100(right_percent);
}

void motor_stop(void) {
    g_left.target = 0;
    g_right.target = 0;
    g_left.current = 0;
    g_right.current = 0;
    apply_side_left(0);
    apply_side_right(0);
}

void motor_kick_watchdog(uint32_t now_ticks) {
    g_last_cmd_ticks = now_ticks;
}

void motor_update(void) {
    uint32_t now = timebase_ticks();

    // Timeout na brak komendy -> stop
    const uint32_t timeout_ticks = (uint32_t)MOTOR_TIMEOUT_MS * (uint32_t)TICKS_PER_MS;
    if ((uint32_t)(now - g_last_cmd_ticks) > timeout_ticks) {
        g_left.target = 0;
        g_right.target = 0;
    }

    // Ramp co MOTOR_RAMP_INTERVAL_MS
    const uint32_t interval_ticks = (uint32_t)MOTOR_RAMP_INTERVAL_MS * (uint32_t)TICKS_PER_MS;
    if ((uint32_t)(now - g_last_step_ticks) < interval_ticks) {
        return;
    }
    g_last_step_ticks = now;

    int8_t new_left = ramp_step(g_left.current, g_left.target);
    int8_t new_right = ramp_step(g_right.current, g_right.target);

    if (new_left != g_left.current) {
        g_left.current = new_left;
        apply_side_left(g_left.current);
    }
    if (new_right != g_right.current) {
        g_right.current = new_right;
        apply_side_right(g_right.current);
    }
}

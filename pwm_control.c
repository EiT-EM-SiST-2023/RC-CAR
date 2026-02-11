#include <avr/io.h>
#include "pwm_control.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/*
 * PWM dla autka RC:
 *  - Lewa strona:  Timer1
 *      * D9  (PB1 / OC1A)  -> LEFT_FWD
 *      * D10 (PB2 / OC1B)  -> LEFT_REV
 *    Tryb: Fast PWM, TOP=ICR1 (Mode 14), preskaler=1, ~15kHz
 *
 *  - Prawa strona: Timer2
 *      * D3  (PD3 / OC2B)  -> RIGHT_FWD
 *      * D11 (PB3 / OC2A)  -> RIGHT_REV
 *    Tryb: Phase Correct PWM, TOP=0xFF, preskaler=1, ~31.37kHz
 */

// ===== Timer1 ~15kHz  =====
#define T1_TOP 1066u // Jeśli chcesz też 31kHz w Timer1 to T1_TOP = 511 (ale wtedy gorsza rozdzielczość) Bazowo 16MHz / 1066 ≈ 15kHz

static inline uint16_t percent_to_t1(uint8_t percent) {
    if (percent > 100) percent = 100;
    return (uint16_t)(((uint32_t)percent * T1_TOP) / 100u);
}

// ===== Timer2 ~31kHz (8-bit) =====
static inline uint8_t percent_to_t2(uint8_t percent) {
    if (percent > 100) percent = 100;
    // 0..255
    return (uint8_t)(((uint16_t)percent * 255u) / 100u);
}

void pwm_init(void) {
    // ===== Timer1: OC1A(D9) + OC1B(D10) =====
    DDRB |= (1 << PB1) | (1 << PB2);

    // Mode 14: Fast PWM, TOP=ICR1
    // Non-inverting on OC1A/OC1B
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // preskaler=1

    ICR1 = T1_TOP;
    OCR1A = 0;
    OCR1B = 0;

    // ===== Timer2: OC2B(D3) + OC2A(D11) =====
    // D3 = PD3 (OC2B), D11 = PB3 (OC2A)
    DDRD |= (1 << PD3);
    DDRB |= (1 << PB3) | (1 << PB2) | (1 << PB3);

    // Phase Correct PWM, TOP=0xFF: WGM22:0 = 0b001
    // Non-inverting on OC2A/OC2B
    TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM20);
    // preskaler=1
    TCCR2B = (1 << CS20);

    OCR2A = 0;
    OCR2B = 0;
}

// ===== Settery kanałów =====
void pwm_set_left_fwd(uint8_t percent) {
    OCR1A = percent_to_t1(percent);
}

void pwm_set_left_rev(uint8_t percent) {
    OCR1B = percent_to_t1(percent);
}

void pwm_set_right_rev(uint8_t percent) {
    // OC2A = D11
    OCR2A = percent_to_t2(percent);
}

void pwm_set_right_fwd(uint8_t percent) {
    // OC2B = D3
    OCR2B = percent_to_t2(percent);
}

void set_motor_power(uint8_t percent) {
    // Stare demo: oba silniki w przód.
    if (percent > 100) percent = 100;
    pwm_set_left_rev(0);
    pwm_set_right_rev(0);
    pwm_set_left_fwd(percent);
    pwm_set_right_fwd(percent);
}

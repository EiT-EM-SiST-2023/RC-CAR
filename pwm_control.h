#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h>

// Inicjalizacja PWM:
//  - Timer1: OC1A (Arduino D9) i OC1B (Arduino D10) ~15kHz
//  - Timer2: OC2B (Arduino D3) i OC2A (Arduino D11) ~31kHz (phase-correct)
void pwm_init(void);

// Ustawienia w procentach (0..100) dla poszczególnych wejść mostków.
// UWAGA: dla jednej strony auta NIE ustawiaj naraz FWD i REV > 0.
void pwm_set_left_fwd(uint8_t percent);
void pwm_set_left_rev(uint8_t percent);
void pwm_set_right_fwd(uint8_t percent);
void pwm_set_right_rev(uint8_t percent);

// Kompatybilność ze starym demo: ustawia oba silniki "do przodu".
// (Lewy FWD i Prawy FWD), a kanały REV zeruje.
void set_motor_power(uint8_t percent);

#endif

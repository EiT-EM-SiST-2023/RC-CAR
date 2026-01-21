#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h> // Potrzebne, żeby używać typów uint8_t

// Funkcja inicjalizująca Timer 1 pod PWM 15kHz
void pwm_init(void);

// Funkcja ustawiająca moc silników (0 - 100%)
void set_motor_power(uint8_t percent);

#endif
#include <avr/io.h>
#include "pwm_control.h"

/*
 * KONFIGURACJA DLA 15 kHz
 * Procesor: ATmega328P @ 16 MHz
 *
 * Wzór na częstotliwość w trybie Fast PWM (z ICR1 jako TOP):
 * f_PWM = f_CPU / (Preskaler * (TOP + 1))
 *
 * Podstawiając dane:
 * 15 000 = 16 000 000 / (1 * (TOP + 1))
 * TOP + 1 = 1066.666...
 * TOP = 1066
 */
#define PWM_TOP 1066

void pwm_init(void) {
    // 1. Ustawienie pinu PB1 (Pin 9 w Arduino) jako wyjście
    // To jest fizyczny pin, z którego wyjdzie sygnał PWM na mostek
    DDRB |= (1 << PB1);

    // 2. Konfiguracja Timera 1
    // Używamy trybu "Fast PWM" (Mode 14), który pozwala ustawić
    // dokładną częstotliwość za pomocą rejestru ICR1.
    
    // WGM13:0 = 1110 -> Tryb 14 (Fast PWM, TOP = ICR1)
    // COM1A1:0 = 10   -> Clear OC1A on Compare Match (nieodwrócony)
    // CS12:0 = 001    -> Preskaler = 1 (brak dzielnika zegara)

    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

    // 3. Wpisanie wartości TOP (okres sygnału dla 15kHz)
    ICR1 = PWM_TOP;

    // 4. Na start ustawiamy wypełnienie 0% (silniki stoją)
    OCR1A = 0;
}

void set_motor_power(uint8_t percent) {
    // Zabezpieczenie: nie możemy przekroczyć 100%
    if (percent > 100) percent = 100;

    // Matematyka: Zamiana procentów (0-100) na zakres Timera (0-1066)
    // Używamy rzutowania na (uint32_t), żeby uniknąć przepełnienia podczas mnożenia
    uint16_t pwm_value = ((uint32_t)percent * PWM_TOP) / 100;

    // Wpisanie wyniku do rejestru porównania - to zmienia szerokość impulsu
    OCR1A = pwm_value;
}
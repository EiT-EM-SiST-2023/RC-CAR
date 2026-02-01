#pragma once
#include <stdint.h>

// Timebase na Timer0: tick = 0.5 us (F_CPU=16MHz, prescaler=8)
// Uwaga: jeśli kiedyś przejdziecie na Arduino core, Timer0 jest używany przez micros()/millis()/delay().
// W tym repo (avr-gcc, bez Arduino) Timer0 jest wolny i można go użyć jako zegar do NEC.

void timebase_init(void);
uint32_t timebase_ticks(void); // ticki po 0.5 us
static inline uint32_t time_us(void) { return timebase_ticks() >> 1; }

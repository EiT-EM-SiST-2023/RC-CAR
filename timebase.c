#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "timebase.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

static volatile uint32_t g_ticks = 0; // rośnie o 256 na overflow (8-bit)

ISR(TIMER0_OVF_vect) {
    g_ticks += 256;
}

void timebase_init(void) {
    // Timer0 normal mode
    TCCR0A = 0;

    // preskaler=8 => 16MHz/8 = 2MHz => 0.5us na tick
    TCCR0B = (1 << CS01);

    // overflow interrupt enable
    TIMSK0 = (1 << TOIE0);
}

uint32_t timebase_ticks(void) {
    uint32_t base;
    uint8_t tcnt;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        base = g_ticks;
        tcnt = TCNT0;

        // Jeśli overflow zaszedł, a ISR jeszcze nie wykonał:
        if (TIFR0 & (1 << TOV0)) {
            base += 256;
            tcnt = TCNT0;
        }
    }
    return base + tcnt;
}

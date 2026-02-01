#pragma once

#include <stdint.h>
#include <stdbool.h>

// Prosty dekoder NEC (dla popularnych odbiorników TSOP itp.)
// - wejście: D2 / PD2 / INT0
// - nie dotyka Timer1 (możesz mieć PWM na D9/D10)
// - do pomiaru czasu używa Timer2 (timebase.c)

typedef struct {
    uint16_t address;   // 8-bit w dolnym bajcie albo 16-bit (extended)
    uint8_t  command;
    bool     repeat;    // ramka powtórzeniowa (przytrzymany przycisk)
    bool     valid;     // przeszła walidację (cmd ^ ~cmd)
} nec_frame_t;

void ir_nec_init(void);

// Zwraca true gdy jest nowa ramka (albo repeat). Wynik w *out.
bool ir_nec_read(nec_frame_t *out);

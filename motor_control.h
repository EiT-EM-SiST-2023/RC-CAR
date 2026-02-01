#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
/*
Rola motor_control
przyjmować docelowe prędkości lewej i prawej strony (target)
dochodzić do nich łagodnie (rampowanie), żeby nie było pików prądu
bezpiecznie zmieniać kierunek (zawsze przez 0) funkcja ramp_step
nigdy nie podać PWM na FWD i REV jednocześnie na tej samej stronie apply_side_left i right
zatrzymać auto, jeśli nie ma komend z pilota (timeout) motor_update()
*/

// Konfiguracja zachowania (Czyli co ile ms zwiększam moc o ileś procęt)
#define MOTOR_RAMP_STEP_PERCENT   5u   // o ile % zmieniamy przy jednym kroku (chodzi o soft start żeby nie było tego spike prądu)
#define MOTOR_RAMP_INTERVAL_MS    10u  // co ile ms wykonujemy krok rampy (chodzi o soft start żeby nie było tego spike prądu)
#define MOTOR_TIMEOUT_MS          250u // po ilu ms bez komendy -> stop (coast) (żeby auto nie odjechało wpizdu jak zgubi pilota)

// Jeśli po podłączeniu okaże się, że "przód" jedzie w tył, ustaw na 1.
#define MOTOR_LEFT_INVERT         0u
#define MOTOR_RIGHT_INVERT        0u

// Inicjalizacja warstwy sterowania silnikami
void motor_init(void);

// Ustaw docelową prędkość dla lewej i prawej strony.
// Zakres: -100..+100
//  + = przód, - = tył, 0 = stop (coast)
void motor_set_target(int8_t left_percent, int8_t right_percent);

// Natychmiastowe zatrzymanie (coast) + wyzerowanie targetów
void motor_stop(void);

// Wywołuj często w pętli głównej (robi rampowanie i bezpieczną zmianę kierunku)
void motor_update(void);

// Zapisz "czas ostatniej komendy" (do timeoutu) w tickach timebase
void motor_kick_watchdog(uint32_t now_ticks);

#endif

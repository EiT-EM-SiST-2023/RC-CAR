#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm_control.h"
#include "timebase.h"
#include "ir_nec.h"
#include "uart.h"
#include "motor_control.h"

// ====== PIN LED (Arduino D13) ======
#define LED_DDR  DDRB
#define LED_PORT PORTB
#define LED_BIT  PB5

// ====== KODY PILOTA (wpiszesz po podglądzie w UART) ======
#define CMD_FORWARD   0x00
#define CMD_BACKWARD  0x00
#define CMD_LEFT      0x00
#define CMD_RIGHT     0x00
#define CMD_STOP      0x00

// Prędkość jazdy w procentach (0..100)
#define SPEED_PERCENT  60

static inline void led_init(void) {
    LED_DDR |= (1 << LED_BIT);
    LED_PORT &= ~(1 << LED_BIT);
}

static inline void led_toggle(void) {
    LED_PORT ^= (1 << LED_BIT);
}

int main(void) {
    cli();

    led_init();
    uart_init(115200);

    // Zegar dla dekodera IR (Timer0) - 1 tick = 0.5us
    timebase_init();
    // PWM (Timer1 + Timer2)
    pwm_init();
    // Logika sterowania silnikami (rampowanie + watchdog)
    motor_init();
    // IR NEC (INT0 na D2)
    ir_nec_init();

    sei();

    uart_print("RC-CAR: start\r\n");
    uart_print("IR on D2(INT0). Open serial 115200\r\n");

    // Na start stop
    motor_stop();

    while (1) {
        // Aktualizuj rampę i timeout na brak pilota
        motor_update();

        nec_frame_t f;
        if (ir_nec_read(&f) && f.valid) {
            // migaj LED na każdą ramkę (żeby było widać, że odbiera)
            led_toggle();

            uart_print("IR addr=0x");
            uart_print_hex16(f.address);
            uart_print(" cmd=0x");
            uart_print_hex8(f.command);
            if (f.repeat) uart_print(" R");
            uart_print("\r\n");

            // Każda poprawna ramka odświeża watchdog (żeby nie stanęło w trakcie trzymania)
            motor_kick_watchdog(timebase_ticks());

            // Mapowanie komend -> target dla lewej/prawej strony
            if (f.command == CMD_FORWARD) {
                motor_set_target(+SPEED_PERCENT, +SPEED_PERCENT);
            } else if (f.command == CMD_BACKWARD) {
                motor_set_target(-SPEED_PERCENT, -SPEED_PERCENT);
            } else if (f.command == CMD_LEFT) {
                // obrót w miejscu
                motor_set_target(-SPEED_PERCENT, +SPEED_PERCENT);
            } else if (f.command == CMD_RIGHT) {
                motor_set_target(+SPEED_PERCENT, -SPEED_PERCENT);
            } else if (f.command == CMD_STOP) {
                motor_set_target(0, 0);
            }
        }
    }
}
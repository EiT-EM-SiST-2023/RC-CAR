#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "uart.h"

#include <avr/io.h>

void uart_init(uint32_t baud) {
    // UBRR = F_CPU/(16*baud) - 1
    uint16_t ubrr = (uint16_t)((F_CPU / (16UL * baud)) - 1UL);
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    // TX enable
    UCSR0B = (1 << TXEN0);
    // 8N1
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_tx(uint8_t b) {
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = b;
}

void uart_print(const char *s) {
    while (*s) uart_tx((uint8_t)*s++);
}

static uint8_t nibble_to_hex(uint8_t n) {
    n &= 0x0F;
    return (n < 10) ? (uint8_t)('0' + n) : (uint8_t)('A' + (n - 10));
}

void uart_print_hex8(uint8_t v) {
    uart_tx(nibble_to_hex(v >> 4));
    uart_tx(nibble_to_hex(v));
}

void uart_print_hex16(uint16_t v) {
    uart_print_hex8((uint8_t)(v >> 8));
    uart_print_hex8((uint8_t)(v & 0xFF));
}

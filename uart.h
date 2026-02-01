#pragma once

#include <stdint.h>

void uart_init(uint32_t baud);
void uart_tx(uint8_t b);
void uart_print(const char *s);
void uart_print_hex8(uint8_t v);
void uart_print_hex16(uint16_t v);

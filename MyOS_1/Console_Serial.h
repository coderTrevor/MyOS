#pragma once

// comment out this define if your machine lacks a serial port:
#define USE_SERIAL 1

#define PORT 0x3f8   /* COM1 */

void init_serial();

int is_transmit_empty();

char read_serial();

int serial_received();

int serial_printf(char *messageFormat, ...);

void write_serial(char a);

void write_serial_string(const char *str);

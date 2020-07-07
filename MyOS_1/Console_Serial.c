#include "Console_Serial.h"
#include "System_Specific.h"
#include "printf.h"

#ifndef USE_SERIAL
void init_serial() {}
int serial_received() { return false; }
char read_serial() { return '\0'; }
int is_transmit_empty() { return true; }
int sprintf(char *messageFormat, ...) { return 0; }
void write_serial(char a) {}
void write_serial_string(const char *str) {}
#endif

#ifdef USE_SERIAL
void init_serial() {
    outb(PORT + 1, 0x00);    // Disable all interrupts
    outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 1);       // set divisor to 1 for 115200 Set divisor to 12 (lo byte) 9600 baud
    outb(PORT + 1, 0x00);    //                  (hi byte)
    outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
    return inb(PORT + 5) & 1;
}

char read_serial() {
    while (serial_received() == 0);

    return inb(PORT);
}

int is_transmit_empty() {
    return inb(PORT + 5) & 0x20;
}

int serial_printf(char *messageFormat, ...)
{
    char buffer[128] = { 0 };
    va_list va;
    va_start(va, messageFormat);
    const int ret = vsnprintf(buffer, 128, messageFormat, va);
    
    write_serial_string(buffer);
    va_end(va);    

    return ret;
}

void write_serial(char a) {
    while (is_transmit_empty() == 0);

    outb(PORT, a);
}

void write_serial_string(const char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len; ++i)
        write_serial(str[i]);
}

#endif
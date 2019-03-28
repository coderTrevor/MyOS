#include "Console_Serial.h"
#include "System_Specific.h"

#ifndef USE_SERIAL
void init_serial() {}
/*int serial_received() {}
char read_serial() {}
int is_transmit_empty() {}*/
void write_serial(char a) {}
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

void write_serial(char a) {
    while (is_transmit_empty() == 0);

    outb(PORT, a);
}
#endif
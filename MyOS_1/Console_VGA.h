#pragma once
#include <stdint.h>
#include <stdbool.h>

extern uint16_t terminal_row;
extern uint16_t terminal_column;
extern uint8_t terminal_color;
extern uint16_t* terminal_buffer;

/* Hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return (uint8_t)(fg | bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

uint16_t get_cursor_position(void);

void VGA_terminal_backspace();

void terminal_dumpHex(uint8_t *data, size_t length);

void terminal_fill(char c, uint8_t foreground, uint8_t background);

void terminal_get_textmode_text(char *dst, uint16_t maxLength);

void VGA_terminal_initialize(void);

void VGA_terminal_putchar(char c);

void terminal_print_byte(int number);

void terminal_print_byte_hex(int value);

void terminal_print_byte_hex_leading(int value, bool leading);

void terminal_print_int(int value);

void VGA_terminal_print_int_top(int value, uint16_t column);

void terminal_print_ulong_hex(uint32_t value);

void terminal_print_ulonglong_hex(uint64_t value);

void terminal_print_ushort_hex(uint16_t value);

void VGA_terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

void terminal_resume();

void terminal_setcolor(uint8_t color);

void terminal_scroll_up();

void terminal_write(const char* data, size_t size);

void terminal_writestring(const char* data);

void VGA_terminal_writestring_top(const char* data, uint16_t column);
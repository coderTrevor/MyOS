#include <stdint.h>
#include <stdbool.h>
#include "System_Specific.h"
#include "Console_VGA.h"
#include "misc.h"
#include "Terminal.h"
#include "printf.h"

/* terminal globals */
static const uint16_t VGA_WIDTH = 80;
static const uint16_t VGA_HEIGHT = 25;

uint16_t terminal_row;
uint16_t terminal_column;
uint8_t terminal_color = (uint8_t)(VGA_COLOR_WHITE | VGA_COLOR_BLUE << 4);
uint16_t* terminal_buffer = (uint16_t*)0xB8000;

void update_cursor(uint16_t x, uint16_t y)
{
    uint16_t pos = y * VGA_WIDTH + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    return pos;
}

// Read the contents of the screen and convert it to a string
void terminal_get_textmode_text(char *dst, uint16_t maxLength)
{
    int destPos = 0;
    int srcPos = 0;
    uint16_t* source = terminal_buffer;

    // Zero out the destination buffer
    memset(dst, 0, maxLength);

    //terminal_writestring("Getting screen contents...\n");

    // Copy each row
    for (int row = 0; row < terminal_row && row <= VGA_HEIGHT && destPos < maxLength; ++row)
    {
        /*terminal_writestring("row ");
        terminal_print_int(row);
        terminal_newline();*/

        // copy each column of each row
        for (int col = 0; col < VGA_WIDTH; ++col)
        {
            if (destPos > maxLength)
                return;
            //terminal_putchar((char) ((source[currentPos]) & 0xFF) );
            dst[destPos++] = (char)(source[srcPos++] & 0xFF);
        }
        // end each row with a newline
        dst[destPos++] = '\n';
    }

    // copy the last row
    if (terminal_row >= VGA_HEIGHT)
        return;

    for (int col = 0; destPos < maxLength && col < VGA_WIDTH && col < terminal_column; ++destPos, ++col, ++srcPos)
    {
        dst[destPos] = (char)(source[srcPos] & 0xFF);
    }
}

void VGA_terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*)0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    update_cursor(0, 0);
}

// Useful for stand-alone programs that were launched by the shell, and the shell resuming from stand-alone programs
void terminal_resume()
{
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    uint16_t pos = get_cursor_position();
    terminal_row = pos / VGA_WIDTH;
    terminal_column = pos % VGA_WIDTH;
    terminal_buffer = (uint16_t*)0xB8000;
}

void terminal_dumpHex(uint8_t *data, size_t length)
{
    size_t offset = 0;
    while (offset < length)
    {
        for (int i = 0; i < 8 && offset < length; i++)
        {
            terminal_print_byte_hex(data[offset + i]);
            terminal_putchar(' ');
        }
        offset += 8;
        terminal_putchar(' ');
        for (int i = 0; i < 8 && offset < length; i++)
        {
            terminal_print_byte_hex(data[offset + i]);
            terminal_putchar(' ');
        }
        offset += 8;
        terminal_newline();
    }
    //terminal_newline();
}

// Fills the terminal up with a given character
// Intended to be used before paging is initialized, so it doesn't rely on any globals
void fill_term(char c, uint8_t foreground_color, uint8_t background_color)
{
    // assign relevant values to locals, because we can't rely on globals before paging is enabled
    const size_t LOCAL_VGA_WIDTH = 80;
    uint8_t localTermColor;
    uint16_t* localTermBuffer;

    localTermColor = vga_entry_color(foreground_color, background_color);
    localTermBuffer = (uint16_t*)0xB8000;
    for (size_t y = 0; y < LOCAL_VGA_WIDTH; y++) {
        for (size_t x = 0; x < LOCAL_VGA_WIDTH; x++) {
            const size_t index = y * LOCAL_VGA_WIDTH + x;
            localTermBuffer[index] = (uint16_t)c | (uint16_t)(localTermColor) << 8;
        }
    }
}

void terminal_fill(char c, uint8_t foreground, uint8_t background)
{
    terminal_row = 0;
    terminal_column = 0;
    uint8_t color = vga_entry_color(foreground, background);
    terminal_buffer = (uint16_t*)0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(c, color);
        }
    }
}

void VGA_terminal_backspace()
{
    if (terminal_column == 0 && terminal_row == 0)
    {
        VGA_terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        return;
    }

    if (terminal_column > 0)
    {
        --terminal_column;
        VGA_terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        update_cursor(terminal_column, terminal_row);
        return;
    }

    terminal_column = VGA_WIDTH - 1;
    --terminal_row;
    VGA_terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    update_cursor(terminal_column, terminal_row);
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void VGA_terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll_up()
{
    size_t index = 0;

    // for each row (except the last row)
    for (size_t current_row = 0; current_row < (size_t)(VGA_HEIGHT - 1); ++current_row)
    {
        // copy the next row to the current row
        for (size_t current_col = 0; current_col < VGA_WIDTH; ++current_col)
        {
            terminal_buffer[index] = terminal_buffer[index + VGA_WIDTH];
            ++index;
        }
    }

    // clear the last row
    for (size_t current_col = 0; current_col < VGA_WIDTH; ++current_col)
    {
        terminal_buffer[index] = vga_entry(' ', terminal_color);
        ++index;
    }

    terminal_column = 0;
    terminal_row = VGA_HEIGHT - 1;

    update_cursor(terminal_column, terminal_row);
}

void VGA_terminal_writestring_top(const char *string, uint16_t column)
{
    uint16_t oldRow = terminal_row;
    uint16_t oldCol = terminal_column;
    uint8_t oldColor = terminal_color;

    terminal_row = 0;
    terminal_column = column;
    terminal_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);

    terminal_writestring(string);

    terminal_row = oldRow;
    terminal_column = oldCol;
    terminal_color = oldColor;

    update_cursor(terminal_column, terminal_row);
}

void VGA_terminal_print_int_top(int value, uint16_t column)
{
    uint16_t oldRow = terminal_row;
    uint16_t oldCol = terminal_column;
    uint8_t oldColor = terminal_color;

    terminal_row = 0;
    terminal_column = column;
    terminal_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);

    //terminal_print_int(value);
    kprintf("%d", value);

    terminal_row = oldRow;
    terminal_column = oldCol;
    terminal_color = oldColor;

    update_cursor(terminal_column, terminal_row);
}

void terminal_newline()
{
    terminal_putchar('\n');
}

void VGA_terminal_putchar(char c)
{
    if (c == '\b')
    {
        VGA_terminal_backspace();
        return;
    }

    if (c == '\n' )//|| c == '\r')
    {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row == VGA_HEIGHT)
        terminal_scroll_up();

    if (c != '\n' && c != '\r')
    {
        VGA_terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

        if (++terminal_column == VGA_WIDTH)
        {
            terminal_column = 0;
            ++terminal_row;
        }
    }

    update_cursor(terminal_column, terminal_row);
}

void terminal_print_byte(int number)
{
    if (number >= 200)
    {
        terminal_putchar('2');
        terminal_putchar(intToChar((number % 100) / 10));
    }
    else
    {
        if (number >= 100)
        {
            terminal_putchar('1');
            terminal_putchar(intToChar((number % 100) / 10));
        }
    }

    if (number >= 10 && number < 100)
    {
        number %= 100;
        char c = intToChar(number / 10);
        terminal_putchar(c);
    }

    int ones = number % 10;
    terminal_putchar(intToChar(ones));

    update_cursor(terminal_column, terminal_row);
}

char int_to_hex(int value)
{
    char *digits = "0123456789ABCDEF";
    if (value > 0xF || value < 0)
        return '?';

    return digits[value];
}

void terminal_print_ulong_hex(uint32_t value)
{
    terminal_putchar('0');
    terminal_putchar('x');

    terminal_putchar(int_to_hex(value >> 28));
    terminal_putchar(int_to_hex((value & 0x0F000000) >> 24));
    terminal_putchar(int_to_hex((value & 0x00F00000) >> 20));
    terminal_putchar(int_to_hex((value & 0x000F0000) >> 16));
    terminal_putchar(int_to_hex((value & 0x0000F000) >> 12));
    terminal_putchar(int_to_hex((value & 0x00000F00) >> 8));
    terminal_putchar(int_to_hex((value & 0x000000F0) >> 4));
    terminal_putchar(int_to_hex(value & 0x0000000F));

    update_cursor(terminal_column, terminal_row);
}

typedef union _LARGE_INTEGER {
    struct {
        uint32_t LowPart;
        int32_t  HighPart;
    } lowAndHigh;
    int64_t QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

void terminal_print_ulonglong_hex(uint64_t value)
{
    uint8_t *currentByte = (uint8_t *)&value;

    terminal_putchar('0');
    terminal_putchar('x');

    // print each hex byte in reverse-order (for little-endian architecture)
    for (int i = 7; i >= 0; --i)
    {
        terminal_print_byte_hex(currentByte[i]);
    }
}

void terminal_print_ushort_hex(uint16_t value)
{
    terminal_putchar('0');
    terminal_putchar('x');

    terminal_putchar(int_to_hex((value & 0x0000F000) >> 12));
    terminal_putchar(int_to_hex((value & 0x00000F00) >> 8));
    terminal_putchar(int_to_hex((value & 0x000000F0) >> 4));
    terminal_putchar(int_to_hex(value & 0x0000000F));

    update_cursor(terminal_column, terminal_row);
}

void terminal_print_byte_hex_leading(int value, bool leading)
{
    if (leading)
    {
        terminal_putchar('0');
        terminal_putchar('x');
    }

    terminal_print_byte_hex(value);
}

void terminal_print_byte_hex(int value)
{
    terminal_putchar(int_to_hex((value & 0x000000F0) >> 4));
    terminal_putchar(int_to_hex(value & 0x0000000F));

    update_cursor(terminal_column, terminal_row);
}
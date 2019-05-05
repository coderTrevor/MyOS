#include "Terminal.h"
#include "Graphics/Display_HAL.h"
#include "Graphics/Graphical_Terminal.h"
#include "misc.h"
#include "printf.h"

// Handles an "abstract" terminal

void terminal_backspace()
{
    if (textMode)
        VGA_terminal_backspace();
    else
        GraphicalTerminalBackspace();
}

// Scans a range of memory looking for nonzero values and dumps them to the screen when it finds them
void terminal_dump_nonzero_memory(uint32_t address, uint32_t lastAddress)
{
    for (; address < lastAddress; address += 16)
    {
        bool nonZero = false;

        for (int i = 0; i < 16; ++i)
        {
            if (*(uint8_t *)(address + i))
                nonZero = true;
        }

        if (nonZero)
        {
            kprintf("0x%X: ", address);
            terminal_dumpHex((uint8_t*)address, 16);
        }
    }
}

void terminal_initialize(void)
{
    if (textMode)
        VGA_terminal_initialize();
    else
        GraphicalTerminalInit();
}

void terminal_print_int(int number)
{
    int digits = 1;

    if (number >= 10)
        digits = 2;
    if (number >= 100)
        digits = 3;
    if (number >= 1000)
        digits = 4;
    if (number >= 10000)
        digits = 5;
    if (number >= 100000)
        digits = 6;
    if (number >= 1000000)
        digits = 7;
    if (number >= 10000000)
        digits = 8;
    if (number >= 100000000)
        digits = 9;
    if (number >= 1000000000)
        digits = 10;

    for (int i = digits; i > 0; --i)
    {
        int currentDigitPlace = 1;

        // calculate current digit position (10^i)
        for (int j = 1; j < i; j++)
            currentDigitPlace *= 10;

        // get current digit
        int digit = number / currentDigitPlace;

        // print current digit
        terminal_putchar(intToChar(digit));

        // update number to remove highest digit
        number = number % currentDigitPlace;
    }

    //update_cursor(terminal_column, terminal_row);
}

void terminal_print_int_top(int value, uint16_t column)
{
    if (textMode)
        VGA_terminal_print_int_top(value, column);
    else
        GraphicalTerminalPrintIntTop(value, column);
}

void terminal_putchar(char c)
{
    if (c == '\b')
    {
        terminal_backspace();
        return;
    }

    if (textMode)
        VGA_terminal_putchar(c);
    else
        GraphicalTerminalPutChar(c);

    // TODO: Output to serial
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    if (textMode)
        VGA_terminal_putentryat(c, color, x, y);
    else
        GraphicalTerminalPutEntryAt(c, x, y);
}

void terminal_write(const char* data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        terminal_putchar(data[i]);
    }
}

// TODO: Fix long strings causing problems somehow
void terminal_writestring(const char* data)
{
    terminal_write(data, strlen(data));
}

void terminal_writestring_top(const char *string, uint16_t column)
{
    if (textMode)
        VGA_terminal_writestring_top(string, column);
    else
        GraphicalTerminalWritestringTop(string, column);
}
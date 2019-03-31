#pragma once
#include "Console_VGA.h"
#include "Graphics/Graphical_Terminal.h"

void terminal_initialize(void);

void terminal_print_int(int number);

void terminal_print_int_top(int number, uint16_t column);

void terminal_putchar(char c);

void terminal_write(const char* data, size_t size);

void terminal_writestring(const char* data);

void terminal_writestring_top(const char *string, uint16_t column);

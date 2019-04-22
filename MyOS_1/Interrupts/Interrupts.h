#pragma once

#include <stdarg.h>

#define HARDWARE_INTERRUPTS_BASE     0x20

extern unsigned long interrupts_fired;

void default_exception_handler(void);

void default_interrupt_handler(void);

void Interrupts_Init();

void keyboard_interrupt_handler(void);

void page_fault_handler(void);

void invalid_opcode_handler(void);

void print_string_interrupt_handler(char *str);

void printf_interrupt_handler(char *fmt, va_list va);
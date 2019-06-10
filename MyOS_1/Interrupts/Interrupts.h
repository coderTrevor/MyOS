#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#define HARDWARE_INTERRUPTS_BASE     0x20

extern unsigned long interrupts_fired;

void default_exception_handler(void);

void default_interrupt_handler(void);

void get_graphics_interrupt_handler(int eflags, int cs, bool *graphicsInitialized, int *width, int *height);

void gpf_exception_handler(void);

void Interrupts_Add_Shared_Handler(bool (*sharedHandlerAddress)(void), uint8_t irq);

void Interrupts_Init();

void keyboard_interrupt_handler(void);

void page_allocator_interrupt_handler(int eflags, int cs, unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pRetVal);

void page_fault_handler(void);

void invalid_opcode_handler(void);

void print_string_interrupt_handler(int eflags, int cs, char *str);

void printf_interrupt_handler(int eflags, int cs, const char *fmt, va_list va);
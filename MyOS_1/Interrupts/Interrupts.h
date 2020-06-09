#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "../Libs/SDL/include/SDL_rect.h"
#include "../Graphics/Display_HAL.h"
#include "../myos_io.h"
#include "../Drivers/mouse.h"

#define HARDWARE_INTERRUPTS_BASE     0x20

extern unsigned long interrupts_fired;

void default_exception_handler(void);

void exit_interrupt_handler(int eflags, int cs);

void default_interrupt_handler(void);

uint32_t dispatch_new_task_interrupt_handler(uint32_t eflags, uint32_t cs, uint32_t newStack, uint32_t oldStackStart);

void fclose_interrupt_handler(int eflags, int cs, int fp, int *pRetVal);

void fopen_interrupt_handler(int eflags, int cs, const char *filename, const char *mode, int *fp);

void fread_interrupt_handler(int eflags, int cs, void * ptr, size_t size, size_t count, FILE * stream, size_t *pSize);

void fseek_interrupt_handler(int eflags, int cs, FILE *stream, long int offset, int origin, int *pRetVal);

void ftell_interrupt_handler(int eflags, int cs, FILE *stream, long int *pRetVal);

void get_graphics_interrupt_handler(int eflags, int cs, bool *graphicsInitialized, int *width, int *height);

void get_mouse_state_interrupt_handler(int eflags, int cs, MOUSE_STATE *pMouseState);

void gpf_exception_handler(void);

void graphics_blit_interrupt_handler(int eflags, int cs, const SDL_Rect *sourceRect, PIXEL_32BIT *image);

void Interrupts_Add_Shared_Handler(bool (*sharedHandlerAddress)(void), uint8_t irq);

void Interrupts_Init();

void keyboard_interrupt_handler(void);

void page_allocator_interrupt_handler(int eflags, int cs, unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pRetVal);

void page_fault_handler(void);

void invalid_opcode_handler(void);

void print_string_interrupt_handler(int eflags, int cs, char *str);

void printf_interrupt_handler(int eflags, int cs, const char *fmt, va_list va);

void read_from_keyboard_handler(int eflags, int cs, uint16_t *pScanCode, bool *pRetVal);

void time_delay_ms_interrupt_handler(int eflags, int cs, uint32_t milliSeconds);

void time_get_uptime_ms_handler(int eflags, int cs, uint32_t *pRetVal);
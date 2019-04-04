#pragma once
#include <stdbool.h>
#include <stdint.h>

// Values taken from https://wiki.osdev.org/Sound_Blaster_16
//Digital Sound Processor ports
#define DSP_RESET       0x06
#define DSP_READ        0x0A
#define DSP_WRITE       0x0C
#define DSP_BUFFER      0x0A
#define DSP_STATUS      0x0E
#define DSP_INTERRUPT   0x0F

//Digital Sound Processor commands
#define DSP_CMD_OUTPUT_RATE     0x41
#define DSP_CMD_TRANSFER_MODE   0xB6
#define DSP_CMD_STOP            0xD5
#define DSP_CMD_VERSION         0xE1

//DMA ports
#define DMA_ADDRESS         0xC4
#define DMA_COUNT           0xC6
#define DMA_PAGE            0x8B
#define DMA_SINGLE_MASK     0xD4
#define DMA_TRANSFER_MODE   0xD6
#define DMA_CLEAR_POINTER   0xD8

#define SB16_BASE0          0x220
#define SB16_BASE1          0x240
#define SB16_BASE2          0x388

#define READ_WRITE_READY_BIT    0x80
#define DSP_READY               0xAA

// globals
extern bool sb16Present;
extern uint16_t sb16BaseAddress;

// functions
void SB16_Init(void);

uint8_t SB16_Read();

void SB16_Write(uint8_t data);
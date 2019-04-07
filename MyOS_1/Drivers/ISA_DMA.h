#pragma once
#include <stdint.h>
#include <stdbool.h>

// 8-bit channels
#define DMA_CHANNEL_1_ADDRESS   0x02
#define DMA_CHANNEL_1_COUNT     0x03
#define DMA_CHANNEL_2_ADDRESS   0x04
#define DMA_CHANNEL_2_COUNT     0x05
#define DMA_CHANNEL_3_ADDRESS   0x06
#define DMA_CHANNEL_3_COUNT     0x07

// Page registers for 8-bit channels
#define DMA_CHANNEL_1_PAGE_REG  0x83
#define DMA_CHANNEL_2_PAGE_REG  0x81
#define DMA_CHANNEL_3_PAGE_REG  0x82

// Flip-Flop clear register for 8-bit channels
#define DMA_REG_FLIP_FLOP_8BIT  0x0C

// 16-bit channels
#define DMA_CHANNEL_5_ADDRESS   0xC4
#define DMA_CHANNEL_5_COUNT     0xC6
#define DMA_CHANNEL_6_ADDRESS   0xC8
#define DMA_CHANNEL_6_COUNT     0xCA
#define DMA_CHANNEL_7_ADDRESS   0xCC
#define DMA_CHANNEL_7_COUNT     0xCE

// Page registers for 16-bit channels
#define DMA_CHANNEL_5_PAGE_REG  0x8B
#define DMA_CHANNEL_6_PAGE_REG  0x89
#define DMA_CHANNEL_7_PAGE_REG  0x8A

// Flip-Flop clear register for 16-bit channels
#define DMA_REG_FLIP_FLOP_16BIT  0xD8

// DMA Mode Registers
#define DMA_MODE_REGISTER_8BIT  0x0B
#define DMA_MODE_REGISTER_16BIT 0xD6

// Values for Mode Registers
// Bits 0-1 Channel Selection
#define DMA_MODE_CHANNEL_0      0
#define DMA_MODE_CHANNEL_4      0
#define DMA_MODE_CHANNEL_1      1
#define DMA_MODE_CHANNEL_5      1
#define DMA_MODE_CHANNEL_2      2
#define DMA_MODE_CHANNEL_6      2
#define DMA_MODE_CHANNEL_3      3
#define DMA_MODE_CHANNEL_7      3

// Bits 2-3 Transfer Type
#define DMA_MODE_TRANSFER_VERIFY    0x00 /* self-test */
#define DMA_MODE_TRANSFER_WRITE     0x04
#define DMA_MODE_TRANSFER_READ      0x08
// NOTE: setting both bits of transfer type is invalid

// Bit 4 Auto-Initialization Enable
#define DMA_MODE_SINGLE_CYCLE       0x00
#define DMA_MODE_AUTO_INIT          0x10

// Bit 5 Address Increment/Decrement
#define DMA_MODE_ADDRESS_INC        0x00
#define DMA_MODE_ADDRESS_DEC        0x20

// Bits 6-7 Mode Selection
#define DMA_MODE_DEMAND_MODE        0x00
#define DMA_MODE_SINGLE_MODE        0x40
#define DMA_MODE_BLOCK_MODE         0x80
#define DMA_MODE_CASCADE_MODE       0xC0

// Single Channel mask registers
#define DMA_SINGLE_CHANNEL_MASK_REG_8BIT    0x0A
#define DMA_SINGLE_CHANNEL_MASK_REG_16BIT   0xD4

// Values for Single Channel mask registers
// Bit 2 - mask enable / clear
#define DMA_SINGLE_DRQ_DONT_MASK    0x00
#define DMA_SINGLE_DRQ_MASK_CHANNEL 0x04

// bits 1-0 - Channel select
#define DMA_SINGLE_DRQ_CHANNEL_0    0x00
#define DMA_SINGLE_DRQ_CHANNEL_4    0x00
#define DMA_SINGLE_DRQ_CHANNEL_1    0x01
#define DMA_SINGLE_DRQ_CHANNEL_5    0x01
#define DMA_SINGLE_DRQ_CHANNEL_2    0x02
#define DMA_SINGLE_DRQ_CHANNEL_6    0x02
#define DMA_SINGLE_DRQ_CHANNEL_3    0x03
#define DMA_SINGLE_DRQ_CHANNEL_7    0x03

#define DMA_BUFFER_SIZE             (64 * 1024)   /* 64K DMA BUFFER (the maximum size for 8-bit transfers) */


extern uint8_t *DMA_Buffer;

void DMA_ClearFlipFlop(uint8_t channel);

void DMA_DisableChannel(uint8_t channel);

void DMA_EnableChannel(uint8_t channel);

void DMA_InitBuffer();

void DMA_SetBuffer(uint8_t channel, uint8_t *buffer);

void DMA_SetMode(uint8_t channel, uint8_t options);

void DMA_SetTransferLength(uint8_t channel, uint32_t transferLength);

bool DMA_VerifyChannel(uint8_t channel);
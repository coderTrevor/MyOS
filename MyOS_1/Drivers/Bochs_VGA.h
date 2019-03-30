#pragma once
#include <stdbool.h>
#include <stdint.h>

// TODO: Support multiple displays
extern uint8_t BGA_bus;
extern uint8_t BGA_slot;
extern uint8_t BGA_function;
extern uint32_t *BGA_linearFrameBuffer;
extern bool BGA_present;

#define VBE_DISPI_IOPORT_INDEX  0x01CE
#define VBE_DISPI_IOPORT_DATA   0x01CF

// BGA Register Indices
#define VBE_DISPI_INDEX_ID      0
#define VBE_DISPI_INDEX_XRES    1
#define VBE_DISPI_INDEX_YRES    2
#define VBE_DISPI_INDEX_BPP     3
#define VBE_DISPI_INDEX_ENABLE  4
#define VBE_DISPI_INDEX_BANK    5
#define VBE_DISPI_INDEX_VIRT_WIDTH  6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET    8
#define VBE_DISPI_INDEX_Y_OFFSET    9

// Version magics
// Qemu always reports version 1, while having capabilities of the latest version, making these magics somewhat unusable :/
#define BOCHS_VGA_VERSION_1     0xB0C0
#define BOCHS_VGA_VERSION_2     0xB0C1
#define BOCHS_VGA_VERSION_3     0xB0C2
#define BOCHS_VGA_VERSION_4     0xB0C3
#define BOCHS_VGA_VERSION_5     0xB0C4
#define BOCHS_VGA_VERSION_6     0xB0C5
#define BOCHS_VGA_VERSION_LATEST    BOCHS_VGA_VERSION_6

// Flags
#define VBE_DISPI_DISABLED      0x00
#define VBE_DISPI_ENABLED       0x01
#define VBE_DISPI_LFB_ENABLED   0x40
#define VBE_DISPI_NOCLEARMEM    0x80

bool BGA_CheckPresence();

void BGA_Init(uint8_t bus, uint8_t slot, uint8_t function);

uint16_t BGA_ReadRegister(uint16_t registerIndex);

bool BGA_SetResolution(uint16_t width, uint16_t height, uint16_t bitDepth);

void BGA_WriteRegister(uint16_t registerIndex, uint16_t data);
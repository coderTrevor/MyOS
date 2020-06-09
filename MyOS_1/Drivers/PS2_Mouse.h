#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../Graphics/Display_HAL.h"
#include "../misc.h"
#include "mouse.h"

#define PS2_DATA_PORT        0x60
#define PS2_STATUS_PORT      0x64
#define PS2_COMMAND_PORT     0x64

//#define PS2_COMMAND                     8
//#define PS2_DATA                        0

#define PS2_COMMAND_GET_STATUS_BYTE     0x20
#define PS2_COMMAND_SET_STATUS_BYTE     0x60
#define PS2_COMMAND_DISABLE_PORT_2      0xA7
#define PS2_COMMAND_ENABLE_PORT_2       0xA8
#define PS2_COMMAND_DISABLE_PORT_1      0xAD
#define PS2_COMMAND_ENABLE_PORT_1       0xAE
#define PS2_COMMAND_DISABLE_SCANNING    0xF5
#define PS2_COMMAND_WRITE_TO_PORT_2     0xD4    /* Next byte written to 0x60 will go to second port */
#define PS2_STATUS_OUTPUT_BUFFER_FULL   1
#define PS2_STATUS_INPUT_BUFFER_FULL    2
#define PS2_STATUS_ENABLE_IRQ_12        0x02
#define PS2_STATUS_ENABLE_MOUSE_CLOCK   0x20

#define PS2_DEVICE_SELF_TEST_OK         0xAA
#define PS2_DEVICE_IDENTIFY             0xF2
#define PS2_DEVICE_ENABLE_STREAMING     0xF4
#define PS2_DEVICE_ACK                  0xFA
#define PS2_DEVICE_RESET                0xFF

#define PS2_DEVICE_TYPE_MOUSE           0x00
#define PS2_DEVICE_TYPE_MOUSE_SCROLL    0x03
#define PS2_DEVICE_TYPE_MOUSE_5BUTTON   0x04

// defines for packet byte 1
#define MOUSE_LEFT_BUTTON               0x01
#define MOUSE_RIGHT_BUTTON              0x02
#define MOUSE_MIDDLE_BUTTON             0x04
#define MOUSE_ALWAYS_1                  0x08
#define MOUSE_X_SIGN                    0x10
#define MOUSE_Y_SIGN                    0x20
#define MOUSE_X_OVERFLOW                0x40
#define MOUSE_Y_OVERFLOW                0x80

extern MOUSE_STATE mouseState;

extern bool mousePresent;
extern bool fourBytePackets;

extern int oldMouseX;
extern int oldMouseY;
extern PIXEL_32BIT oldColor;


void Mouse_Init(void);

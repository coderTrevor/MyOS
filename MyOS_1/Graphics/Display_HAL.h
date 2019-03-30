#pragma once
#include <stdbool.h>
#include <stdint.h>

extern bool graphicsPresent;
extern bool textMode;
extern uint32_t *linearFrameBuffer;
extern int graphicsBpp;
extern int graphicsWidth;
extern int graphicsHeight;

#pragma pack(push, 1)
typedef struct PIXEL_32BIT
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha; // might need to be zero, I'm not sure
} PIXEL_32BIT;
#pragma pack(pop)

void GraphicsFillScreen(uint8_t red, uint8_t green, uint8_t blue);
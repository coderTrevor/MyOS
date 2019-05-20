#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../multiboot.h"

extern bool graphicsPresent;
extern bool textMode;
extern uint32_t *linearFrameBuffer;
extern unsigned int graphicsBpp;
extern unsigned int graphicsWidth;
extern unsigned int graphicsHeight;

#pragma pack(push, 1)
typedef struct PIXEL_32BIT
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha; // might need to be zero, I'm not sure
} PIXEL_32BIT;
#pragma pack(pop)


void GraphicsBlit(unsigned int x, unsigned int y, PIXEL_32BIT *image, uint32_t width, uint32_t height);

void GraphicsBlitToForeground(unsigned int x, unsigned int y, PIXEL_32BIT *image, uint32_t width, uint32_t height);

void GraphicsBlitWithAlpha(unsigned int x, unsigned int y, PIXEL_32BIT *image, uint32_t width, uint32_t height);

void GraphicsClearLines(unsigned int firstLine, unsigned int lines, PIXEL_32BIT color, uint32_t *imageBuffer);

void GraphicsCopyToImage(unsigned int x, unsigned int y, PIXEL_32BIT *dest, uint32_t width, uint32_t height);

void GraphicsInitFromGrub(multiboot_info *multibootInfo);

void GraphicsFillScreen(uint8_t red, uint8_t green, uint8_t blue);

PIXEL_32BIT GraphicsGetPixel(unsigned int x, unsigned int y);

void GraphicsPlotPixel(unsigned int x, unsigned int y, PIXEL_32BIT color);
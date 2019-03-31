#include "Display_HAL.h"
#include "../Drivers/Bochs_VGA.h"
#include "picofont.h"
#include "../multiboot.h"
#include "../misc.h"

// TODO: Support multiple displays
bool graphicsPresent = false;
bool textMode = true;
uint32_t *linearFrameBuffer = NULL;
unsigned int graphicsBpp;
unsigned int graphicsWidth;
unsigned int graphicsHeight;

// TODO: Create HAL interface and tie Bochs_VGA into it

void GraphicsInitFromGrub(multiboot_info *multibootInfo)
{
    // If Grub didn't initialize graphics for us, return immediately
    if (!(multibootInfo->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO))
        return;

    // Make sure Grub initialized a graphical mode
    if (multibootInfo->framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT)
        return;

    // TODO: Support palettized modes, maybe
    // TODO: Support bit depths other than 32, maybe

    graphicsBpp = multibootInfo->framebuffer_bpp;
    graphicsWidth = multibootInfo->framebuffer_width;
    graphicsHeight = multibootInfo->framebuffer_height;

    linearFrameBuffer = (uint32_t *)((uint32_t)multibootInfo->framebuffer_addr);

    graphicsPresent = true;
    textMode = false;

    // Fill screen with color so we know it worked
    GraphicsFillScreen(168, 68, 255);
}

void GraphicsFillScreen(uint8_t red, uint8_t green, uint8_t blue)
{
    // TODO: Support other bitdepths / announce error
    if (graphicsBpp != 32)
        return;

    uint32_t color;
    PIXEL_32BIT *pColor = (PIXEL_32BIT *)&color;

    pColor->red = red;
    pColor->green = green;
    pColor->blue = blue;
    pColor->alpha = 0;
    
    int currentIndex = 0;

    for (unsigned int y = 0; y < graphicsHeight; ++y)
    {
        for (unsigned int x = 0; x < graphicsWidth; ++x)
            linearFrameBuffer[currentIndex++] = color;
    }
}

// Sets a given range of lines on the screen to the given color
void GraphicsClearLines(unsigned int firstLine, unsigned int lines, PIXEL_32BIT color)
{
    // TODO: Support other bitdepths / announce error
    if (graphicsBpp != 32)
        return;

    unsigned int currentIndex = firstLine * graphicsWidth;

    uint32_t clearColor;
    memcpy(&clearColor, &color, sizeof(uint32_t));

    for (unsigned y = 0; y < lines; ++y)
    {
        for (unsigned int x = 0; x < graphicsWidth; ++x)
            linearFrameBuffer[currentIndex++] = clearColor;
    }
}
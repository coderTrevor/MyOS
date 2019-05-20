#include "Display_HAL.h"
#include "../Drivers/Bochs_VGA.h"
#include "picofont.h"
#include "../multiboot.h"
#include "../misc.h"
#include "Graphical_Terminal.h"

// TODO: Support multiple displays
bool graphicsPresent = false;
bool textMode = true;
uint32_t *linearFrameBuffer = NULL;
unsigned int graphicsBpp;
unsigned int graphicsWidth;
unsigned int graphicsHeight;

// TODO: Create HAL interface and tie Bochs_VGA into it

// copy an image to the given coordinates on the screen
// it is the caller's responsibility to ensure the image doesn't go off the screen (for now)
void GraphicsBlit(unsigned int x, unsigned int y, PIXEL_32BIT *image, uint32_t width, uint32_t height)
{
    unsigned int framebufferOffset = ((y * graphicsWidth + x) * sizeof(PIXEL_32BIT));
    PIXEL_32BIT *currentPixel = (PIXEL_32BIT *)((uint32_t)linearFrameBuffer + framebufferOffset);

    // for every row of pixels
    for (size_t h = 0; h < height; ++h)
    {
        // copy the current row
        memcpy((void*)currentPixel, (const void *)image, sizeof(PIXEL_32BIT) * width);

        // advance the pointers
        currentPixel += graphicsWidth;
        image += width;
    }
}

// copy an image to the given coordinates in the foreground
// it is the caller's responsibility to ensure the image doesn't go off the screen (for now)
void GraphicsBlitToForeground(unsigned int x, unsigned int y, PIXEL_32BIT *image, uint32_t width, uint32_t height)
{
    unsigned int framebufferOffset = ((y * graphicsWidth + x) * sizeof(PIXEL_32BIT));
    PIXEL_32BIT *currentPixel = (PIXEL_32BIT *)((uint32_t)foregroundText + framebufferOffset);

    // for every row of pixels
    for (size_t h = 0; h < height; ++h)
    {
        // copy the current row
        memcpy((void*)currentPixel, (const void *)image, sizeof(PIXEL_32BIT) * width);

        // advance the pointers
        currentPixel += graphicsWidth;
        image += width;
    }
}

void GraphicsBlitWithAlpha(unsigned int x, unsigned int y, PIXEL_32BIT *image, uint32_t width, uint32_t height)
{
    unsigned int framebufferOffset = ((y * graphicsWidth + x) * sizeof(PIXEL_32BIT));
    PIXEL_32BIT *currentPixel = (PIXEL_32BIT *)((uint32_t)linearFrameBuffer + framebufferOffset);

    // for every row of pixels
    for (size_t h = 0; h < height; ++h)
    {
        // copy the current row
        for (size_t w = 0; w < width; ++w)
        {
            if(image->alpha)
                memcpy((void*)currentPixel, (const void *)image, sizeof(PIXEL_32BIT));

            ++currentPixel;
            ++image;
        }

        // advance the pointers
        currentPixel += graphicsWidth - width;
    }
}

// Backup section of the screen to an image
void GraphicsCopyToImage(unsigned int x, unsigned int y, PIXEL_32BIT *dest, uint32_t width, uint32_t height)
{
    unsigned int framebufferOffset = ((y * graphicsWidth + x) * sizeof(PIXEL_32BIT));
    PIXEL_32BIT *currentPixel = (PIXEL_32BIT *)((uint32_t)linearFrameBuffer + framebufferOffset);
    //unsigned int destOffset = 0;

    // for every row of pixels
    for (size_t h = 0; h < height; ++h)
    {
        // copy the current row
        memcpy(dest, (void*)currentPixel, sizeof(PIXEL_32BIT) * width);

        // advance the pointers
        currentPixel += graphicsWidth;
        dest += width;
    }
}

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

PIXEL_32BIT GraphicsGetPixel(unsigned int x, unsigned int y)
{
    unsigned int framebufferOffset = ((y * graphicsWidth + x) * sizeof(PIXEL_32BIT));
    PIXEL_32BIT *currentPixel = (PIXEL_32BIT *)((uint32_t)linearFrameBuffer + framebufferOffset);

    return *currentPixel;
}

// Sets a given range of lines on the screen to the given color
void GraphicsClearLines(unsigned int firstLine, unsigned int lines, PIXEL_32BIT color, uint32_t *imageBuffer)
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
            imageBuffer[currentIndex++] = clearColor;
    }
}

void GraphicsPlotPixel(unsigned int x, unsigned int y, PIXEL_32BIT color)
{
    unsigned int framebufferOffset = ((y * graphicsWidth + x) * sizeof(PIXEL_32BIT));
    PIXEL_32BIT *currentPixel = (PIXEL_32BIT *)((uint32_t)linearFrameBuffer + framebufferOffset);

    // copy the current row
    memcpy((void*)currentPixel, &color, sizeof(PIXEL_32BIT));
}
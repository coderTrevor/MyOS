#include "Display_HAL.h"
#include "../Drivers/Bochs_VGA.h"

// TODO: Support multiple displays
bool graphicsPresent = false;
bool textMode = true;
uint32_t *linearFrameBuffer = NULL;
int graphicsBpp;
int graphicsWidth;
int graphicsHeight;

// TODO: Create HAL interface and tie Bochs_VGA into it

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

    for (int y = 0; y < graphicsHeight; ++y)
    {
        for (int x = 0; x < graphicsWidth; ++x)
            linearFrameBuffer[currentIndex++] = color;
    }
}
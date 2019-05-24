#include "Graphical_Terminal.h"
#include "../Terminal.h"
#include "../misc.h"
#include "../printf.h"
#include "Bitmap.h"

// TODO: Support multiple graphical terminals
unsigned int graphicalColumn = 0;
unsigned int graphicalRow = 0;
unsigned int graphicalMaxColumns = 0;
unsigned int graphicalMaxRows = 0;

FNT_xy currentPos;
PIXEL_32BIT graphicalForeground, graphicalBackground, graphicalOutline;
PIXEL_32BIT *backgroundImage = NULL;
PIXEL_32BIT *foregroundText = NULL;

// TODO: Support larger resolutions. Presently making this buffer too large causes a GPF at boot.
// It's possible I'm reaching the limits of what I can do without memory allocation.
PIXEL_32BIT scrollBuffer[MAX_X_RES * MAX_Y_RES];

void GraphicalTerminalBackspace()
{
    if (graphicalColumn == 0 && graphicalRow == 0)
    {
        GraphicalTerminalPutEntryAt(' ', 0, 0);
        return;
    }

    if (graphicalColumn > 0)
    {
        --graphicalColumn;
        GraphicalTerminalPutEntryAt(' ', graphicalColumn, graphicalRow);
        //update_cursor(terminal_column, terminal_row);
        //TODO: Update cursor
        return;
    }

    graphicalColumn = graphicalMaxColumns - 1;
    --graphicalRow;
    GraphicalTerminalPutEntryAt(' ', graphicalColumn, graphicalRow);
    //update_cursor(terminal_column, terminal_row);
     //TODO: Update cursor
}

void GraphicalTerminalInit()
{
    // determine maximum columns and rows based on font size
    graphicalMaxColumns = (graphicsWidth - (FNT_LEFTRIGHTMARGIN * 2)) / FNT_FONTWIDTH;
    graphicalMaxRows = (graphicsHeight - (FNT_TOPBOTTOMMARGIN * 2)) / (FNT_FONTHEIGHT + FNT_ROWSPACING);

    //currentPos.x = currentPos.y = 0;

    // set graphicalForeground to white
    graphicalForeground.alpha = 255;
    graphicalForeground.red = 255;
    graphicalForeground.green = 255;
    graphicalForeground.blue = 255;

    // set graphicalBackground to black
    graphicalBackground.alpha = 0;
    graphicalBackground.red = 0;
    graphicalBackground.green = 0;
    graphicalBackground.blue = 0;

    // set graphicalOutline to black with opaque alpha
    graphicalOutline.alpha = 255;
    graphicalOutline.red = 0;
    graphicalOutline.green = 0;
    graphicalOutline.blue = 0;

    graphicalColumn = 0;
    graphicalRow = 0;

    GraphicsFillScreen(0, 0, 0);

    size_t foregroundBufferSize = sizeof(PIXEL_32BIT) * graphicsWidth * graphicsHeight;

    // If the foreground hasn't been allocated
    if(!foregroundText)
    {
        // Allocate memory for foreground buffer
        foregroundText = malloc(foregroundBufferSize);
    }

    // Clear out the foreground buffer
    memset(foregroundText, 0, foregroundBufferSize);

    // If there's a background image, draw it now
    if (backgroundImage)
        GraphicsBlit(0, 0, backgroundImage, graphicsWidth, graphicsHeight);

}

/*void GraphicalTerminalWriteString(char *string)
{
    currentPos.y = graphicalRow * (FNT_FONTHEIGHT + FNT_ROWSPACING);
    currentPos.x = graphicalColumn * FNT_FONTWIDTH;

    FNT_Render(string, currentPos);
}*/


void GraphicalTerminalPrintIntTop(int value, uint16_t column)
{
    unsigned int oldRow = graphicalRow;
    unsigned int oldCol = graphicalColumn;
    
    // set position to top of screen at the given column
    graphicalRow = 0;
    graphicalColumn = column;

    // swap background and foreground colors
    PIXEL_32BIT temp = graphicalBackground;
    graphicalBackground = graphicalForeground;
    graphicalForeground = temp;

    //terminal_print_int(value);
    kprintf("%d", value);

    // restore position
    graphicalRow = oldRow;
    graphicalColumn = oldCol;

    // restore colors
    graphicalForeground = graphicalBackground;
    graphicalBackground = temp;
}

void GraphicalTerminalPutChar(char c)
{
    if (c == '\n')//|| c == '\r')
    {
        graphicalColumn = 0;
        graphicalRow++;
    }

    // TODO: Scrolling
    if (graphicalRow >= graphicalMaxRows)
        GraphicalTerminalScrollUp();

    if (c != '\n' && c != '\r')
    {
        GraphicalTerminalPutEntryAt(c, graphicalColumn, graphicalRow);

        if (++graphicalColumn >= graphicalMaxColumns)
        {
            graphicalColumn = 0;
            ++graphicalRow;
        }
    }

    //update_cursor(terminal_column, terminal_row);
    // TODO: Update cursor
}

void GraphicalTerminalPutEntryAt(char c, size_t x, size_t y)
{
    char str[2] = { 0 };

    currentPos.y = y * (FNT_FONTHEIGHT + FNT_ROWSPACING);
    currentPos.x = x * FNT_FONTWIDTH;

    str[0] = c;
    FNT_RenderMax(str, 1, currentPos);
}

void GraphicalTerminalScrollUp(void)
{
    graphicalColumn = 0;
    graphicalRow = graphicalMaxRows - 1;

    //memset(scrollBuffer, 0, MAX_X_RES * MAX_Y_RES * 4);

    if (backgroundImage && foregroundText)
    {
        // Read the contents of the foreground text, but don't copy the first line
        uint32_t bufferSize = graphicsWidth * graphicsHeight * (graphicsBpp / 8);
        uint32_t lineOffset = graphicsWidth * (FNT_FONTHEIGHT + FNT_TOPBOTTOMMARGIN) * (graphicsBpp / 8);
        memcpy(scrollBuffer, (void *)((uint32_t)foregroundText + lineOffset), bufferSize - lineOffset);

        // Clear the bottom two rows of the foreground
        uint32_t firstLine = graphicalRow * (FNT_FONTHEIGHT + FNT_TOPBOTTOMMARGIN);
        uint32_t lines = (FNT_FONTHEIGHT + FNT_TOPBOTTOMMARGIN) * 2;
        GraphicsClearLines(firstLine, lines, graphicalBackground, (uint32_t *)foregroundText);

        //GraphicsFillScreen(graphicalBackground.red, graphicalBackground.green, graphicalBackground.blue);

        // Copy the buffer back to the screen
        memcpy(foregroundText, scrollBuffer, bufferSize - lineOffset);

        GraphicsBlit(0, 0, backgroundImage, graphicsWidth, graphicsHeight);
        GraphicsBlitWithAlpha(0, 0, foregroundText, graphicsWidth, graphicsHeight);
    }
    else
    {
        // Read the contents of the screen, but don't copy the first line
        uint32_t bufferSize = graphicsWidth * graphicsHeight * (graphicsBpp / 8);
        uint32_t lineOffset = graphicsWidth * (FNT_FONTHEIGHT + FNT_TOPBOTTOMMARGIN) * (graphicsBpp / 8);
        memcpy(scrollBuffer, (void *)((uint32_t)linearFrameBuffer + lineOffset), bufferSize - lineOffset);

        // Clear the bottom two rows of the screen (or the entire screen)
        uint32_t firstLine = graphicalRow * (FNT_FONTHEIGHT + FNT_TOPBOTTOMMARGIN);
        uint32_t lines = (FNT_FONTHEIGHT + FNT_TOPBOTTOMMARGIN) * 2;
        GraphicsClearLines(firstLine, lines, graphicalBackground, linearFrameBuffer);

        //GraphicsFillScreen(graphicalBackground.red, graphicalBackground.green, graphicalBackground.blue);

        // Copy the buffer back to the screen
        memcpy(linearFrameBuffer, scrollBuffer, bufferSize - lineOffset);
    }
}

void GraphicalTerminalWritestringTop(const char *string, uint16_t column)
{
    unsigned int oldRow = graphicalRow;
    unsigned int oldCol = graphicalColumn;

    // swap background and foreground colors
    PIXEL_32BIT temp = graphicalBackground;
    graphicalBackground = graphicalForeground;
    graphicalForeground = temp;

    graphicalRow = 0;
    graphicalColumn = column;

    terminal_writestring(string);

    graphicalRow = oldRow;
    graphicalColumn = oldCol;

    // restore colors
    graphicalForeground = graphicalBackground;
    graphicalBackground = temp;
}

bool SetGraphicalTerminalBackground(char *bitmapFileName)
{
    uint32_t width, height;

    if (!Bitmap24Load(bitmapFileName, &backgroundImage, &width, &height))
        return false;

    // We don't have support for resizing images yet
    if (width != MAX_X_RES || height != MAX_Y_RES)
    {
        free(backgroundImage);
        backgroundImage = NULL;
        kprintf("Image given is %d x %d resolution\n", width, height);
        kprintf("Can only set a background with an image of %d x %d\n", graphicsWidth, graphicsHeight);
        return false;
    }

    GraphicsCopyToImage(0, 0, foregroundText, graphicsWidth, graphicsHeight);
    return true;
}

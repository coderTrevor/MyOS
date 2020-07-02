#include "GUI_TerminalWindow.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __MYOS
#include "../MyOS_1/Interrupts/System_Calls.h"
#endif

#include <string.h>

GUI_TerminalWindow::GUI_TerminalWindow(const char *name)
    : GUI_Window(NewWindowPosition(GUI_TERMINAL_DEFAULT_WIDTH, GUI_TERMINAL_DEFAULT_HEIGHT), name)
{
    // TODO: Check for out of memory error
    characterBuffer = new char(characterBufferSize);

    foregroundTextColor = SDL_MapRGB(pSurface->format, 255, 255, 255);
    
    // Set a dark gray background
    backgroundTextColor = SDL_MapRGB(pSurface->format, 40, 40, 40);
    backgroundColor = SDL_DARKGRAY;

    row = 0;
    column = 0;
    maxColumns = GUI_TERMINAL_DEFAULT_COLUMNS;
    maxRows = GUI_TERMINAL_DEFAULT_ROWS;

    DrawWindow();
}

GUI_TerminalWindow::~GUI_TerminalWindow()
{
    delete characterBuffer;
}

void GUI_TerminalWindow::PutEntryAt(char c, size_t x, size_t y)
{
    char str[2] = { 0 };

    FNT_xy currentPos;

    currentPos.y = y * (FNT_FONTHEIGHT + FNT_ROWSPACING) + SYSTEM_MENU_HEIGHT;
    currentPos.x = x * FNT_FONTWIDTH + 1;

    str[0] = c;
    FNT_RenderMax(str, 1, currentPos);
}

void GUI_TerminalWindow::PutChar(char c)
{
    if (c == '\n')//|| c == '\r')
    {
        column = 0;
        row++;
    }

    // Handle scrolling
    if (row >= maxRows)
        ScrollUp();

    if (c != '\n' && c != '\r')
    {
        PutEntryAt(c, column, row);

        if (++column >= maxColumns)
        {
            column = 0;
            ++row;
        }
    }

    // TODO: Update cursor
}

void GUI_TerminalWindow::ScrollUp(void)
{
    column = 0;
    row = maxRows - 1;

    /*if (backgroundImage && foregroundText)
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
    }*/
}

void GUI_TerminalWindow::SendWindowText(const char * text)
{
    size_t len = strlen(text);
    for (size_t i = 0; i < len; ++i)
        PutChar(text[i]);
}

/* Code taken from picofont.c by Fredrik Hultin
    http://nurd.se/~noname/sdl_picofont
    modified for MyOS:
*/

FNT_xy GUI_TerminalWindow::FNT_Generate(const char* text, unsigned int len, unsigned int w, uint32_t *pixels, FNT_xy position)
{
    unsigned int i, x, y, col, row, stop;
    unsigned char *fnt, chr;
    FNT_xy xy;

    fnt = FNT_GetFont();

    col = row = stop = 0;
    xy.x = position.x + FNT_LEFTRIGHTMARGIN;
    xy.y = position.y + FNT_TOPBOTTOMMARGIN;
        
    for (i = 0; i < len && text[i] != '\0'; i++)
    {
        col++;
        chr = text[i];

        if (stop) {
            break;
        }
        
        if (chr == 0 || w == 0)
            continue;

        // TODO: print border based on FNT_ROWSPACING with background color
        for (y = 0; y < FNT_FONTHEIGHT; y++)
        {
            for (x = 0; x < FNT_FONTWIDTH; x++)
            {
                if (fnt[text[i] * FNT_FONTHEIGHT + y] >> (7 - x) & 1)
                    pixels[((col - 1) * FNT_FONTWIDTH) + x + xy.x + (xy.y + y + row * (FNT_FONTHEIGHT + FNT_ROWSPACING)) * w] = foregroundTextColor;
                else
                {
                    //pixels[((col - 1) * FNT_FONTWIDTH) + x + xy.x + (xy.y + y + row * (FNT_FONTHEIGHT + FNT_ROWSPACING)) * w] = backgroundTextColor;
                }
            }
        }
    }

    return xy;
}

void GUI_TerminalWindow::FNT_Render(const char* text, FNT_xy position)
{
    FNT_RenderMax(text, strlen(text), position);
}

void GUI_TerminalWindow::FNT_RenderMax(const char* text, unsigned int len, FNT_xy position)
{
    FNT_Generate(text, len, pSurface->w, (uint32_t *)pSurface->pixels, position);
}
// End of picofont functions

#ifdef __cplusplus
}
#endif /* __cplusplus */
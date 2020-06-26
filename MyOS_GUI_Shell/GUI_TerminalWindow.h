#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Window.h"
#include "MyOS_GUI_Shell.h"
#include "../MyOS_1/Graphics/picofont.h"

#define GUI_TERMINAL_DEFAULT_BUFFER_SIZE    4096
#define GUI_TERMINAL_DEFAULT_TEXT_COLOR     {255,   255,    255,    255}
#define GUI_TERMINAL_DEFAULT_BG_COLOR       {40,     40,     40,    255}


#ifndef FNT_FONTHEIGHT
#define FNT_FONTHEIGHT  8
#endif
#ifndef FNT_FONTWIDTH
#define FNT_FONTWIDTH   8
#endif
#ifndef FNT_ROWSPACING
#define FNT_ROWSPACING  3
#endif

#ifndef FNT_xy
typedef struct
{
    int x;
    int y;
}FNT_xy;
#endif

// put some margins around the borders
#ifndef FNT_LEFTRIGHTMARGIN
#define FNT_LEFTRIGHTMARGIN     3
#define FNT_TOPBOTTOMMARGIN     3
#endif

#define GUI_TERMINAL_DEFAULT_COLUMNS    80
#define GUI_TERMINAL_DEFAULT_ROWS       60
#define GUI_TERMINAL_DEFAULT_WIDTH      (GUI_TERMINAL_DEFAULT_COLUMNS * FNT_FONTWIDTH)
#define GUI_TERMINAL_DEFAULT_HEIGHT     (GUI_TERMINAL_DEFAULT_ROWS * (FNT_FONTHEIGHT + FNT_ROWSPACING))


class GUI_TerminalWindow :
    public GUI_Window
{
public:
    GUI_TerminalWindow(const char *name);
    ~GUI_TerminalWindow();

    void ScrollUp();
    void SendWindowText(const char *text);
    void PutChar(char c);
    void PutEntryAt(char c, size_t x, size_t y);

    // Font functions from picofont by 
    FNT_xy FNT_Generate(const char* text, unsigned int len, unsigned int w, uint32_t *pixels, FNT_xy position);
    void FNT_Render(const char* text, FNT_xy position);
    void FNT_RenderMax(const char* text, unsigned int len, FNT_xy position);

private:
    uint32_t foregroundTextColor;
    uint32_t backgroundTextColor;
    uint32_t characterBufferSize;
    char *characterBuffer;
    //FNT_xy currentPos;
    int column;
    int row;
    int maxColumns;
    int maxRows;
};

#ifdef __cplusplus
}
#endif
#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Window.h"
#include "MyOS_GUI_Shell.h"

#define GUI_TERMINAL_DEFAULT_BUFFER_SIZE    4096
#define GUI_TERMINAL_DEFAULT_TEXT_COLOR     {255,   255,    255,    255}
#define GUI_TERMINAL_DEFAULT_BG_COLOR       {40,     40,     40,    255}

#define GUI_TERMINAL_DEFAULT_WIDTH          80 * 8
#define GUI_TERMINAL_DEFAULT_HEIGHT         60 * 8

class GUI_TerminalWindow :
    public GUI_Window
{
public:
    GUI_TerminalWindow(const char *name);
    ~GUI_TerminalWindow();

    SDL_Color foregroundTextColor;
    uint32_t characterBufferSize;
    char *characterBuffer;
};

#ifdef __cplusplus
}
#endif
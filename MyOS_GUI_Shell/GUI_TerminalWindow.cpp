#include "GUI_TerminalWindow.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __MYOS
#include "../MyOS_1/Interrupts/System_Calls.h"
#endif

GUI_TerminalWindow::GUI_TerminalWindow(const char *name)
    : GUI_Window(NewWindowPosition(GUI_TERMINAL_DEFAULT_WIDTH, GUI_TERMINAL_DEFAULT_HEIGHT), name)
{
    // TODO: Check for out of memory error
    characterBuffer = new char(characterBufferSize);
        //(char*)malloc(characterBufferSize);

    foregroundTextColor = GUI_TERMINAL_DEFAULT_TEXT_COLOR;
    backgroundColor = GUI_TERMINAL_DEFAULT_BG_COLOR;

    DrawWindow();
}


GUI_TerminalWindow::~GUI_TerminalWindow()
{
    //free(characterBuffer);
    delete characterBuffer;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
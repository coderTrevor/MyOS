#pragma once

#include "GUI_Window.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    struct GUI_WINDOW_STACK_ENTRY;
    typedef struct GUI_WINDOW_STACK_ENTRY
    {
        GUI_Window *pWindow;
        GUI_WINDOW_STACK_ENTRY *pUnderneath;
        GUI_WINDOW_STACK_ENTRY *pAbove;
    } GUI_WINDOW_STACK_ENTRY;

    void BringWindowID_ToFront(uint32_t windowID);
    void BringWindowToFront(GUI_WINDOW_STACK_ENTRY *pEntry);
    void Shell_Destroy_Window(GUI_Window *pWindow);


#ifdef __cplusplus
}
#endif /* __cplusplus */
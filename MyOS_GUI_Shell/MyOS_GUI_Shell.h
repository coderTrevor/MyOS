#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Window.h"

struct GUI_WINDOW_STACK_ENTRY;
typedef struct GUI_WINDOW_STACK_ENTRY
{
    GUI_Window *pWindow;
    GUI_WINDOW_STACK_ENTRY *pUnderneath;
    GUI_WINDOW_STACK_ENTRY *pAbove;
} GUI_WINDOW_STACK_ENTRY;

void BringWindowID_ToFront(uint32_t windowID);
void BringWindowToFront(GUI_WINDOW_STACK_ENTRY *pEntry);
GUI_Window *CreateTextWindow(uint32_t uniqueID, const char *windowName);
GUI_Window *GetWindowFromID(uint32_t uniqueID);
GUI_Rect NewWindowPosition(int x, int y);
void MessageBox(char *messageText, char *windowTitle);
void Shell_Destroy_Window(GUI_Window *pWindow);


#ifdef __cplusplus
}
#endif /* __cplusplus */
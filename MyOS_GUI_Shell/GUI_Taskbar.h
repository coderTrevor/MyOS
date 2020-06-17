#pragma once
#include "../MyOS_GUI_Shell/MyOS_GUI_Shell.h"
#include "../MyOS_GUI_Shell/GUI_Window.h"

#define GUI_TASKBAR_HEIGHT          32
#define GUI_TASKBAR_BUTTON_MARGINS  4
#define GUI_TASKBAR_START_WIDTH     50
#define GUI_TASKBAR_COLOR           { 200, 200, 200 }
#define GUI_TASKBAR_START_ID        0

// Taskbar is created by the GUI Shell application and ties into it
class GUI_Taskbar :
    public GUI_Window
{
public:
    GUI_Taskbar(uint32_t desktopWidth, uint32_t desktopHeight);
    ~GUI_Taskbar();

    void OnDrag(int startRelX, int startRelY, int relX, int relY);
};


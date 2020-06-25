#pragma once
#include "MyOS_GUI_Shell.h"
#include "GUI_Window.h"
#include "GUI_Button.h"

#define GUI_TASKBAR_HEIGHT              32
#define GUI_TASKBAR_BUTTON_MARGINS      4
#define GUI_TASKBAR_BUTTON_MAX_WIDTH    100
//#define GUI_TASKBAR_START_WIDTH         50
#define GUI_TASKBAR_COLOR               { 200, 200, 200 }
#define GUI_TASKBAR_START_ID            0xFFFFFFFF

class GUI_TaskbarButton : public GUI_Button
{
public:
    GUI_TaskbarButton(const char *buttonText, uint32_t controlID, GUI_Window *pOwner, GUI_Rect dimensions)
        : GUI_Button(buttonText, controlID, pOwner, dimensions) {}
    void Highlight();
    virtual void OnMouseUp(int relX, int relY);
    void UnClick();
};


// Taskbar is created by the GUI Shell application and ties into it
class GUI_Taskbar :
    public GUI_Window
{
public:
    GUI_Taskbar(uint32_t desktopWidth, uint32_t desktopHeight);
    ~GUI_Taskbar();

    void AddWindow(uint32_t windowID, GUI_Window *pWindow);
    void ButtonsChanged();
    void ControlClicked(uint32_t controlID);
    void OnDrag(int startRelX, int startRelY, int relX, int relY);
    void RemoveWindow(uint32_t windowID);
    void WindowActivated(uint32_t windowID);

protected:
    int windowButtons;

    GUI_TaskbarButton *pActiveWindowButton;
};


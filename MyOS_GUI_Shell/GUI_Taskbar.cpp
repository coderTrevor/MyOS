#ifndef __MYOS
#include <stdio.h>
#else
#include "../MyOS_1/Interrupts/System_Calls.h"
#endif
#include "GUI_Taskbar.h"
#include "GUI_Button.h"


GUI_Taskbar::GUI_Taskbar(uint32_t desktopWidth, uint32_t desktopHeight) : 
    GUI_Window(desktopHeight - GUI_TASKBAR_HEIGHT, 0, desktopWidth, GUI_TASKBAR_HEIGHT, "Taskbar")
{
    // taskbar has no system menu or "standard window controls

    backgroundColor = GUI_TASKBAR_COLOR;

    // Draw the background
    FillSurface(backgroundColor);

    // Draw a 3d border
    Draw3D_Box(pSurface, 0, 0, dimensions.width, dimensions.height);

    // Add a "Start" button control
    pControls[0] = new GUI_Button("MyOS", GUI_TASKBAR_START_ID, this);
    pControls[0]->dimensions.left = GUI_TASKBAR_BUTTON_MARGINS;

    windowButtons = 0;
    pActiveWindowButton = NULL;
}


GUI_Taskbar::~GUI_Taskbar()
{
}

void GUI_Taskbar::AddWindow(uint32_t windowID, GUI_Window *pWindow)
{
    // Add button for window

    // Find an index for the window
    int i;
    for (i = 1; i < MAX_WINDOW_CONTROLS; ++i)
    {
        if (!pControls[i])
            break;
    }

    if (i == MAX_WINDOW_CONTROLS)
    {
        printf("Error: Too many windows to add one to taskbar!\n");
        return;
    }

    // Get the x position for the new window
    int x = pControls[i - 1]->dimensions.left + pControls[i - 1]->dimensions.width + GUI_TASKBAR_BUTTON_MARGINS;
    ++windowButtons;
    // Set placement of new button
    GUI_Rect buttonPlacement = { GUI_TASKBAR_BUTTON_MARGINS,
                                 x,
                                 GUI_TASKBAR_BUTTON_MAX_WIDTH,
                                 GUI_TASKBAR_HEIGHT - (GUI_TASKBAR_BUTTON_MARGINS * 2) };

    pControls[i] = new GUI_TaskbarButton(pWindow->windowName, windowID, this, buttonPlacement);
    WindowActivated(windowID);
}

void GUI_Taskbar::ControlClicked(uint32_t controlID)
{
    if (controlID != GUI_TASKBAR_START_ID)
    {
        if (pActiveWindowButton)
            pActiveWindowButton->UnClick();

        BringWindowID_ToFront(controlID);
        // Find the control ID
        for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
        {
            pActiveWindowButton = (GUI_TaskbarButton *)pClickedControl;
            return;
        }
    }
    else
    {
        // start clicked
    }
}

void GUI_Taskbar::OnDrag(int startRelX, int startRelY, int relX, int relY)
{
    // Don't allow the taskbar itself to be dragged
}

void GUI_Taskbar::RemoveWindow(uint32_t windowID)
{
    // Find the control ID
    for (int i = 1; i < MAX_WINDOW_CONTROLS; ++i)
    {
        if (pControls[i] && pControls[i]->controlID == windowID)
        {
            GUI_TaskbarButton *pButton = (GUI_TaskbarButton *)pControls[i];
            
            // Remove all references to pButton
            pControls[i] = NULL;
            if (pActiveWindowButton == pButton)
                pActiveWindowButton = NULL;
            if (pClickedControl == pButton)
                pClickedControl = NULL;

            delete pButton;
            
            break;
        }
    }

    // We need to redraw the taskbar background to erase the old button
    FillSurface(backgroundColor);

    // Draw a 3d border
    Draw3D_Box(pSurface, 0, 0, dimensions.width, dimensions.height);

    --windowButtons;
}

void GUI_Taskbar::WindowActivated(uint32_t windowID)
{
    // Find the control ID
    for (int i = 1; i < MAX_WINDOW_CONTROLS; ++i)
    {
        if (pControls[i]->controlID == windowID)
        {
            // if there's an active window button that's not the same button, de-highlight it
            if (pActiveWindowButton && pActiveWindowButton != (GUI_TaskbarButton *)pControls[i])
                pActiveWindowButton->UnClick();
            
            // highlight the button for the new window
            pActiveWindowButton = (GUI_TaskbarButton *)pControls[i];
            pActiveWindowButton->Highlight();
        
            return;
        }
    }
}

void GUI_TaskbarButton::Highlight()
{
    Draw3D_InsetBox(pSurface, 0, 0, dimensions.width, dimensions.height);
}

void GUI_TaskbarButton::OnMouseUp(int relX, int relY)
{
    // Clicked taskbar windows should stay active
}

void GUI_TaskbarButton::UnClick()
{
    Draw3D_Box(pSurface, 0, 0, dimensions.width, dimensions.height);
}

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
    pControls[0] = new GUI_Button("MyOS", 0, this);
    pControls[0]->dimensions.left = GUI_TASKBAR_BUTTON_MARGINS;
}


GUI_Taskbar::~GUI_Taskbar()
{
}

void GUI_Taskbar::OnDrag(int startRelX, int startRelY, int relX, int relY)
{
    // Don't allow the taskbar itself to be dragged
}

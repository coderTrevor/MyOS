#include "GUI_RunWindow.h"
#include "MyOS_GUI_Shell.h"
#include "GUI_Button.h"
#include "GUI_EditControl.h"

// TODO: place window relative to bottom-left
GUI_RunWindow::GUI_RunWindow() : GUI_Window(0, 0, RUN_WINDOW_WIDTH, RUN_WINDOW_HEIGHT, "Run" )
{
    // Create a button control for an "OK" button
    pControls[0] = new GUI_Button("OK", SYSTEM_MENU_CLOSE_BUTTON_ID, this);

    // Move button to bottom-right corner
    pControls[0]->dimensions.left = dimensions.width - pControls[0]->dimensions.width - 2;
    pControls[0]->dimensions.top = dimensions.height - pControls[0]->dimensions.height - 2;

    pControls[1] = new GUI_EditControl(RunWindowEditBoxDimensions, this, 1);

    AddNewWindow(this);
}


GUI_RunWindow::~GUI_RunWindow()
{

}

#include "GUI_RunWindow.h"
#include "MyOS_GUI_Shell.h"
#include "GUI_Button.h"
#include "GUI_EditControl.h"

// TODO: place window relative to bottom-left
GUI_RunWindow::GUI_RunWindow() : GUI_Window(0, 0, RUN_WINDOW_WIDTH, RUN_WINDOW_HEIGHT, "Run", WINDOW_STYLE_NORMAL)
{
    // Create a button control for an "OK" button
    pControls[0] = new GUI_Button("OK", BUTTON_ID_OK, this);

    // Move button to bottom-right corner
    pControls[0]->dimensions.left = dimensions.width - pControls[0]->dimensions.width - 2;
    pControls[0]->dimensions.top = dimensions.height - pControls[0]->dimensions.height - 2;

    pControls[1] = new GUI_EditControl(RunWindowEditBoxDimensions, this, 1);

    focusedControlIndex = 1;
    enterClicksButtonID = BUTTON_ID_OK;

    AddNewWindow(this);
}


GUI_RunWindow::~GUI_RunWindow()
{

}

void GUI_RunWindow::ControlClicked(uint32_t controlID)
{
    // We only care about the OK button
    if (controlID != BUTTON_ID_OK)
        return;
    
    // Get the edit control
    GUI_EditControl *pEdit = (GUI_EditControl*)pControls[1];
    
    // Launch the requested app
    Shell_Launch_App(pEdit->stringContents);

    // Tell the shell to destroy this window
    Shell_Destroy_Window(this);
}

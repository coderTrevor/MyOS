#pragma once
#include "GUI_Window.h"

// TEMP TEMP (Maybe)
#define MAX_MESSAGE_LENGTH  256

// TEMP TEMP
#define MESSAGE_BOX_X       250
#define MESSAGE_BOX_Y       225
#define MESSAGE_BOX_WIDTH   300
#define MESSAGE_BOX_HEIGHT  150

class GUI_MessageBox : public GUI_Window
{
public:
    GUI_MessageBox(char *messageText, char *windowTitle);

private:
    char messageText[MAX_MESSAGE_LENGTH];
};

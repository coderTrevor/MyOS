#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Kernel_Shell.h"
#include "MyOS_GUI_Shell.h"
#include "../MyOS_1/GUI_Messages.h"

extern int lastWindowID;

void GUI_Kernel_Callback(uint32_t PID, uint32_t messageType, void * pData)
{
    GUI_Window *pWindow = NULL;

    switch (messageType)
    {
        case GUI_MSG_NEW_CONSOLE_APP:
            // Create a new window with the app name
            CreateTextWindow(PID,
                            ((GUI_NEW_CONSOLE_APP_DATA *)pData)->appName);
            break;

        case GUI_MSG_CONSOLE_PRINT:
            // Send the text to the window
            
            // Find the window associated with the PID
            pWindow = GetWindowFromID(PID);
            if (!pWindow)
            {
                MessageBox("GUI_Kernel_Callback called with unrecognized PID!", "ERROR");
                return;
            }
            pWindow->SendWindowText( ((GUI_CONSOLE_PRINT_DATA *)pData)->textString );
            break;

        default:
            MessageBox("GUI_Kernel_Callback called with unrecognized messageType.", "ERROR");
            break;
    }
}

#ifdef __cplusplus
}
#endif

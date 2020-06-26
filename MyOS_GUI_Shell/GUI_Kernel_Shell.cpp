#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Kernel_Shell.h"
#include "MyOS_GUI_Shell.h"
#include "../MyOS_1/GUI_Messages.h"

extern int lastWindowID;

void GUI_Kernel_Callback(uint32_t PID, uint32_t messageType, void * pData)
{
    switch (messageType)
    {
        case GUI_MSG_NEW_CONSOLE_APP:
            // Create a new window with the app name
            CreateTextWindow(PID,
                             //lastWindowID++,
                             ((GUI_NEW_CONSOLE_APP_DATA *)pData)->appName);
            break;
        default:
            MessageBox("GUI_Kernel_Callback called with unrecognized messageType.", "ERROR");
            break;
    }
}

#ifdef __cplusplus
}
#endif

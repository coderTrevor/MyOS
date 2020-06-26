#include "GUI_Kernel.h"
#include "GUI_Messages.h"
#include "Tasks/Context.h"

GUI_CALLBACK guiCallback = NULL;

void GUI_CallbackAdded()
{
    // Send a new window message to the GUI shell for each running application
    for (int i = 0; i < MAX_TASKS; ++i)
    {
        if (!tasks[i].inUse)
            continue;

        // Construct the message data
        GUI_NEW_CONSOLE_APP_DATA data;
        data.appName = tasks[i].imageName;

        (*guiCallback)(tasks[i].PID, GUI_MSG_NEW_CONSOLE_APP, &data);
    }
}

#include <stdarg.h>
#include "GUI_Kernel.h"
#include "GUI_Messages.h"
#include "Tasks/Context.h"
#include "printf.h"
#include "misc.h"

GUI_CALLBACK guiCallback = NULL;

// Send a formatted text string to the console window for the currently running application
int GUI_printf(const char *format, va_list va)
{
    GUI_CONSOLE_PRINT_DATA param;
    char outString[MAX_GUI_CONSOLE_STRING_LENGTH];

    int returnValue = vsnprintf(outString, MAX_GUI_CONSOLE_STRING_LENGTH, format, va);

    param.textString = outString;

    (*guiCallback)(tasks[currentTask].PID, GUI_MSG_CONSOLE_PRINT, &param);

    return returnValue;
}

void GUI_CallbackAdded()
{
    // Send a new window message to the GUI shell for each running application
    for (int i = 0; i < MAX_TASKS; ++i)
    {
        if (!tasks[i].inUse)
            continue;

        GUI_CreateConsoleWindowForApp(i);
    }
}

void GUI_CreateConsoleWindowForApp(uint32_t taskNumber)
{
    // Construct the message data
    GUI_NEW_CONSOLE_APP_DATA data;
    //data.appName = tasks[taskNumber].imageName;
    memset(data.appName, 0, MAX_IMAGE_NAME_LENGTH);
    strncpy(data.appName, tasks[taskNumber].imageName, MAX_IMAGE_NAME_LENGTH - 1);

    (*guiCallback)(tasks[taskNumber].PID, GUI_MSG_NEW_CONSOLE_APP, &data);
}

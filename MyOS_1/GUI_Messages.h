#pragma once
#include <stdint.h>

// MESSAGES
#define GUI_MSG_NEW_CONSOLE_APP 0x1000
#define GUI_MSG_CONSOLE_PRINT   0x1001
// END OF MESSAGES

#define MAX_GUI_CONSOLE_STRING_LENGTH   512

// DATA TYPES USED BY MESSAGES

// Data type used by GUI_MSG_NEW_CONSOLE_APP messages
typedef struct GUI_NEW_CONSOLE_APP_DATA
{
    const char *appName;
    // TODO: We may as well send the terminal contents over in the same message
} GUI_NEW_CONSOLE_APP_DATA;

// Data type used by GUI_MSG_CONSOLE_PRINT
typedef struct GUI_CONSOLE_PRINT_DATA
{
    const char *textString;
} GUI_CONSOLE_PRINT_DATA;
// END OF DATA TYPES
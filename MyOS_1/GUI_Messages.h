#pragma once
#include <stdint.h>

// MESSAGES
#define GUI_MSG_NEW_CONSOLE_APP 0x1000
// END OF MESSAGES

// DATA TYPES USED BY MESSAGES

// Data type used by GUI_MSG_NEW_CONSOLE_APP messages
typedef struct GUI_NEW_CONSOLE_APP_DATA
{
    const char *appName;

    // TODO: We may as well send the terminal contents over in the same message
} GUI_NEW_CONSOLE_APP_DATA;
// END OF DATA TYPES
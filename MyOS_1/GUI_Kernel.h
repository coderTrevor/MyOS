#pragma once
#include <stdint.h>

// definitions for interfacing between User apps <> kernel <> GUI shell

typedef void(*GUI_CALLBACK)(uint32_t PID, uint32_t messageType, void *pData);

extern GUI_CALLBACK guiCallback;

// Called immediately after a callback has been registered, to transition running apps into windowed consoles
void GUI_CallbackAdded();

// Sends the formatted text string to the running GUI application
int  GUI_printf(const char *format, va_list va);
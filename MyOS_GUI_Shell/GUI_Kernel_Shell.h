#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

// Used to define the interface between the Kernel and the GUI Shell (on the shell side)

#define \
GUI_BASE_ADDRESS 0xC00000

#define WINDOW_ID_IS_NOT_PID    0x80000000 /* Window ID's with this bit set aren't associated with an application */

// Called by the kernel to send messages to the GUI
void GUI_Kernel_Callback(uint32_t PID, uint32_t messageType, void *pData);

#ifdef __cplusplus
}
#endif /* __cplusplus */
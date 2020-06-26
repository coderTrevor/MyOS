#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

// Used to define the interface between the Kernel and the GUI Shell (on the shell side)

// Called by the kernel to send messages to the GUI
void GUI_Kernel_Callback(uint32_t PID, uint32_t messageType, void *pData);

#ifdef __cplusplus
}
#endif /* __cplusplus */
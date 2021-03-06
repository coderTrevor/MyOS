#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>
#include <stdint.h>
#include "../Graphics/Display_HAL.h"
#include "../Libs/SDL/include/SDL_rect.h"
#include "../myos_io.h"
#include "../Drivers/mouse.h"
#include "../GUI_Kernel.h"

#define SYSCALL_PRINT               255
#define SYSCALL_PRINTF              254
#define SYSCALL_GET_GRAPHICS_INFO   253
#define SYSCALL_PAGE_ALLOCATOR      252
#define SYSCALL_TIME_DELAY_MS       251
#define SYSCALL_GRAPHICS_BLIT       250
#define SYSCALL_FOPEN               249
#define SYSCALL_FCLOSE              248
#define SYSCALL_FREAD               247
#define SYSCALL_FSEEK               246
#define SYSCALL_FTELL               245
#define SYSCALL_EXIT_APP            244
#define SYSCALL_TIME_UPTIME_MS      243
#define SYSCALL_READ_FROM_KEYBOARD  242
#define SYSCALL_DISPATCH_NEW_TASK   241
#define SYSCALL_GET_MOUSE_STATE     240
#define SYSCALL_HIDE_SHELL_DISPLAY  239
#define SYSCALL_REGISTER_GUI_CALLBACK   238
#define SYSCALL_LAUNCH_APP          237
#define SYSCALL_CLOSE_APP           236

void SystemCallCloseApp(uint32_t PID);
#define closeApp SystemCallCloseApp

void SystemCallExit(int returnCode);
#define exit SystemCallExit

int SystemCallFClose(FILE *fp);
#define fclose SystemCallFClose

FILE *SystemCallFOpen(const char * filename, const char * mode);
#define fopen SystemCallFOpen

size_t SystemCallFRead(void * ptr, size_t size, size_t count, FILE * stream);
#define fread SystemCallFRead

int SystemCallFSeek(FILE * stream, long int offset, int origin);
#define fseek SystemCallFSeek

long int SystemCallFTell(FILE * stream);
#define ftell SystemCallFTell

bool SystemCallLaunchApp(const char *appFileName, bool exclusive);
#define launchApp SystemCallLaunchApp

void SystemCallPageAllocator(unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pRreturnVal);
#define pageAllocator SystemCallPageAllocator

void SystemCallGetGraphicsInfo(bool *graphicsArePresent, int *width, int *height);
#define getGraphicsInfo SystemCallGetGraphicsInfo

MOUSE_STATE SystemCallGetMouseState();
#define getMouseState SystemCallGetMouseState

void SystemCallGraphicsBlit(const SDL_Rect *sourceRect, PIXEL_32BIT *image);
#define graphicsBlit SystemCallGraphicsBlit

void SystemCallHideShellDisplay();
#define hideShellDisplay SystemCallHideShellDisplay

void SystemCallPrint(char *str);

bool SystemCallReadFromKeyboard(uint16_t *key);
#define readFromKeyboard SystemCallReadFromKeyboard

void SystemCallRegisterGuiCallback(GUI_CALLBACK guiCallback);
#define registerGuiCallback SystemCallRegisterGuiCallback

#define printf SystemCallPrintf
int __cdecl SystemCallPrintf(const char* format, ...);

void SystemCallTimeDelayMS(uint32_t milliSeconds);
#define timeDelayMS SystemCallTimeDelayMS

uint32_t SystemCallTimeGetUptimeMS();
#define timeGetUptimeMS SystemCallTimeGetUptimeMS


#ifdef __cplusplus
}
#endif /* __cplusplus */
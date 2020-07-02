/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2019 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"
#include <stdbool.h>

#if SDL_VIDEO_DRIVER_MYOS

#include "../../../Interrupts/System_Calls.h"

/* Video driver for MyOS. Based on Dummy driver.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_myosvideo.h"
#include "SDL_myosevents_c.h"
#include "SDL_myosframebuffer_c.h"

#define MYOSVID_DRIVER_NAME "myos"

/* Initialization/Query functions */
static int MYOS_VideoInit(_THIS);
static int MYOS_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void MYOS_VideoQuit(_THIS);

/* MYOS driver bootstrap functions */

static int
MYOS_Available(void)
{
    return (1);
}

static void
MYOS_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device);
}

static SDL_VideoDevice *
MYOS_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        printf("Couldn't create device\n");
        SDL_OutOfMemory();
        return (0);
    }
    device->is_dummy = SDL_FALSE;

    /* Set the function pointers */
    device->VideoInit = MYOS_VideoInit;
    device->VideoQuit = MYOS_VideoQuit;
    device->SetDisplayMode = MYOS_SetDisplayMode;
    device->PumpEvents = MYOS_PumpEvents;
    device->CreateWindowFramebuffer = SDL_MYOS_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = SDL_MYOS_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = SDL_MYOS_DestroyWindowFramebuffer;

    device->free = MYOS_DeleteDevice;

    printf("MYOS_CreateDevice called\n");

    return device;
}

VideoBootStrap MYOS_bootstrap = {
    MYOSVID_DRIVER_NAME, "SDL MyOS video driver",
    MYOS_Available, MYOS_CreateDevice
};


int
MYOS_VideoInit(_THIS)
{
    SDL_DisplayMode mode;

    printf("Using MyOS driver\n");

    bool hasGraphics;
    int height, width;
    getGraphicsInfo(&hasGraphics, &width, &height);

    if (!hasGraphics)
    {
        printf("GFX subsystem must be initialized first.\n");
        return -1;
    }

    /* Use a fake 32-bpp desktop mode */
    mode.format = SDL_PIXELFORMAT_BGRA8888;
    mode.w = width;
    mode.h = height;
    mode.refresh_rate = 0;
    mode.driverdata = NULL;
    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    MyOS_InitKeyboard();

    /* We're done! */
    return 0;
}

static int
MYOS_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

void
MYOS_VideoQuit(_THIS)
{
}

#endif /* SDL_VIDEO_DRIVER_MYOS */

/* vi: set ts=4 sw=4 expandtab: */

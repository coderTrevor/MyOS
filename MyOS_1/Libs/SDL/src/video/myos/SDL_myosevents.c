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
#include "SDL_myoskeyboard.h"

#if SDL_VIDEO_DRIVER_MYOS

/* Event driver for MyOS */

#include "../../events/SDL_events_c.h"
#include "../../../Interrupts/System_Calls.h"
#include "../../../Drivers/mouse.h"

#include "SDL_myosvideo.h"
#include "SDL_myosevents_c.h"

MOUSE_STATE oldMouseState = { 0, 0, false, false, false };

void
MYOS_PumpEvents(_THIS)
{
    // Check for keyboard events
    uint16_t sc = 0;
    unsigned char scancode;

    while (readFromKeyboard(&sc))
    {
        scancode = sc & 0xff;

        unsigned char keyRelease = (0x80 & scancode);

        scancode = (0x7F & scancode);

        if (keyRelease)
            SDL_SendKeyboardKey(SDL_RELEASED, MyOS_Keycodes[scancode]);
        else
            SDL_SendKeyboardKey(SDL_PRESSED, MyOS_Keycodes[scancode]);
    }

    // Check for mouse motion
    MOUSE_STATE mouseState = getMouseState();
    if (mouseState.mouseX != oldMouseState.mouseX
        || mouseState.mouseY != oldMouseState.mouseY)
    {
        // send mouse motion
        SDL_SendMouseMotion(NULL, 0, SDL_FALSE, mouseState.mouseX, mouseState.mouseY);
    }

    // Check for mouse buttons changing
    // left mouse button
    if (mouseState.leftButton != oldMouseState.leftButton)
    {
        if (mouseState.leftButton)
            SDL_SendMouseButton(NULL, 0, SDL_PRESSED, SDL_BUTTON_LEFT);
        else
            SDL_SendMouseButton(NULL, 0, SDL_RELEASED, SDL_BUTTON_LEFT);
    }
    // middle mouse button
    if (mouseState.middleButton != oldMouseState.middleButton)
    {
        if (mouseState.middleButton)
            SDL_SendMouseButton(NULL, 0, SDL_PRESSED, SDL_BUTTON_MIDDLE);
        else
            SDL_SendMouseButton(NULL, 0, SDL_RELEASED, SDL_BUTTON_MIDDLE);
    }
    // right mouse button
    if (mouseState.rightButton != oldMouseState.rightButton)
    {
        if (mouseState.rightButton)
            SDL_SendMouseButton(NULL, 0, SDL_PRESSED, SDL_BUTTON_RIGHT);
        else
            SDL_SendMouseButton(NULL, 0, SDL_RELEASED, SDL_BUTTON_RIGHT);
    }

    oldMouseState = mouseState;
}

#endif /* SDL_VIDEO_DRIVER_MYOS */

/* vi: set ts=4 sw=4 expandtab: */

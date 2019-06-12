// TestApp1.cpp : A super-simple application for testing with MyOS
//

//#include "stdafx.h"
#ifndef WIN32
#include "../MyOS_1/Console_VGA.h"
#include <intrin.h>
#include "../MyOS_1/Interrupts/System_Calls.h"
//#undef _MSVC_VER
#undef _WIN32
#define HAVE_LIBC 1
#define SDL_ATOMIC_DISABLED 1
#define SDL_EVENTS_DISABLED 1
#define SDL_TIMERS_DISABLED 1
#define HAVE_MALLOC 1
#else
#include <stdio.h>
#endif

#define SDL_MAIN_HANDLED 1

#include "../MyOS_1/Libs/SDL/include/SDL.h"
#include "../MyOS_1/Libs/SDL/include/SDL_video.h"

int main(int argc, char* argv[]) 
{
    SDL_Window *window;                    // Declare a pointer
                                           //The window we'll be rendering to

    SDL_Surface* screenSurface = NULL;     // the surface to render to
    
    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

                                           // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        800,                               // width, in pixels (ignored)
        600,                               // height, in pixels (ignored
        SDL_WINDOW_FULLSCREEN_DESKTOP      // Make the window the same size as the desktop
    );

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    // The window is open: could enter program loop here (see SDL_PollEvent())
    
    // Get window surface
    screenSurface = SDL_GetWindowSurface(window);

    // Fill the surface green
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0xFF, 0x7F));

    // Update the surface
    SDL_UpdateWindowSurface(window);

    SDL_Delay(500);  // Pause execution for 500 milliseconds, for example

    // Load BMP
    char *filename = "kghrwide.bmp";
#if __MYOS__
    filename = "kg2.bmp";
#endif
    SDL_Surface *bitmapSurface = SDL_LoadBMP(filename);
    if (bitmapSurface == NULL)
    {
        printf("Unable to load image %s! SDL Error: %s\n", filename, SDL_GetError());
        //success = false;
    }

    if (bitmapSurface)
    {
        // Blit bitmap to window
        SDL_BlitScaled(bitmapSurface, NULL, screenSurface, NULL);

        // Update the surface
        SDL_UpdateWindowSurface(window);

        SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);
    
    // Free BMP surface
    SDL_FreeSurface(bitmapSurface);

    // Clean up
    SDL_Quit();

    printf("Done with SDL Test Program.\n");

    return 0;
}
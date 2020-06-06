#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Window.h"

#ifdef __MYOS
#include "../MyOS_1/Interrupts/System_Calls.h"
#else
#include <stdio.h>
#endif


void GUI_Window::CreateSurface()
{
    pSurface = SDL_CreateRGBSurface(0, // flags (unused)
                                    dimensions.width,
                                    dimensions.height,
                                    32, // bit-depth
                                    0,  // Rmask (default)
                                    0,  // Gmask (default)
                                    0,  // Bmask (default),
                                    0); // Amask*/

    if (!pSurface)
        return;
     //   printf("Error! Couldn't create RGB surface for window!\n");

    backgroundColor.r = backgroundColor.b = 50;
    backgroundColor.b = 255;
    DrawWindow();
}

void GUI_Window::PaintToSurface(SDL_Surface *pTargetSurface)
{
    SDL_BlitSurface(pSurface, NULL, pTargetSurface, dimensions.GetSDL_Rect());
}

void GUI_Window::SetBackgroundColor(SDL_Color color)
{
    backgroundColor = color;
    DrawWindow();
}

void GUI_Window::DrawWindow()
{
    // Draw the background
    SDL_FillRect(pSurface, NULL, SDL_MapRGB(pSurface->format, backgroundColor.r, backgroundColor.g, backgroundColor.b));

    // Draw the system menu at the top
    DrawSystemMenu(pSurface, windowName);

    // Draw a black outline around the background
    SDL_Color black;
    black.r = black.g = black.b = 0;
    DrawBox(pSurface, 0, 0, dimensions.width - 1, dimensions.height - 1, black);
}

#ifdef __cplusplus
};
#endif /* __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Window.h"
#include "MyOS_GUI_Shell.h"

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

    backgroundColor.r = backgroundColor.b = 205;
    backgroundColor.b = 205;
    DrawWindow();
}

void GUI_Window::PaintToSurface(SDL_Surface *pTargetSurface)
{
    // TEMP
    for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
    {
        if (pControls[i])
            pControls[i]->PaintToSurface(pSurface);
    }

    SDL_BlitSurface(pSurface, NULL, pTargetSurface, dimensions.GetSDL_Rect());
}

void GUI_Window::ControlClicked(uint32_t controlID)
{
    if (controlID == SYSTEM_MENU_CLOSE_BUTTON_ID)
    {
        // TODO: Send exit signal / message to any associated app

        // Tell the shell to destroy this window
        Shell_Destroy_Window(this);
    }
}

void GUI_Window::OnClick(int relX, int relY)
{
    if (relX < 0 || relY < 0 || relX > dimensions.width || relY > dimensions.height)
    {
        printf("Window OnClick() called outside of range!\n");
        return;
    }
    
    // Pass the click onto each control
    for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
    {
        if (pControls[i] && pControls[i]->PointInBounds(relX, relY))
            pControls[i]->OnClick(relX - pControls[i]->dimensions.left, relY - pControls[i]->dimensions.top);
    }
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
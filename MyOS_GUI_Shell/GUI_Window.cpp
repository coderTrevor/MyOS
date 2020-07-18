#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Window.h"
#include "MyOS_GUI_Shell.h"
#include "GUI_SystemMenuControl.h"

#ifdef __MYOS
#include "../MyOS_1/Interrupts/System_Calls.h"
#else
#include <stdio.h>
#endif


GUI_Window::GUI_Window(GUI_Rect size, const char *name, int style) : GUI_Window(size.top, size.left, size.width, size.height, name, style)
{
}

GUI_Window::GUI_Window(int top, int left, int width, int height, const char *name, int windowStyle)
{
    dimensions.top = top;
    dimensions.left = left;
    dimensions.width = width;
    dimensions.height = height;

    SDL_strlcpy(windowName, name, MAX_WINDOW_NAME_LENGTH);
    CreateSurface();

    style = windowStyle;

    for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
        pControls[i] = NULL;

    // Should we create a system menu control?
    if (!(style & WINDOW_STYLE_NO_SYSTEM_MENU))
    {
        // create system menu
        pControls[MAX_WINDOW_CONTROLS - 1] = new GUI_SystemMenuControl(this);
    }

    pClickedControl = NULL;
    focusedControlIndex = -1;
    enterClicksButtonID = -1;
}

GUI_Window::~GUI_Window()
{
    if (!pSurface)
        SDL_FreeSurface(pSurface);
}

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

    backgroundColor = { 205, 205, 205, 255 };

    DrawWindow();
}

void GUI_Window::SetFocus(int controlID)
{
    // Ignore if this control alread has the focus
    if (focusedControlIndex >= 0 && pControls[focusedControlIndex]->controlID == controlID)
        return;

    // Unfocus the previously focused control
    if (focusedControlIndex >= 0)
        pControls[focusedControlIndex]->LoseFocus();
        
    // Find the control with the given ID and focus it
    for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
    {
        if (pControls[i] && pControls[i]->controlID == controlID)
        {
            focusedControlIndex = i;
            return;
        }
    }

    MessageBox("Couldn't find a matching control for SetFocus", "ERROR");
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

void GUI_Window::UpdateCursor()
{
    if (focusedControlIndex < 0)
        return;

    pControls[focusedControlIndex]->UpdateCursor();
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

// Fill the entire window with a color
void GUI_Window::FillSurface(SDL_Color color)
{
    uint32_t col = SDL_MapRGB(pSurface->format, color.r, color.g, color.b);
    SDL_FillRect(pSurface, NULL, col);
}

void GUI_Window::LoseFocus()
{
    if (focusedControlIndex < 0)
        return;

    pControls[focusedControlIndex]->LoseFocus();
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
        {
            pClickedControl = pControls[i];
            pControls[i]->OnClick(relX - pControls[i]->dimensions.left, relY - pControls[i]->dimensions.top);
        }
    }
}

void GUI_Window::OnDrag(int startRelX, int startRelY, int relX, int relY)
{
    dimensions.left += relX - startRelX;
    dimensions.top += relY - startRelY;
}

void GUI_Window::OnMouseUp(int relX, int relY)
{
    if (pClickedControl)
        pClickedControl->OnMouseUp(relX - pClickedControl->dimensions.left, relY - pClickedControl->dimensions.top);
}

void GUI_Window::Resize(GUI_Rect newDimensions)
{
    SDL_FreeSurface(pSurface);

    dimensions = newDimensions;

    CreateSurface();

    DrawWindow();
}

void GUI_Window::SetBackgroundColor(SDL_Color color)
{
    backgroundColor = color;
    DrawWindow();
}

void GUI_Window::SendChar(char c)
{
    if (c == SDLK_RETURN && enterClicksButtonID != -1)
    {
        ControlClicked(enterClicksButtonID);
        return;
    }

    if (focusedControlIndex < 0)
        return;

    pControls[focusedControlIndex]->SendChar(c);
}

void GUI_Window::DrawWindow()
{
    // Draw the background
    FillSurface(backgroundColor);

    // Draw the system menu at the top
    //DrawSystemMenu(pSurface, windowName);

    // Draw a black outline around the background
    Draw3D_Box(pSurface, 0, 0, dimensions.width, dimensions.height);
}

#ifdef __cplusplus
};
#endif /* __cplusplus */
#pragma once
#include "GUI_Object.h"
#include "GUI_Rect.h"
#include "GUI_Control.h"

#define MAX_WINDOW_NAME_LENGTH  64
#define SYSTEM_MENU_CLOSE_BUTTON_ID (uint32_t)-1
#define MAX_WINDOW_CONTROLS     8   /* TEMP */

class GUI_Control;

class GUI_Window : public GUI_Object
{
public:
    GUI_Window(GUI_Rect size, char *name)
    {
        dimensions = size;
        SDL_strlcpy(windowName, name, MAX_WINDOW_NAME_LENGTH);
        CreateSurface();

        for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
            pControls[i] = NULL;
    }

    GUI_Window(int top, int left, int width, int height, char *name)
    {
        dimensions.top = top;
        dimensions.left = left;
        dimensions.width = width;
        dimensions.height = height;

        SDL_strlcpy(windowName, name, MAX_WINDOW_NAME_LENGTH);
        CreateSurface();

        for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
            pControls[i] = NULL;
    }

    virtual void ControlClicked(uint32_t controlID);

    void OnClick(int relX, int relY);

    void OnDrag(int startRelX, int startRelY, int relX, int relY);

    void SetBackgroundColor(SDL_Color color);

    SDL_Surface *GetSurface() { return pSurface; }
    void PaintToSurface(SDL_Surface *pTargetSurface);

protected:
    void GUI_Window::CreateSurface();
    SDL_Surface *pSurface;
    SDL_Color backgroundColor;
    void DrawWindow();
    char windowName[MAX_WINDOW_NAME_LENGTH];
    GUI_Control *pControls[MAX_WINDOW_CONTROLS];
};

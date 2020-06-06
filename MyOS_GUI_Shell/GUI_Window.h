#pragma once
#include "GUI_Object.h"
#include "GUI_Rect.h"

#define MAX_WINDOW_NAME_LENGTH  64

class GUI_Window : public GUI_Object
{
public:
    GUI_Window(GUI_Rect size)
    {
        dimensions = size;
        CreateSurface();
        SDL_strlcpy(windowName, "test", MAX_WINDOW_NAME_LENGTH);
    }

    GUI_Window(int top, int left, int height, int width)
    {
        dimensions.top = top;
        dimensions.left = left;
        dimensions.height = height;
        dimensions.width = width;

        SDL_strlcpy(windowName, "test", MAX_WINDOW_NAME_LENGTH);
        CreateSurface();
    }

    void SetBackgroundColor(SDL_Color color);

    SDL_Surface *GetSurface() { return pSurface; }
    void PaintToSurface(SDL_Surface *pTargetSurface);

private:
    void GUI_Window::CreateSurface();
    SDL_Surface *pSurface;
    SDL_Color backgroundColor;
    void DrawWindow();
    char windowName[MAX_WINDOW_NAME_LENGTH];
};

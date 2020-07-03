#pragma once
#include "GUI_Object.h"
#include "GUI_Rect.h"
#include "GUI_Control.h"
#include "SDL_picofont.h"

#define MAX_WINDOW_NAME_LENGTH  64
#define SYSTEM_MENU_CLOSE_BUTTON_ID (uint32_t)-1
#define BUTTON_ID_OK                (uint32_t)-2
#define MAX_WINDOW_CONTROLS     24   /* TEMP */

class GUI_Control;

class GUI_Window : public GUI_Object
{
public:
    GUI_Window(GUI_Rect size, const char *name)
    {
        dimensions = size;
        SDL_strlcpy(windowName, name, MAX_WINDOW_NAME_LENGTH - 1);
        CreateSurface();

        for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
            pControls[i] = NULL;

        pClickedControl = NULL;
        focusedControlIndex = -1;
    }

    GUI_Window(int top, int left, int width, int height, const char *name)
    {
        dimensions.top = top;
        dimensions.left = left;
        dimensions.width = width;
        dimensions.height = height;

        SDL_strlcpy(windowName, name, MAX_WINDOW_NAME_LENGTH);
        CreateSurface();

        for (int i = 0; i < MAX_WINDOW_CONTROLS; ++i)
            pControls[i] = NULL;

        pClickedControl = NULL;
        focusedControlIndex = -1;
    }

    virtual void ControlClicked(uint32_t controlID);

    void DrawWindow();

    void FillSurface(SDL_Color color);

    virtual void LoseFocus();

    void OnClick(int relX, int relY);

    void OnDrag(int startRelX, int startRelY, int relX, int relY);

    void OnMouseUp(int relX, int relY);

    virtual void Resize(GUI_Rect newDimensions);

    void SetBackgroundColor(SDL_Color color);

    void SendChar(char c);

    virtual void SendWindowText(const char *text)
    {
        (void)text; // text sent to GUI_Window base class is ignored
    }

    void SetFocus(int controlID);

    SDL_Surface *GetSurface() { return pSurface; }
    
    void PaintToSurface(SDL_Surface *pTargetSurface);

    void UpdateCursor();


    SDL_Surface *pSurface;
    GUI_Control *pClickedControl;
    char windowName[MAX_WINDOW_NAME_LENGTH];
protected:
    void GUI_Window::CreateSurface();
    SDL_Color backgroundColor;
    GUI_Control *pControls[MAX_WINDOW_CONTROLS];
    int focusedControlIndex;
};

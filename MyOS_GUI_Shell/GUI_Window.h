#pragma once
#include "GUI_Object.h"
#include "GUI_Rect.h"
#include "GUI_Control.h"
#include "SDL_picofont.h"

#define MAX_WINDOW_NAME_LENGTH  64
#define SYSTEM_MENU_CLOSE_BUTTON_ID (uint32_t)-1
#define BUTTON_ID_OK                (uint32_t)-2
#define SYSTEM_MENU_CONTROL_ID      (uint32_t)-3
#define MAX_WINDOW_CONTROLS     24   /* TEMP */

#define WINDOW_STYLE_NORMAL         0
#define WINDOW_STYLE_NO_SYSTEM_MENU 1

class GUI_Control;

class GUI_Window : public GUI_Object
{
public:
    GUI_Window(GUI_Rect size, const char *name, int windowStyle);

    GUI_Window(int top, int left, int width, int height, const char *name, int windowStyle);

    ~GUI_Window();

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
    int enterClicksButtonID;        // There's probably a better way to do this
    int style;
};

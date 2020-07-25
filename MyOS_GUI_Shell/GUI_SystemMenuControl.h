#pragma once
#include "GUI_Control.h"
#include "GUI_Button.h"

class GUI_SystemMenuControl :
    public GUI_Control
{
public:
    GUI_SystemMenuControl(GUI_Window *pOwner);
    ~GUI_SystemMenuControl();

    void CreateSurface();
    void OnClick(int relX, int relY);
    void PaintToSurface(SDL_Surface *pTargetSurface);

    SDL_Surface *pSurface;

    GUI_Button closeButton;
};


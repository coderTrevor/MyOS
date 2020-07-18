#pragma once
#include "GUI_Control.h"

class GUI_SystemMenuControl :
    public GUI_Control
{
public:
    GUI_SystemMenuControl(GUI_Window *pOwner);
    ~GUI_SystemMenuControl();

    void CreateSurface();
    void PaintToSurface(SDL_Surface *pTargetSurface);

    SDL_Surface *pSurface;
};


#pragma once
#include "GUI_Control.h"

class GUI_EditControl :
    public GUI_Control
{
public:
    GUI_EditControl(GUI_Rect guiRect, GUI_Window *pOwner, uint32_t controlID);
    ~GUI_EditControl();

    void CreateSurface();
    void LoseFocus();
    void OnClick(int relX, int relY);
    void PaintToSurface(SDL_Surface *pTargetSurface);
    void UpdateCursor();

    SDL_Surface *pSurface;

    bool cursorBlinkOn;
    int cursorX;
};


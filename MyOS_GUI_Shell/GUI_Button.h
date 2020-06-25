#pragma once

#include "GUI_Control.h"

#define GUI_BUTTON_TEXT_MARGIN  8

class GUI_Button : public GUI_Control
{
public:
    GUI_Button(const char *buttonText, uint32_t controlID, GUI_Window *pOwner);
    GUI_Button(const char *buttonText, uint32_t controlID, GUI_Window *pOwner, GUI_Rect dimensions);
    
    ~GUI_Button()
    {
        SDL_free(buttonText);
    }

    void PaintToSurface(SDL_Surface * pTargetSurface);
    virtual void OnClick(int relX, int relY);
    virtual void OnMouseUp(int relX, int relY);
    void Resize(GUI_Rect newDimensions);    // TODO: Make dimensions a private variable

protected:
    SDL_Surface *CreateSurface();

    char *buttonText;
    SDL_Surface *pSurface;
};
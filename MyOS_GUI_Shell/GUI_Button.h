#pragma once

#include "GUI_Control.h"

#define GUI_BUTTON_TEXT_MARGIN  8

class GUI_Button : public GUI_Control
{
public:
    GUI_Button(const char *buttonText, uint32_t controlID, GUI_Window *pOwner);
    
    ~GUI_Button()
    {
        SDL_free(buttonText);
    }

    void PaintToSurface(SDL_Surface * pTargetSurface);

protected:
    char *buttonText;
    SDL_Surface *pSurface;
};
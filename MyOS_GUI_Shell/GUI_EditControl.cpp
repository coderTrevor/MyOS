#include "GUI_EditControl.h"

GUI_EditControl::GUI_EditControl(GUI_Rect guiRect, GUI_Window *pOwner, uint32_t controlID)
                                : GUI_Control(pOwner, controlID)
{
    pSurface = NULL;
    dimensions = guiRect;
    CreateSurface();
    cursorX = 2;
}


GUI_EditControl::~GUI_EditControl()
{
    SDL_FreeSurface(pSurface);
}

void GUI_EditControl::CreateSurface()
{
    if (pSurface)
        SDL_FreeSurface(pSurface);

    pSurface = SDL_CreateRGBSurface(0,
                                    dimensions.width,
                                    dimensions.height,
                                    32,
                                    0xFF000000,
                                    0x00FF0000,
                                    0x0000FF00,
                                    0x000000FF);

    FillSurface(pSurface, SDL_WHITE);

    Draw3D_InsetBox(pSurface, 0, 0, dimensions.width - 1, dimensions.height - 1);
}

void GUI_EditControl::LoseFocus()
{
    if(cursorBlinkOn)
        DrawVerticalLine(pSurface, cursorX, 1, dimensions.height - 2, SDL_WHITE);

    cursorBlinkOn = false;
}

void GUI_EditControl::OnClick(int relX, int relY)
{
    pOwner->SetFocus(controlID);
}

void GUI_EditControl::PaintToSurface(SDL_Surface * pTargetSurface)
{
    SDL_BlitSurface(pSurface, NULL, pTargetSurface, dimensions.GetSDL_Rect());
}

void GUI_EditControl::UpdateCursor()
{
    cursorBlinkOn = !cursorBlinkOn;

    if (cursorBlinkOn)
    {
        DrawVerticalLine(pSurface, cursorX, 1, dimensions.height - 2, SDL_BLACK);
    }
    else
    {
        DrawVerticalLine(pSurface, cursorX, 1, dimensions.height - 2, SDL_WHITE);
    }
}

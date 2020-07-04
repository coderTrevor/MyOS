#include "GUI_EditControl.h"
#include <string.h>
#include "MyOS_GUI_Shell.h"

GUI_EditControl::GUI_EditControl(GUI_Rect guiRect, GUI_Window *pOwner, uint32_t controlID)
                                : GUI_Control(pOwner, controlID)
{
    pSurface = NULL;
    dimensions = guiRect;
    CreateSurface();
    cursorX = 2;

    memset(stringContents, 0, GUI_EDIT_MAX_STRING_LENGTH);
    stringLength = 0;
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

// TODO: Handle horizontal scrolling
void GUI_EditControl::SendChar(char c)
{
    if (c == SDLK_LSHIFT || c == SDLK_RSHIFT)
        return;

    char str[2] = { 0 };
    str[0] = c;
    
    if (c == SDLK_BACKSPACE)
    {
        if (stringLength == 0)
            return;

        stringContents[stringLength--] = '\0';
        
        // Erase the previous cursor
        if (cursorBlinkOn)
            DrawVerticalLine(pSurface, cursorX, 1, dimensions.height - 2, SDL_WHITE);

        cursorBlinkOn = true;

        // Erase the previous character
        cursorX -= FNT_FONTWIDTH;
        SDL_Rect charRect = { cursorX, FNT_TOPBOTTOMMARGIN, FNT_FONTWIDTH, FNT_FONTHEIGHT };
        SDL_FillRect(pSurface, &charRect, 0xFFFFFFFF);  // fill the space with white

        // Redraw the cursor
        DrawVerticalLine(pSurface, cursorX, 1, dimensions.height - 2, SDL_BLACK);

        return;
    }

    if (stringLength >= GUI_EDIT_MAX_STRING_LENGTH)
    {
        MessageBox("Edit string is too long", "ERROR");
        return;
    }

    // Copy char to internal string
    stringContents[stringLength++] = c;

    // Erase the previous cursor
    if(cursorBlinkOn)
        DrawVerticalLine(pSurface, cursorX, 1, dimensions.height - 2, SDL_WHITE);
    
    cursorBlinkOn = true;

    // Render the char at the end of the string
    {
        SDL_Surface *pFont = FNT_Render(str, SDL_BLACK);

        SDL_Rect dest = { cursorX, FNT_TOPBOTTOMMARGIN, pFont->w, pFont->h };
        SDL_BlitSurface(pFont, NULL, pSurface, &dest);

        SDL_FreeSurface(pFont);
    }
    
    // Advance the cursor and redraw it
    cursorX += FNT_FONTWIDTH;
    DrawVerticalLine(pSurface, cursorX, 1, dimensions.height - 2, SDL_BLACK);
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

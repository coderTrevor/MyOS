#include "GUI_SystemMenuControl.h"
#include "GUI_Window.h"


GUI_SystemMenuControl::GUI_SystemMenuControl(GUI_Window *pOwner) : GUI_Control(pOwner, SYSTEM_MENU_CONTROL_ID), 
                                                                    closeButton("X", SYSTEM_MENU_CLOSE_BUTTON_ID, pOwner)
{
    dimensions.top = 1;
    dimensions.left = 1;
    dimensions.width = pOwner->dimensions.width - 2;
    dimensions.height = SYSTEM_MENU_HEIGHT;

    pSurface = NULL;
    CreateSurface();

    // Make close button red
    closeButton.backgroundColor = { 255, 120, 120, 255 };

    // Calculate close button dimensions
    int margin = 4;
    int boxHeight = SYSTEM_MENU_HEIGHT - margin + 1;
    int boxWidth = boxHeight;
    int boxTop = (margin / 2);
    int boxLeft = dimensions.width - boxWidth - margin + 2;
    SDL_Rect boxRect = { boxLeft, boxTop, boxWidth, boxHeight };
    
    closeButton.Resize({ boxTop, boxLeft, boxWidth, boxHeight });
}

GUI_SystemMenuControl::~GUI_SystemMenuControl()
{
    SDL_FreeSurface(pSurface);
}

void GUI_SystemMenuControl::CreateSurface()
{
    if (pSurface)
        SDL_FreeSurface(pSurface);


    pSurface = SDL_CreateRGBSurface(0,
                                    dimensions.width,
                                    dimensions.height,
                                    32,
                                    SDL_R_MASK,
                                    SDL_G_MASK,
                                    SDL_B_MASK,
                                    SDL_A_MASK);

    if (!pSurface)
    {
        //printf("Error! Couldn't allocate surface for system menu\n");
        return;
    }

    // Fill the surface with the menu color
    FillSurface(pSurface, SDL_SYS_MENU_COLOR);

    // Draw the title of the window
    SDL_Surface *pFont = FNT_Render(pOwner->windowName, SDL_BLACK);
    SDL_Rect destRect = { 8, 8, 8, 32 };
    SDL_BlitSurface(pFont, NULL, pSurface, &destRect);
    SDL_FreeSurface(pFont);

    // Draw the system boxes
    int margin = 4;
    int boxHeight = SYSTEM_MENU_HEIGHT - margin + 1;
    int boxWidth = boxHeight;
    int boxTop = (margin / 2);
    int boxLeft = dimensions.width - boxWidth - margin + 2;
    SDL_Rect boxRect = { boxLeft, boxTop, boxWidth, boxHeight };

    boxLeft = boxLeft - boxWidth - 2;
    boxRect = { boxLeft, boxTop, boxWidth, boxHeight };
    SDL_FillRect(pSurface, &boxRect, SDL_MapRGB(pSurface->format, 205, 205, 205));
    Draw3D_Box(pSurface, boxLeft, boxTop, boxWidth, boxHeight);

    pFont = FNT_Render("[", SDL_BLACK);
    boxRect.x += 4;
    boxRect.y += 7;
    SDL_BlitSurface(pFont, NULL, pSurface, &boxRect);
    boxRect.x += 4;
    SDL_FreeSurface(pFont);
    pFont = FNT_Render("]", SDL_BLACK);
    SDL_BlitSurface(pFont, NULL, pSurface, &boxRect);
    SDL_FreeSurface(pFont);

    boxLeft = boxLeft - boxWidth - 2;
    boxRect = { boxLeft, boxTop, boxWidth, boxHeight };
    SDL_FillRect(pSurface, &boxRect, SDL_MapRGB(pSurface->format, 205, 205, 205));
    Draw3D_Box(pSurface, boxLeft, boxTop, boxWidth, boxHeight);

    pFont = FNT_Render("_", SDL_BLACK);
    boxRect.x += 7;
    boxRect.y += 7;
    SDL_BlitSurface(pFont, NULL, pSurface, &boxRect);
    SDL_FreeSurface(pFont);
}

void GUI_SystemMenuControl::OnClick(int relX, int relY)
{
    // Pass click on to button controls
    if (closeButton.PointInBounds(relX, relY))
        closeButton.OnClick(closeButton.GetRelX(relX), closeButton.GetRelY(relY));
}

void GUI_SystemMenuControl::PaintToSurface(SDL_Surface * pTargetSurface)
{
    closeButton.PaintToSurface(pSurface);
    SDL_BlitSurface(pSurface, NULL, pTargetSurface, dimensions.GetSDL_Rect());
}

#include "GUI_PopupMenu.h"
#include "GUI_Window.h"
#include "SDL_picofont.h"
#include "MyOS_GUI_Shell.h"
#include <string.h>

GUI_PopupMenu::GUI_PopupMenu(GUI_Window *pOwner, MENU_HANDLER menuHandler, POPUP_TYPE popupType) : GUI_Control(pOwner, -1)
{
    choices = 0;
    handlerCallback = menuHandler;
    shown = false;
    this->popupType = popupType;
    pSurface = NULL;
}

GUI_PopupMenu::~GUI_PopupMenu()
{
}

void GUI_PopupMenu::AddMenuItem(char * choiceText, int choiceID)
{
    choiceIDs[choices] = choiceID;
    memset(choiceStrings[choices], 0, MENU_MAX_CHOICE_LENGTH);
    strncpy(choiceStrings[choices++], choiceText, MENU_MAX_CHOICE_LENGTH - 1);

    CreateSurface();
}

void GUI_PopupMenu::CreateSurface()
{
    if (pSurface)
    {
        SDL_FreeSurface;
        pSurface = NULL;
    }

    // determine maximum width of menu
    int maxChoiceLength = 1;
    for (int i = 0; i < choices; ++i)
    {
        if (strlen(choiceStrings[i]) > maxChoiceLength)
            maxChoiceLength = strlen(choiceStrings[i]);
    }

    int menuWidth = (maxChoiceLength * FNT_FONTWIDTH) + (2 * FNT_LEFTRIGHTMARGIN);
    choiceHeight = FNT_FONTHEIGHT;
    int menuHeight = (choices * choiceHeight) + ((choices - 1) * FNT_ROWSPACING) + (2 * FNT_TOPBOTTOMMARGIN);
    
    pSurface = SDL_CreateRGBSurface(0,
                                    menuWidth,
                                    menuHeight,
                                    32,
                                    0xFF000000,
                                    0x00FF0000,
                                    0x0000FF00,
                                    0x000000FF);

    dimensions.width = menuWidth;
    dimensions.height = menuHeight;

    Draw(-1);
}

void GUI_PopupMenu::Draw(int selectedChoice)
{
    // Fill the surface with the default color
    FillSurface(pSurface, SDL_DEFAULT_BUTTON_COLOR);

    // Draw the choices to the surface
    SDL_Rect textDest = { FNT_LEFTRIGHTMARGIN, FNT_TOPBOTTOMMARGIN, 0, 0 };

    int selectionY = FNT_TOPBOTTOMMARGIN;

    for (int i = 0; i < choices; ++i)
    {
        SDL_Surface *pFont;

        if (i == selectedChoice)
        {
            // Draw a box for the selection
            SDL_Rect selectedDim = { 1, selectionY, dimensions.width - 3, choiceHeight };
            SDL_FillRect(pSurface, &selectedDim, SDL_MapRGB(pSurface->format,
                                                            SDL_SYS_MENU_COLOR.r,
                                                            SDL_SYS_MENU_COLOR.g,
                                                            SDL_SYS_MENU_COLOR.b));
            // Draw the text in white
            pFont = FNT_Render(choiceStrings[i], SDL_WHITE);
        }
        else
        {
            pFont = FNT_Render(choiceStrings[i], SDL_BLACK);
            selectionY += choiceHeight + FNT_ROWSPACING;
        } 

        textDest.w = pFont->w;
        textDest.h = pFont->h;

        SDL_BlitSurface(pFont, NULL, pSurface, &textDest);

        textDest.y += choiceHeight + FNT_ROWSPACING;

        SDL_FreeSurface(pFont);
    }

    // Draw a border around the menu
    Draw3D_Box(pSurface, 0, 0, dimensions.width, dimensions.height);
}

bool GUI_PopupMenu::MouseOver(int relX, int relY)
{
    if (relX < 0 || relY < 0 || relX > dimensions.width || relY > dimensions.height)
    {
        MessageBox("Menu called with mouse out of position\n", "ERROR");
        return false;
    }

    // Highlight the appropriate option
    int heightPerOption = dimensions.height / choices;
    int selection = relY / heightPerOption;

    Draw(selection);

    return true;
}

void GUI_PopupMenu::PaintToSurface(SDL_Surface * pTargetSurface)
{
    if (!shown)
        return;

    SDL_BlitSurface(pSurface, NULL, pTargetSurface, dimensions.GetSDL_Rect());
}

void GUI_PopupMenu::RegisterMenuHandler(MENU_HANDLER handler)
{
    handlerCallback = handler;
}

void GUI_PopupMenu::ShowMenu(SDL_Point origin)
{
    if (!pSurface)
        return;

    dimensions.left = origin.x;
    
    if (popupType == ABOVE_AND_RIGHT_OF_ORIGIN)
        dimensions.top = origin.y - dimensions.height;

    shown = true;
}

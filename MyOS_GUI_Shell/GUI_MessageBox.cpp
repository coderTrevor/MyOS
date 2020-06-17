#include "GUI_MessageBox.h"
#include "GUI_Object.h"
#include "GUI_Button.h"

GUI_MessageBox::GUI_MessageBox(char * messageText, char * windowTitle)
    :   GUI_Window(MESSAGE_BOX_X, MESSAGE_BOX_Y, MESSAGE_BOX_WIDTH, MESSAGE_BOX_HEIGHT, windowTitle)
{
    // TODO: Use static memory so we can display out of memory messages
    SDL_Surface *pFont = FNT_Render(messageText, SDL_BLACK);
    uint32_t messageMargin = 8;
    
    SDL_Rect destRect = { messageMargin, 
        messageMargin + SYSTEM_MENU_HEIGHT,
        dimensions.width - (messageMargin * 2),
        dimensions.height - (messageMargin * 2) };
    
    SDL_BlitSurface(pFont, NULL, pSurface, &destRect);
    SDL_FreeSurface(pFont);

    // Create a button control for an "OK" button
    pControls[0] = new GUI_Button("OK", SYSTEM_MENU_CLOSE_BUTTON_ID, this);
}

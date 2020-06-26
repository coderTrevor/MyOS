#include "GUI_MessageBox.h"
#include "GUI_Object.h"
#include "GUI_Button.h"
#include "MyOS_GUI_Shell.h"

GUI_MessageBox::GUI_MessageBox(char * messageText, char * windowTitle)
    :   GUI_Window( NewWindowPosition(MESSAGE_BOX_WIDTH, MESSAGE_BOX_HEIGHT), windowTitle)
{
    // TODO: Use static memory so we can display out of memory messages
    SDL_Surface *pFont = FNT_Render(messageText, SDL_BLACK);
    const int messageMargin = 8;
    
    int minTextWidth = pFont->w + (messageMargin * 2);
    
    // Resize the message box if the text won't fit
    if (dimensions.width < minTextWidth)
        Resize( { dimensions.top, dimensions.left, minTextWidth, dimensions.height  } );

    SDL_Rect destRect = { messageMargin, 
        messageMargin + SYSTEM_MENU_HEIGHT,
        minTextWidth,
        dimensions.height - (messageMargin * 2) };
    
    SDL_BlitSurface(pFont, NULL, pSurface, &destRect);
    SDL_FreeSurface(pFont);

    // Create a button control for an "OK" button
    pControls[0] = new GUI_Button("OK", SYSTEM_MENU_CLOSE_BUTTON_ID, this);
}


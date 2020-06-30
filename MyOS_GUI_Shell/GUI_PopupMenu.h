#pragma once
#include "GUI_Control.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MENU_MAX_CHOICES        16
#define MENU_MAX_CHOICE_LENGTH  32

typedef enum POPUP_TYPE
{
    ABOVE_AND_RIGHT_OF_ORIGIN = 0
}POPUP_TYPE;

typedef void(*MENU_HANDLER)(int choiceID);

class GUI_PopupMenu :
    public GUI_Control
{
public:
    GUI_PopupMenu(GUI_Window *pOwner, MENU_HANDLER menuHandler, POPUP_TYPE popupType);
    ~GUI_PopupMenu();

    void AddMenuItem(char *Choice, int choiceID);
    void CreateSurface();
    void Draw(int selectedChoice);
    bool MouseOver(int relX, int relY);
    void PaintToSurface(SDL_Surface *pTargetSurface);
    void RegisterMenuHandler(MENU_HANDLER handler);
    void ShowMenu(SDL_Point origin);

    char choiceStrings[MENU_MAX_CHOICES][MENU_MAX_CHOICE_LENGTH];
    int choiceIDs[MENU_MAX_CHOICES];
    int choices;
    MENU_HANDLER handlerCallback;
    bool shown;
    POPUP_TYPE popupType;
    SDL_Surface *pSurface;

protected:
    int choiceHeight;
};


#ifdef __cplusplus
}
#endif /* __cplusplus */


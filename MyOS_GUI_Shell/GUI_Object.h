#pragma once
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Rect.h"

#ifdef __MYOS
#include "../MyOS_1/Libs/SDL/include/SDL.h"
#else
#include <SDL.h>
#endif

#define SYSTEM_MENU_HEIGHT 24
#define SYSTEM_MENU_COLOR_R 128
#define SYSTEM_MENU_COLOR_G 128
#define SYSTEM_MENU_COLOR_B 255

class GUI_Object
{
public:
    GUI_Object();
    ~GUI_Object();

    virtual void PaintToSurface(SDL_Surface *pTargetSurface) {}
    virtual bool MouseOver(int relX, int relY) { return true; }
    
//protected:
    GUI_Rect dimensions;

protected:
    void DrawSystemMenu(SDL_Surface *pSurface, char *windowName);
    void DrawVerticalLine(SDL_Surface *pSurface, int x, int startY, int endY, SDL_Color lineColor);
    void DrawHorizontalLine(SDL_Surface *pSurface, int startX, int endX, int y, SDL_Color lineColor);
    void DrawBox(SDL_Surface *pSurface, int x, int y, int width, int height, SDL_Color lineColor);
};

#ifdef __cplusplus
};
#endif /* __cplusplus */


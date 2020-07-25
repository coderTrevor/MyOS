#pragma once
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "SDL_picofont.h"
#include "GUI_Rect.h"

#ifdef __MYOS
#include "../MyOS_1/Libs/SDL/include/SDL.h"
#else
#include <SDL.h>
#endif

#define SYSTEM_MENU_HEIGHT 24
#define SYSTEM_MENU_COLOR_R 110
#define SYSTEM_MENU_COLOR_G 160
#define SYSTEM_MENU_COLOR_B 255

#define SDL_R_MASK          0xFF000000
#define SDL_G_MASK          0x00FF0000
#define SDL_B_MASK          0x0000FF00
#define SDL_A_MASK          0x000000FF

const SDL_Color SDL_BLACK       = { 0,   0,  0, 255 };
const SDL_Color SDL_DARKGRAY    = { 40, 40, 40, 255 };
const uint32_t  RGB_BLACK = 0x000000;
const SDL_Color SDL_WHITE = { 255, 255, 255, 255 };
const SDL_Color SDL_DEFAULT_BUTTON_COLOR = { 200, 200, 200, 255 };
const SDL_Color SDL_SYS_MENU_COLOR = { SYSTEM_MENU_COLOR_R, SYSTEM_MENU_COLOR_G, SYSTEM_MENU_COLOR_B, 255 };

class GUI_Object
{
public:
    GUI_Object();
    ~GUI_Object();

    virtual void GetRelativePoint(int &x, int &y);
    int GetRelX(int x) { return x - dimensions.left; }
    int GetRelY(int y) { return y - dimensions.top;  }
    virtual void PaintToSurface(SDL_Surface *pTargetSurface) {}
    virtual bool MouseOver(int relX, int relY) { return false; }
    virtual void OnClick(int relX, int relY) {}
    virtual void OnDrag(int startRelX, int startRelY, int relX, int relY) {}
    virtual void OnMouseUp(int relX, int relY) {}
    virtual bool PointInBounds(int x, int y);

//protected:
    GUI_Rect dimensions;

protected:
    void DrawVerticalLine(SDL_Surface *pSurface, int x, int startY, int endY, SDL_Color lineColor);
    void DrawHorizontalLine(SDL_Surface *pSurface, int startX, int endX, int y, SDL_Color lineColor);
    void Draw3D_Box(SDL_Surface *pSurface, int x, int y, int width, int height);
    void Draw3D_InsetBox(SDL_Surface * pSurface, int x, int y, int width, int height);
    void DrawBox(SDL_Surface *pSurface, int x, int y, int width, int height, SDL_Color lineColor);
    void FillSurface(SDL_Surface *pSurface, SDL_Color color);
};

#ifdef __cplusplus
};
#endif /* __cplusplus */


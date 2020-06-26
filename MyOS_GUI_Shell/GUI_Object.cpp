#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "GUI_Object.h"

GUI_Object::GUI_Object()
{
}


GUI_Object::~GUI_Object()
{
}

bool GUI_Object::PointInBounds(int x, int y)
{
    if (x < dimensions.left
       || y < dimensions.top
       || x > dimensions.left + dimensions.width
       || y > dimensions.top + dimensions.height)
        return false;

    return true;
}

void GUI_Object::DrawVerticalLine(SDL_Surface *pSurface, int x, int startY, int endY, SDL_Color lineColor)
{
    uint32_t color = SDL_MapRGB(pSurface->format, lineColor.r, lineColor.g, lineColor.b);

    for (int y = startY; y <= endY; ++y)
    {
        uint32_t *pPixels = (uint32_t *)(pSurface->pixels);
        pPixels[x + y*pSurface->w] = color;
    }
}

void GUI_Object::DrawHorizontalLine(SDL_Surface *pSurface, int startX, int endX, int y, SDL_Color lineColor)
{
    uint32_t color = SDL_MapRGB(pSurface->format, lineColor.r, lineColor.g, lineColor.b);

    uint32_t *pPixels = (uint32_t *)(pSurface->pixels);
    pPixels += startX + y * pSurface->w;
    
    SDL_memset4(pPixels, color, endX - startX);
}

void GUI_Object::Draw3D_Box(SDL_Surface * pSurface, int x, int y, int width, int height)
{
    // Draw a white border on the top and left edges
    DrawVerticalLine(pSurface, x, y, y + height - 1, SDL_WHITE);
    DrawHorizontalLine(pSurface, x, x + width - 1, y, SDL_WHITE);

    // Draw a black border around the bottom and right edges
    DrawVerticalLine(pSurface, x + width - 1, y, y + height - 1, SDL_BLACK);
    DrawHorizontalLine(pSurface, x + 1, x + width - 1, y + height - 1, SDL_BLACK);
}

void GUI_Object::Draw3D_InsetBox(SDL_Surface * pSurface, int x, int y, int width, int height)
{
    // Draw a white border on the top and left edges
    DrawVerticalLine(pSurface, x, y, y + height - 1, SDL_BLACK);
    DrawHorizontalLine(pSurface, x, x + width - 1, y, SDL_BLACK);

    // Draw a black border around the bottom and right edges
    DrawVerticalLine(pSurface, x + width - 1, y, y + height - 1, SDL_WHITE);
    DrawHorizontalLine(pSurface, x + 1, x + width - 1, y + height - 1, SDL_WHITE);
}

void GUI_Object::DrawBox(SDL_Surface *pSurface, int x, int y, int width, int height, SDL_Color lineColor)
{
    DrawHorizontalLine(pSurface, x, x + width, y, lineColor);
    DrawVerticalLine(pSurface, x + width, y, y + height, lineColor);
    DrawHorizontalLine(pSurface, x, x + width, y + height, lineColor);
    DrawVerticalLine(pSurface, x, y, y + height, lineColor);
}

void GUI_Object::DrawSystemMenu(SDL_Surface *pSurface, char *windowName)
{
    // Fill in a bar at the top
    SDL_Rect sysRect;
    sysRect.x = sysRect.y = 0;
    sysRect.w = dimensions.width;
    sysRect.h = SYSTEM_MENU_HEIGHT;

    SDL_Color sysColor;
    sysColor.r = SYSTEM_MENU_COLOR_R;
    sysColor.g = SYSTEM_MENU_COLOR_G;
    sysColor.b = SYSTEM_MENU_COLOR_B;
    uint32_t color = SDL_MapRGB(pSurface->format, sysColor.r, sysColor.g, sysColor.b);
    SDL_FillRect(pSurface, &sysRect, color);
    
    // Draw the title of the window
    SDL_Color black;
    black.r = black.g = black.b = 0;
    SDL_Surface *pFont = FNT_Render(windowName, black);
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
    uint32_t red = SDL_MapRGB(pSurface->format, 255, 120, 120 );
    SDL_FillRect(pSurface, &boxRect, red);
    Draw3D_Box(pSurface, boxLeft, boxTop, boxWidth, boxHeight);

    pFont = FNT_Render("X", black);
    boxRect.x += 7;
    boxRect.y += 7;
    SDL_BlitSurface(pFont, NULL, pSurface, &boxRect);
    SDL_FreeSurface(pFont);


    boxLeft = boxLeft - boxWidth - 2;
    boxRect = { boxLeft, boxTop, boxWidth, boxHeight };
    SDL_FillRect(pSurface, &boxRect, SDL_MapRGB(pSurface->format, 205, 205, 205));
    Draw3D_Box(pSurface, boxLeft, boxTop, boxWidth, boxHeight);

    pFont = FNT_Render("[", black);
    boxRect.x += 4;
    boxRect.y += 7;
    SDL_BlitSurface(pFont, NULL, pSurface, &boxRect);
    boxRect.x += 4;
    SDL_FreeSurface(pFont);
    pFont = FNT_Render("]", black);
    SDL_BlitSurface(pFont, NULL, pSurface, &boxRect);
    SDL_FreeSurface(pFont);

    boxLeft = boxLeft - boxWidth - 2;
    boxRect = { boxLeft, boxTop, boxWidth, boxHeight };
    SDL_FillRect(pSurface, &boxRect, SDL_MapRGB(pSurface->format, 205, 205, 205));
    Draw3D_Box(pSurface, boxLeft, boxTop, boxWidth, boxHeight);

    pFont = FNT_Render("_", black);
    boxRect.x += 7;
    boxRect.y += 7;
    SDL_BlitSurface(pFont, NULL, pSurface, &boxRect);
    SDL_FreeSurface(pFont);
}

#ifdef __cplusplus
};
#endif /* __cplusplus */
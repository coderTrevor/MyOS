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

void GUI_Object::GetRelativePoint(int & x, int & y)
{
    x -= dimensions.left;
    y -= dimensions.top;
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

// Fill a surface with a color
void GUI_Object::FillSurface(SDL_Surface * pSurface, SDL_Color color)
{
    uint32_t col = SDL_MapRGB(pSurface->format, color.r, color.g, color.b);
    SDL_FillRect(pSurface, NULL, col);
}

#ifdef __cplusplus
};
#endif /* __cplusplus */
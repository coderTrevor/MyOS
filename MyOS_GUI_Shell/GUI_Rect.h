#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __MYOS
#include "../MyOS_1/Libs/SDL/include/SDL.h"
#else
#include <SDL.h>
#endif

typedef struct GUI_Rect
{
    int top, left;
    int width, height;

    int GetBottom()
    {
        return top + height;
    }

    int GetRight()
    {
        return left + width;
    }

    SDL_Rect *GetSDL_Rect()
    {
        sdlRect.x = left;
        sdlRect.y = top;
        sdlRect.w = width;
        sdlRect.h = height;

        return &sdlRect;
    }

    SDL_Rect sdlRect;
} GUI_Rect;

#ifdef __cplusplus
};
#endif /* __cplusplus */

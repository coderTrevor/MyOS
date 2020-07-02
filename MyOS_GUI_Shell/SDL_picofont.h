/* sdl_picofont

   http://nurd.se/~noname/sdl_picofont

   File authors:
      Fredrik Hultin
      With Modifications for MyOS by Trevor Thompson

   License: GPLv2
*/

#ifndef SDL_PICOFONT_H
#define SDL_PICOFONT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __MYOS
#include "../MyOS_1/Libs/SDL/include/SDL.h"
#else
#include <SDL.h>
#endif


#define FNT_FONTHEIGHT  8
#define FNT_FONTWIDTH   8

// put same space between rows to improve readability
#define FNT_ROWSPACING  3

// put some margins around the borders
#define FNT_LEFTRIGHTMARGIN     3
#define FNT_TOPBOTTOMMARGIN     3

typedef struct
{
    int x;
    int y;
}FNT_xy;

SDL_Surface* FNT_Render(const char* text, SDL_Color color);
SDL_Surface* FNT_RenderMax(const char* text, unsigned int len, SDL_Color color);

unsigned char* FNT_GetFont();


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* SDL_PICOFONT_H */

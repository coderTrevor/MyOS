/* sdl_picofont

   http://nurd.se/~noname/sdl_picofont

   File authors:
      Fredrik Hultin
      Modified for MyOS by Trevor Thompson

   License: GPLv2
*/

#ifndef SDL_PICOFONT_H
#define SDL_PICOFONT_H

#include "Display_HAL.h"

#define FNT_FONTHEIGHT  8
#define FNT_FONTWIDTH   8
// put same space between rows to improve readability
#define FNT_ROWSPACING  3

// put some margins around the borders
#define FNT_LEFTRIGHTMARGIN 3
#define FNT_TOPBOTTOMMARGIN    3

typedef struct
{
    int x;
    int y;
}FNT_xy;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void FNT_Render(const char* text, FNT_xy position);
void FNT_RenderMax(const char* text, unsigned int len, FNT_xy position);

unsigned char* FNT_GetFont();


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* SDL_PICOFONT_H */

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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void FNT_Render(const char* text);
void FNT_RenderMax(const char* text, unsigned int len);

unsigned char* FNT_GetFont();


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* SDL_PICOFONT_H */

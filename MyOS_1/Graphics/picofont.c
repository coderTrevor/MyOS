/* sdl_picofont

   http://nurd.se/~noname/sdl_picofont

   File authors:
      Fredrik Hultin
      Modified for MyOS by Trevor Thompson

   License: GPLv2
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <string.h>
#include "picofont.h"
#include "Graphical_Terminal.h"

// TODO: Support multiple bit depths
FNT_xy FNT_Generate(const char* text, unsigned int len, unsigned int w, PIXEL_32BIT* pixels, FNT_xy position)
{
	unsigned int i, x, y, col, row, stop;
	unsigned char *fnt, chr;
	FNT_xy xy;

	fnt = FNT_GetFont();

	col = row = stop = 0;
    xy.x = position.x + FNT_LEFTRIGHTMARGIN;
    xy.y = position.y + FNT_TOPBOTTOMMARGIN;

    /*PIXEL_32BIT white, black;
    white.alpha = 0;
    white.blue = 255;
    white.green = 255;
    white.red = 255;

    black.alpha = 0;
    black.red = 0;
    black.green = 0;
    black.blue = 0;*/

	for(i = 0; i < len && text[i] != '\0'; i++)
    {
		switch(text[i])
        {
			/*case '\n':
				row++;
				col = 0;
				chr = 0;
				break;

			case '\r':
				chr = 0;
				break;

			case '\t':
				chr = 0;
				col += 4 - col % 4;
				break;
		
			case '\0':
				stop = 1;
				chr = 0;
				break;*/
	
			default:
				col++;
				chr = text[i];
				break;
		}

		if(stop){
			break;
		}

		/*if((col + 1) * FNT_FONTWIDTH > (unsigned int)xy.x){
			xy.x = col * FNT_FONTWIDTH;
		}
		
		if((row + 1) * FNT_FONTHEIGHT > (unsigned int)xy.y){
			xy.y = (row + 1) * FNT_FONTHEIGHT;
		}*/

        if (chr == 0 || w == 0)
            continue;

        // TODO: print border based on FNT_ROWSPACING with background color

		for(y = 0; y < FNT_FONTHEIGHT; y++)
        {
			for(x = 0; x < FNT_FONTWIDTH; x++)
            {
				if(fnt[text[i] * FNT_FONTHEIGHT + y] >> (7 - x) & 1)
					pixels[((col - 1) * FNT_FONTWIDTH) + x + xy.x + (xy.y + y + row * (FNT_FONTHEIGHT + FNT_ROWSPACING)) * w] = graphicalForeground;
                else
                {
                    //if(!backgroundImage)
                        pixels[((col - 1) * FNT_FONTWIDTH) + x + xy.x + (xy.y + y + row * (FNT_FONTHEIGHT + FNT_ROWSPACING)) * w] = graphicalBackground;
                }
			}
		}	
	}

    return xy;
}

void FNT_Render(const char* text, FNT_xy position)
{
	FNT_RenderMax(text, strlen(text), position);
}

void FNT_RenderMax(const char* text, unsigned int len, FNT_xy position)
{
	//SDL_Surface* surface;
	//PIXEL_32BIT colors[2];
	//FNT_xy xy;

	/*colors[0].red = (color.red + 0x7e) % 0xff;
	colors[0].green = (color.green + 0x7e) % 0xff;
	colors[0].blue = (color.blue + 0x7e) % 0xff;

	colors[1] = color;*/

	//xy = FNT_Generate(text, len, 0, NULL);

	/*surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE, 
			xy.x,
			xy.y,
			8,
			0,
			0,
			0,
			0
	);

	if(!surface){
		return NULL;
	}

	SDL_SetColorKey(surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, (color.r + 0x7e) % 0xff, (color.g + 0x7e) % 0xff, (color.b + 0x7e) % 0xff));
	//SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x0d, 0xea, 0xd0));

	SDL_SetColors(surface, colors, 0, 2);


	if(SDL_MUSTLOCK(surface)){
		SDL_LockSurface(surface);
	}*/

    PIXEL_32BIT temp = graphicalForeground;
    graphicalForeground = graphicalOutline;

    // draw black outline first
    ++position.x;
    ++position.y;
    FNT_Generate(text, len, graphicsWidth, (PIXEL_32BIT *)linearFrameBuffer, position);

    graphicalForeground = temp;
    --position.x;
    --position.y;
    FNT_Generate(text, len, graphicsWidth, (PIXEL_32BIT *)linearFrameBuffer, position);

    if (backgroundImage && foregroundText)
    {
        graphicalForeground = graphicalOutline;

        // draw black outline first
        ++position.x;
        ++position.y;
        FNT_Generate(text, len, graphicsWidth, foregroundText, position);

        graphicalForeground = temp;
        --position.x;
        --position.y;
        FNT_Generate(text, len, graphicsWidth, foregroundText, position);
    }

	/*if(SDL_MUSTLOCK(surface)){
		SDL_UnlockSurface(surface);
	}

	return surface;*/
}


#ifdef __cplusplus
};
#endif /* __cplusplus */

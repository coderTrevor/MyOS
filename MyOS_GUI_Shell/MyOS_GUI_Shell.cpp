// TestApp1.cpp : A super-simple application for testing with MyOS
//

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#include "stdafx.h"
#ifndef WIN32
#include "../MyOS_1/Console_VGA.h"
#include "../MyOS_1/Interrupts/System_Calls.h"
//#undef _MSVC_VER
#undef _WIN32
//#define HAVE_LIBC 0
#define SDL_ATOMIC_DISABLED 1
//#define SDL_EVENTS_DISABLED 0
//#define SDL_TIMERS_DISABLED 0
#define HAVE_MALLOC 1
#else
#include <stdio.h>
#endif

#define SDL_MAIN_HANDLED 1

#include "../MyOS_1/Libs/SDL/include/SDL.h"
#include "../MyOS_1/Libs/SDL/include/SDL_video.h"
#include "../MyOS_1/Graphics/Cursor.h"

#include "GUI_Window.h"

#define MAX_GUI_WINDOWS 256 /*TEMP HACK*/
GUI_Window *windowList[MAX_GUI_WINDOWS] = { NULL };
// This would be something like a std::map when we have that
uint32_t windowIDs[MAX_GUI_WINDOWS] = { 0 };

#define DEFAULT_WINDOW_HEIGHT   320
#define DEFAULT_WINDOW_WIDTH    480

int nextX = 11;
int nextY = 11;
#define MAX_WINDOW_X  500
#define MAX_WINDOW_Y  400
#define WINDOW_X_INC  21
#define WINDOW_Y_INC  21

// probably TEMP:
int bgRed = 0;
int bgGreen = 40;
int bgBlue = 128;
#define BG_RED_INC      22
#define BG_GREEN_INC    31
#define BG_BLUE_INC     42

#define CURSOR_X        16
#define CURSOR_Y        16

SDL_Rect ball = { 0, 0, 20, 20 };
SDL_Point velocity = { 2, 2 };

SDL_Rect cursorRect = { 0, 0, CURSOR_X, CURSOR_Y };

// Keep track of a stack of windows
// (Not a stack in the computer science sense, but in the sense that windows can overlap other windows)
struct GUI_WINDOW_STACK_ENTRY;
typedef struct GUI_WINDOW_STACK_ENTRY
{
    GUI_Window *pThisWin;
    GUI_WINDOW_STACK_ENTRY *pUnderneath;
    GUI_WINDOW_STACK_ENTRY *pAbove;
} GUI_WINDOW_STACK_ENTRY;

GUI_WINDOW_STACK_ENTRY windowStack[MAX_GUI_WINDOWS] = { 0, 0, 0 };
GUI_WINDOW_STACK_ENTRY *pStackTop = NULL;// &windowStack[0];

// Search the stack of windows, top to buttom to see whic window the mouse is hovering over
GUI_WINDOW_STACK_ENTRY *FindWindowFromPoint(int x, int y)
{
    GUI_WINDOW_STACK_ENTRY *pCurrent = pStackTop;

    while (pCurrent && pCurrent->pThisWin)
    {
        if (pCurrent->pThisWin->PointInBounds(x, y))
            return pCurrent;

        pCurrent = pCurrent->pUnderneath;
    }
    
    return NULL;
}

void BringWindowToFront(GUI_WINDOW_STACK_ENTRY *pEntry)
{
    if (pStackTop == pEntry)
        return;

    // bridge the hole left by this window
    if (pEntry->pAbove)
        pEntry->pAbove->pUnderneath = pEntry->pUnderneath;

    if (pEntry->pUnderneath)
        pEntry->pUnderneath->pAbove = pEntry->pAbove;

    // previous stack top will be below this entry
    pStackTop->pAbove = pEntry;
    pEntry->pUnderneath = pStackTop;
    pEntry->pAbove = NULL;

    pStackTop = pEntry;
}

GUI_WINDOW_STACK_ENTRY *GetBottomWindow()
{
    if (!pStackTop)
        return NULL;

    GUI_WINDOW_STACK_ENTRY *pCurrent = pStackTop;

    while (pCurrent->pUnderneath)
        pCurrent = pCurrent->pUnderneath;

    return pCurrent;
}

// This would probably be called with a process' PID
GUI_Window *CreateTextWindow(uint32_t uniqueID)
{
    // find the first unused entry in the windowList
    for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
    {
        if (!windowList[i])
        {
            windowList[i] = new GUI_Window(nextX, nextY, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_WIDTH);
            // TODO: Check for NULL
            windowIDs[i] = uniqueID;
            
            // Advance position of next window
            nextX += WINDOW_X_INC;
            nextX %= MAX_WINDOW_X;
            nextY += WINDOW_Y_INC;
            nextY %= MAX_WINDOW_Y;

            // set bg color and change color of next window
            SDL_Color bg;
            bg.r = bgRed;
            bg.b = bgBlue;
            bg.g = bgGreen;

            windowList[i]->SetBackgroundColor(bg);

            bgRed += BG_RED_INC;
            bgGreen += BG_GREEN_INC;
            bgBlue += BG_BLUE_INC;

            bgRed %= 255;
            bgGreen %= 255;
            bgBlue %= 255;

            // Put this window in a stack entry and make it the topmost window
            windowStack[i].pThisWin = windowList[i];
            windowStack[i].pAbove = NULL;

            if (pStackTop && pStackTop->pThisWin)
            {
                windowStack[i].pUnderneath = pStackTop;
                pStackTop->pAbove = &windowStack[i];
            }
            else
                windowStack[i].pUnderneath = NULL;

            pStackTop = &windowStack[i];

            return windowList[i];
        }
    }

    printf("ERROR: Couldn't find any free window slots!\n");
    return NULL;
}

// TODO: There are memory leaks that need debugging
int main(int argc, char* argv[])
{
#ifdef __MYOS

    // re-enable interrupts (TODO: FIXME: What is disabling them??)
    __asm sti;

    // disable shell display elements
    hideShellDisplay();
#endif

    SDL_Window *window;                    // Declare a pointer
                                           //The window we'll be rendering to

    SDL_Surface* screenSurface = NULL;     // the surface to render to

    //printf("Running gui\n");

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

                                           // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        800,                               // width, in pixels (ignored)
        600,                               // height, in pixels (ignored
        SDL_WINDOW_FULLSCREEN_DESKTOP      // Make the window the same size as the desktop
    );

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    // Get window surface
    screenSurface = SDL_GetWindowSurface(window);

    // Fill the surface green
    /*SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0xFF, 0x7F));

    // Update the surface
    SDL_UpdateWindowSurface(window);

    SDL_Delay(500);  // Pause execution for 500 milliseconds, for example
    */
                     // Load BMP
    char *filename = "kghrwide.bmp";
#if __MYOS__
    filename = "kg2.bmp";
#endif
    SDL_Surface *bitmapSurface = SDL_LoadBMP(filename);
    if (bitmapSurface == NULL)
    {
        printf("Unable to load image %s! SDL Error: %s\n", filename, SDL_GetError());
        //success = false;
        return -1;
    }

    uint32_t green = SDL_MapRGB(screenSurface->format, 0x00, 0xFF, 0x7F);

    // Create a GUI_Window
    //GUI_Window bigWindow(200, 200, 200, 200);

///    windowStack[0].pThisWin = &bigWindow;
    
    bool done = false;

    SDL_Event event;

    int lastWindowID = 0;

    // Hide the mouse cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Create a surface for the cursor
    // TODO: Should we use SDL_CreateSystemCursor()?
    SDL_Surface *pCursor = SDL_CreateRGBSurface(0, // flags (unused)
                                    CURSOR_X, // width,
                                    CURSOR_Y, // height,
                                    32, // bit-depth
                                    0xFF000000,  // Rmask (default)
                                    0x00FF0000,  // Gmask (default)
                                    0x0000FF00,  // Bmask (default),
                                    0x000000FF); // Amask*/
    if (!pCursor)
    {
        printf("ERROR: Couldn't create surface for cursor\n");
    }

    // Fill in cursor image (Not sure why the SDL_memcpy4 isn't working)
    //SDL_memcpy4(pCursor->pixels, cursorImage, 16 * 16);
    for (int y = 0; y < 16; ++y)
    {
        for (int x = 0; x < 16; ++x)
        {
            PIXEL_32BIT originalColor = cursorImage[y][x];
            uint32_t color = SDL_MapRGBA(pCursor->format, 
                                         originalColor.red,
                                         originalColor.green,
                                         originalColor.blue,
                                         originalColor.alpha);
            void *ptr = pCursor->pixels;
            SDL_memcpy4( (void*)((uint32_t)ptr + (((y * 16) + x) * 4)), &color, 1);
        }
    }

    SDL_SetSurfaceBlendMode(pCursor, SDL_BLENDMODE_BLEND);

    // Keep drawing everything
    while (!done)
    {
        // Update cursor position
        uint32_t mouseButtons = SDL_GetMouseState(&cursorRect.x, &cursorRect.y);

        GUI_WINDOW_STACK_ENTRY *pStackEntryUnderCursor = FindWindowFromPoint(cursorRect.x, cursorRect.y);
        
        if (SDL_PollEvent(&event))
        {
            //GUI_Window *pWindow;
            SDL_Color red = { 255, 0, 0 };
            SDL_Color green = { 0, 255, 0 };

            switch (event.type)
            {
                case SDL_USEREVENT:
                    //TODO
                    break;

                case SDL_KEYDOWN:
                    //switch (event.key.keysym)
                    CreateTextWindow(lastWindowID++);
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_n:
                            CreateTextWindow(lastWindowID++);
                            break;
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    };
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    //pWindow = CreateTextWindow(lastWindowID++);

                    switch (event.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            //pWindow->SetBackgroundColor(red);
                            if (pStackEntryUnderCursor)
                                BringWindowToFront(pStackEntryUnderCursor);// ->pThisWin->SetBackgroundColor(red);
                            break;

                        case SDL_BUTTON_RIGHT:
                            if (pStackEntryUnderCursor)
                                pStackEntryUnderCursor->pThisWin->SetBackgroundColor(green);
                            //pWindow->SetBackgroundColor(green);
                            break;
                    };
                    break;

                case SDL_QUIT:
                    done = true;
                    break;

                default:
                    break;
            }
        }

        // draw the "background"
        if (bitmapSurface)
        {
            // Blit bitmap to window
            SDL_BlitScaled(bitmapSurface, NULL, screenSurface, NULL);

            //SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example
        }

        // draw all of the windows
//        bigWindow.PaintToSurface(screenSurface);

        /*for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
        {
            if(windowList[i])
                windowList[i]->PaintToSurface(screenSurface);
        }*/

        GUI_WINDOW_STACK_ENTRY *nextWindowEntry = GetBottomWindow();
        while (nextWindowEntry)
        {
            if (nextWindowEntry->pThisWin)
                nextWindowEntry->pThisWin->PaintToSurface(screenSurface);

            nextWindowEntry = nextWindowEntry->pAbove;
        }

        // draw a bouncing ball
        SDL_FillRect(screenSurface, &ball, green);
        ball.x += velocity.x;
        ball.y += velocity.y;

        if (ball.x > 800 || ball.x < 0)
            velocity.x = -velocity.x;
        if (ball.y > 600 || ball.y < 0)
            velocity.y = -velocity.y;

        // Draw the cursor
        SDL_BlitSurface(pCursor, NULL, screenSurface, &cursorRect);

        // Update the surface
        SDL_UpdateWindowSurface(window);

        //SDL_Delay(1);
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Free BMP surface
    //printf("Pixels at %p\n", bitmapSurface->pixels);
    SDL_FreeSurface(pCursor);
    SDL_FreeSurface(bitmapSurface);
    SDL_FreeSurface(screenSurface);

    // Clean up
    SDL_Quit();

    //printf("Done with SDL Test Program.\n");

    return 0;
}

#ifdef __cplusplus
};
#endif /* __cplusplus */
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
#include "GUI_MessageBox.h"
#include "GUI_Taskbar.h"
#include "GUI_Kernel_Shell.h"
#include "GUI_TerminalWindow.h"

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
GUI_Taskbar *pTaskbar;
#define DELETION_QUEUE_SIZE 16
GUI_Window *pDeletionQueue[DELETION_QUEUE_SIZE]; // UGLY HACK
int nextDeletionQueueIndex = 0;
int lastWindowID = 1;

#define CURSOR_X        16
#define CURSOR_Y        16

SDL_Rect ball = { 0, 0, 20, 20 };
SDL_Point velocity = { 2, 2 };

SDL_Rect cursorRect = { 0, 0, CURSOR_X, CURSOR_Y };
SDL_Point oldMousePos;

GUI_Window *pDraggedWindow = NULL;

// Keep track of a stack of windows
// (Not a stack in the computer science sense, but in the sense that windows can overlap other windows)
GUI_WINDOW_STACK_ENTRY windowStack[MAX_GUI_WINDOWS] = { 0, 0, 0 };
GUI_WINDOW_STACK_ENTRY *pStackTop = NULL;// &windowStack[0];

// Search the stack of windows, top to buttom to see whic window the mouse is hovering over
GUI_WINDOW_STACK_ENTRY *FindWindowFromPoint(int x, int y)
{
    GUI_WINDOW_STACK_ENTRY *pCurrent = pStackTop;

    while (pCurrent && pCurrent->pWindow)
    {
        if (pCurrent->pWindow->PointInBounds(x, y))
            return pCurrent;

        pCurrent = pCurrent->pUnderneath;
    }
    
    return NULL;
}

void RemoveWindowFromStack(GUI_WINDOW_STACK_ENTRY *pEntry)
{
    // bridge the hole left by this window being removed
    if (pEntry->pAbove)
        pEntry->pAbove->pUnderneath = pEntry->pUnderneath;

    if (pEntry->pUnderneath)
        pEntry->pUnderneath->pAbove = pEntry->pAbove;

    // Check if this window was on top
    if (pStackTop == pEntry)
        pStackTop = pEntry->pUnderneath;
}

// TODO: Not sure I like this / not sure windowID helps anything vs using pointers directly
void BringWindowID_ToFront(uint32_t windowID)
{
    // find window pointer associated with window ID
    for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
    {
        if (windowIDs[i] == windowID)
        {
            GUI_Window *pWindow = windowList[i];

            // find stack entry associated with window ID
            for (int j = 0; j < MAX_GUI_WINDOWS; ++j)
            {
                if (windowStack[j].pWindow == pWindow)
                {
                    BringWindowToFront(&windowStack[j]);
                    return;
                }
            }

            printf("Couldn't find stack entry associated with window pointer!!\n");
            return;
        }
    }

    printf("Couldn't find window matching Window ID!\n");
}

void BringWindowToFront(GUI_WINDOW_STACK_ENTRY *pEntry)
{
    if (pStackTop == pEntry)
        return;

    // bridge the hole left by this window
    RemoveWindowFromStack(pEntry);

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

GUI_Window *GetWindowFromID(uint32_t uniqueID)
{
    for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
    {
        if (windowIDs[i] == uniqueID)
            return windowList[i];
    }

    return NULL;
}

void AddWindowToStack(GUI_Window *window, GUI_WINDOW_STACK_ENTRY *pStackEntry)
{
    // Put this window in a stack entry and make it the topmost window
    pStackEntry->pWindow = window;
    pStackEntry->pAbove = NULL;

    if (pStackTop && pStackTop->pWindow)
    {
        pStackEntry->pUnderneath = pStackTop;
        pStackTop->pAbove = pStackEntry;
    }
    else
        pStackEntry->pUnderneath = NULL;

    pStackTop = pStackEntry;
}

// This would probably be called with a process' PID
GUI_Window *CreateTextWindow(uint32_t uniqueID, const char *windowName)
{
    // find the first unused entry in the windowList
    for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
    {
        if (!windowList[i])
        {
            windowList[i] = new GUI_TerminalWindow(windowName);
                //new GUI_Window(nextX, nextY, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, windowName);
            
            // TODO: Check for NULL
            windowIDs[i] = uniqueID;
            
            // Advance position of next window
            /*nextX += WINDOW_X_INC;
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
            bgBlue %= 255;*/

            AddWindowToStack(windowList[i], &windowStack[i]);

            pTaskbar->AddWindow(uniqueID, windowList[i]);

            return windowList[i];
        }
    }

    printf("ERROR: Couldn't find any free window slots!\n");
    return NULL;
}

GUI_Rect NewWindowPosition(int width, int height)
{
    GUI_Rect windowPos = { nextY, nextX, width, height };

    // Advance position of next window
    nextX += WINDOW_X_INC;
    nextX %= MAX_WINDOW_X;
    nextY += WINDOW_Y_INC;
    nextY %= MAX_WINDOW_Y;

    return windowPos;
}

// This is kind of hacky and maybe I'll find a better way in the future.
// Windows can request their own removal, but if they're deleted at that point, execution will return to deleted code
void DeleteAllWindowsInQueue()
{
    for (int i = 0; i < nextDeletionQueueIndex; ++i)
        delete pDeletionQueue[i];

    nextDeletionQueueIndex = 0;
}

// Add a GUI_Window to the deletion queue
void DeleteWindow(GUI_Window *pWindow)
{
    if (nextDeletionQueueIndex > DELETION_QUEUE_SIZE)
    {
        printf("Can't delete any more windows, Queue is full!!\n");
        return;
    }

    pDeletionQueue[nextDeletionQueueIndex++] = pWindow;
}

// Display MessageBox
// TODO: don't require any memory allocation so we can show out-of-memory errors
void MessageBox(char *messageText, char *windowTitle)
{
    for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
    {
        if (!windowList[i])
        {
            windowList[i] = new GUI_MessageBox(messageText, windowTitle);
            AddWindowToStack(windowList[i], &windowStack[i]);            
            return;
        }
    }
    printf("No free spot for window\n");
    for (;;)
        __halt();
}

// Called by windows when they want to be destroyed
void Shell_Destroy_Window(GUI_Window *pWindow)
{
    if (!pWindow)
        return;

    // Find window index
    for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
    {
        if (windowList[i] == pWindow)
        {
            windowList[i] = NULL;

            RemoveWindowFromStack(&windowStack[i]);
            windowStack[i].pAbove = windowStack[i].pUnderneath = NULL;
            windowStack[i].pWindow = NULL;

            if (windowIDs[i])
            {
                pTaskbar->RemoveWindow(windowIDs[i]);
                windowIDs[i] = 0;
            }

            break;
        }
    }
    
    // Check if this window is being dragged / waiting for a MouseUp event
    if (pDraggedWindow == pWindow)
        pDraggedWindow = NULL;

    // Add window to deletion queue
    DeleteWindow(pWindow);
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
        0
        //SDL_WINDOW_FULLSCREEN_DESKTOP      // Make the window the same size as the desktop
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
    //char *filename = "kghrwide.bmp";
    char *filename = "kg2.bmp";
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

///    windowStack[0].pWindow = &bigWindow;
    
    bool done = false;
    bool dragging = false;

    SDL_Event event;

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

    // Create Taskbar
    pTaskbar = new GUI_Taskbar(screenSurface->w, screenSurface->h);

#ifdef __MYOS
    // Tell the kernel how to talk to us
    registerGuiCallback(GUI_Kernel_Callback);
#endif

    //GUI_Kernel_Callback(0, 0, NULL);

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
                    //CreateTextWindow(lastWindowID++, "New Window");

                    //MessageBox("Here's a test message", "Test Message");
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_n:
                            //CreateTextWindow(lastWindowID++);
                            break;
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    };
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        dragging = false;
                    if (pDraggedWindow)
                        pDraggedWindow->OnMouseUp(cursorRect.x - pDraggedWindow->dimensions.left, cursorRect.y - pDraggedWindow->dimensions.top);
                    break;

                case SDL_MOUSEMOTION:
                    if (dragging && pDraggedWindow)
                    {
                        pDraggedWindow->OnDrag(oldMousePos.x, oldMousePos.y, cursorRect.x, cursorRect.y);
                        oldMousePos.x = cursorRect.x;
                        oldMousePos.y = cursorRect.y;
                    }
                    else
                    {
                        if (pTaskbar->pStartMenu->PointInBounds(cursorRect.x, cursorRect.y))
                            pTaskbar->pStartMenu->MouseOver(cursorRect.x - pTaskbar->pStartMenu->dimensions.left,
                                                            cursorRect.y - pTaskbar->pStartMenu->dimensions.top);
                    }

                    break;

                case SDL_MOUSEBUTTONDOWN:
                    //pWindow = CreateTextWindow(lastWindowID++);

                    switch (event.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            //pWindow->SetBackgroundColor(red);
                            // Is the taskbar being clicked?
                            if (cursorRect.y >= pTaskbar->dimensions.top)
                            {
                                pTaskbar->OnClick(cursorRect.x, cursorRect.y - pTaskbar->dimensions.top);
                                pDraggedWindow = pTaskbar;
                            }
                            // What about the start menu?
                            else if (pTaskbar->pStartMenu->PointInBounds(cursorRect.x, cursorRect.y))
                            {
                                pTaskbar->pStartMenu->OnClick(cursorRect.x - pTaskbar->pStartMenu->dimensions.left,
                                                              cursorRect.y - pTaskbar->pStartMenu->dimensions.top);
                            }
                            else if (pStackEntryUnderCursor)
                            {
                                BringWindowToFront(pStackEntryUnderCursor);// ->pWindow->SetBackgroundColor(red);
                                GUI_Window *pWindow = pStackEntryUnderCursor->pWindow;
                                pDraggedWindow = pWindow;
                                pWindow->OnClick(cursorRect.x - pWindow->dimensions.left, cursorRect.y - pWindow->dimensions.top);

                                // Find windowID from window
                                for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
                                {
                                    if (windowList[i] == pWindow)
                                    {
                                        pTaskbar->WindowActivated(windowIDs[i]);
                                        break;
                                    }
                                }
                            }
                            else
                                pDraggedWindow = NULL;

                            oldMousePos = { cursorRect.x, cursorRect.y };
                            dragging = true;
                            break;

                        case SDL_BUTTON_RIGHT:
                            if (pStackEntryUnderCursor)
                                Shell_Destroy_Window(pStackEntryUnderCursor->pWindow);
                                //    pStackEntryUnderCursor->pWindow->SetBackgroundColor(green);
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

        // Fill the background with black instead of an image for a little while
        //SDL_FillRect(screenSurface, NULL, RGB_BLACK);

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
            if (nextWindowEntry->pWindow)
                nextWindowEntry->pWindow->PaintToSurface(screenSurface);

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

        // Draw the Taskbar
        pTaskbar->PaintToSurface(screenSurface);

        // Draw the cursor
        SDL_BlitSurface(pCursor, NULL, screenSurface, &cursorRect);

        // Update the surface
        SDL_UpdateWindowSurface(window);

        // Delete all windows in deletion queue
        DeleteAllWindowsInQueue();

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
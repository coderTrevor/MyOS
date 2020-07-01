#pragma once
#include "GUI_Window.h"


#define RUN_WINDOW_Y_FROM_BOTTOM    200
#define RUN_WINDOW_WIDTH            300
#define RUN_WINDOW_HEIGHT           75

const int editHeight = FNT_FONTHEIGHT + (FNT_TOPBOTTOMMARGIN * 2);
const GUI_Rect RunWindowEditBoxDimensions = { RUN_WINDOW_HEIGHT - 10 - editHeight, 10, RUN_WINDOW_WIDTH - 50, editHeight };

class GUI_RunWindow :
    public GUI_Window
{
public:
    GUI_RunWindow();
    ~GUI_RunWindow();
};


// TestApp1.cpp : A super-simple application for testing with MyOS
//

#include "stdafx.h"
#include "../MyOS_1/Console_VGA.h"
#include <intrin.h>
#include "../MyOS_1/Interrupts/System_Calls.h"

int main()
{
    //terminal_resume();
    //terminal_writestring("Hello World from an .exe!\n");
    SystemCallPrint("Hello World from an .exe!\n");
    return 0;
}


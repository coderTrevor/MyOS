// TestApp1.cpp : A super-simple application for testing with MyOS
//

#include "stdafx.h"
#include "../MyOS_1/Console_VGA.h"
#include <intrin.h>
#include "../MyOS_1/Interrupts/System_Calls.h"

int main()
{
    printf("Hello World from an .exe using a system call and printf()!\n");

    // TODO: This is broken:
    //SystemCallPrintf("Hello World from an .exe using a system call %s!\n%s %d\n", " and printf()", "test2", 42);
    return 0;
}


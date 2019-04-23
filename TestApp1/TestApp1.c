// TestApp1.cpp : A super-simple application for testing with MyOS
//

#include "stdafx.h"
#include "../MyOS_1/Console_VGA.h"
#include <intrin.h>
#include "../MyOS_1/Interrupts/System_Calls.h"

int main()
{
    printf("Hello World from an .exe using a system call%s\nDid you know that the answer to life, the universe, and everything is %d?\n", " and printf()!", 42);

    return 0;
}


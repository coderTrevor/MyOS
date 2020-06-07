// TestApp2.cpp : A super-simple application for testing multiprogramming with MyOS
//
#ifdef __MYOS
#include "../MyOS_1/Console_VGA.h"
#include "../MyOS_1/Interrupts/System_Calls.h"
#include "../MyOS_1/paging.h"
#include "../MyOS_1/misc.h"
#else
#include <stdio.h>
#include <conio.h>
#include <corecrt_io.h>
#endif
#include <stdint.h>
#include <stdbool.h>


uint32_t spBackup;

void func2()
{
    printf("func2\n");

    uint32_t spvar;
    __asm
    {
        mov spvar, esp
    }
    printf("Value of esp: 0x%lX\n", spvar);

    //timeDelayMS(1);
    //func2();
}

void func1()
{
    printf("func1\n");
    func2();
}

int main()
{
    printf("Here's a simple app the will continually print a character to the screen, for testing multi-programming\n");
    
    // TODO: Why won't this work with large values of esp???
    __asm
    {
        mov [spBackup], esp
    }
    
    printf("OK this seems to work... 0x%lX\n", spBackup);
    func1();

#ifdef __MYOS
    while (true)
    {
        printf("a");

        timeDelayMS(150);
    }
#endif

    return 0;
}
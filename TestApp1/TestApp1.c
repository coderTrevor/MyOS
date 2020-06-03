// TestApp1.cpp : A super-simple application for testing with MyOS
//

#include "stdafx.h"
#include "../MyOS_1/Console_VGA.h"
#include "../MyOS_1/Interrupts/System_Calls.h"
#include "../MyOS_1/paging.h"
#include "../MyOS_1/misc.h"
#include <stdint.h>

int main()
{
    /*printf("Hello World from an .exe using a system call%s\nDid you know that the answer to life, the universe, and everything is %d?\n", " and printf()!", 42);

    if (malloc(1))
        printf("malloc() success!\n");
    else
        printf("malloc() failed\n");
        
    //printf("Executing a 1-second delay...\n");
    
    uint32_t ms = timeGetUptimeMS();
    printf("%d ms\n", ms);
    timeDelayMS(1000);
    ms = timeGetUptimeMS();
    printf("%d ms\n", ms);

    // Try opening a file
    FILE *fp = fopen("dir.txt", "rb");

    char c;
    while (fread(&c, 1, 1, fp))
        printf("%c", c);

    // close the file
    printf("\nfp: %d\n", fp);
    int fc = fclose(fp);
    if (fc)
        printf("fclose() returned %d\n", fc);

    // test strdup()
    char *testString = "Testing strdup() - Hello World!\n";
    char *dupe = strdup(testString);

    printf(dupe);

    free(dupe);

    char *testString2 = malloc(128);
    memset(testString2, 0, 128);

    int length = snprintf(testString2, 128, "Hello, %s from snprintf()!\n", "World");

    printf(testString2);

    free(testString2);
    
    char str[] = "This is a sample string";
    char * pch;
    pch = strrchr(str, 's');
    printf("Last occurence of 's' found at %d \n", pch - str + 1);
    */

    // TODO: fix:
    /*
    printf("Press q to quit\n");

    uint16_t key;

    while (true)
    {
        bool keyReady = readFromKeyboard(&key);
        if (keyReady)
        {
            if(!(keyReady & 0x80))
                printf("%c\n", (char)key);
        }
    }*/

    //printf("Done\n");

    /*uint32_t spvar;
    __asm
    {
        mov spvar, esp
    }
    printf("0x%lX\n", spvar);*/

    printf("Hello World from an .exe using a system call%s\nDid you know that the answer to life, the universe, and everything is %d?\n", " and printf()!", 42);
   // exit();

    //printf("This code will never be executed because of the call to exit()\n");

    while (true)
    {
        printf("b");
        timeDelayMS(300);
    }

    return 0;
}
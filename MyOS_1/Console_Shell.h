#pragma once
#include <stdbool.h>
#include "multiboot.h"

#define MAX_COMMAND_LENGTH          160

extern bool shellEnterPressed;
extern char currentCommand[MAX_COMMAND_LENGTH];

void Shell_Backspace_Pressed();

void Shell_Delete_Pressed();

void Shell_Enter_Pressed();

void Shell_Entry(multiboot_info *mbInfo);

void Shell_Key_Received(unsigned char key);

void Shell_Process_command(void);

void Shell_Up_Pressed(void);



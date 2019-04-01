#pragma once
#include <stdbool.h>
#include "multiboot.h"

extern bool shellEnterPressed;


void Shell_Backspace_Pressed();

void Shell_Delete_Pressed();

void Shell_Enter_Pressed();

void Shell_Entry(multiboot_info *mbInfo);

void Shell_Key_Received(unsigned char key);

void Shell_Process_command(void);

void Shell_Up_Pressed(void);



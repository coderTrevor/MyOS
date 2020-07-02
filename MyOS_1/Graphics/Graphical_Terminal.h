#pragma once
#include "Display_HAL.h"

// TODO: Support higher resolutions
#define MAX_X_RES       800
#define MAX_Y_RES       600
//#define MAX_X_RES       1600
//#define MAX_Y_RES       1200

// TODO: Support multiple graphical terminals
extern unsigned int graphicalColumn;
extern unsigned int graphicalRow;
extern unsigned int graphicalMaxColumns;
extern unsigned int graphicalMaxRows;
extern PIXEL_32BIT *backgroundImage;
extern PIXEL_32BIT *foregroundText;

//extern FNT_xy currentPos;
extern PIXEL_32BIT graphicalForeground, graphicalBackground, graphicalOutline;

void GraphicalTerminalBackspace();

void GraphicalTerminalInit();

void GraphicalTerminalPrintIntTop(int value, uint16_t column);

void GraphicalTerminalPutChar(char c);

void GraphicalTerminalPutEntryAt(char c, size_t x, size_t y);

void GraphicalTerminalScrollUp(void);

//void GraphicalTerminalWriteString(char *string);

void GraphicalTerminalWritestringTop(const char *string, uint16_t column);

bool SetGraphicalTerminalBackground(char *bitmapFileName);
#include "Keyboard.h"
#include "../Console_Shell.h"
#include "../System_Specific.h"
#include "../Interrupts/PIC.h"
#include <stdbool.h>
#include "../Terminal.h"
#include "../printf.h"

unsigned char normal_keys_map[256];
bool keys_down[256] = { false };

bool left_shift_held;
bool right_shift_held;
bool awaitingSpecial = false;

// Circular-buffer for input that has been queued
// Just a very naive implementation for letting applications read keyboard input
// TODO: Make less-hacky and don't special-case the shell
uint16_t scanCodeBuffer[KEYS_BUFFER_SIZE] = { 0 };
int keyReadIndex = 0;
int keyWriteIndex = 0;

bool key_released(unsigned char scanCode)
{
    if(scanCode >= 128)
        return true;

    return false;
}
unsigned char scan_code;

void _declspec(naked) keyboard_interrupt_handler(void)
{
    _asm pushad;
    //++interrupts_fired;

    uint8_t status;
    status = inb(0x64);

    // get the scancode and send it on its way
    scan_code = inb(0x60);

    if (!(status & 0x20))
    {
        if (!awaitingSpecial)
        {
            if (scan_code == 0xE0)
                awaitingSpecial = true;
            else
            {
                // Handle keyboard input for the shell
                keyboard_key_received(scan_code);

                // Add scan code to queue for other running applications
                // TODO: Handle buffer full
                scanCodeBuffer[(keyWriteIndex++) % KEYS_BUFFER_SIZE] = scan_code;
            }
        }
        else
        {
            // Handle input for the shell
            keyboard_special_key_received(scan_code);

            // Add scan code to the keyboard queue; Have the upper byte start with 0xE0 to mark the key as special
            scanCodeBuffer[(keyWriteIndex++) % KEYS_BUFFER_SIZE] = 0xE000 + scan_code;

            awaitingSpecial = false;
        }
    }
    else
        kprintf("0x%X\n", scan_code);

    PIC_sendEOI(KEYBOARD_INTERRUPT);

    _asm
    {
        popad
        iretd
    }
}

void init_key_map()
{
    left_shift_held = right_shift_held = false;

    // map '?' to everything (we may end up missing some keys - these will show up as '?')
    for (int i = 0; i < 256; ++i)
        normal_keys_map[i] = '?';
    
    // Map all the keys we know about
    // There's almost certainly a more elegant way of doing this...
    normal_keys_map[A_PRESSED] = 'a';
    normal_keys_map[B_PRESSED] = 'b';
    normal_keys_map[C_PRESSED] = 'c';
    normal_keys_map[D_PRESSED] = 'd';
    normal_keys_map[E_PRESSED] = 'e';
    normal_keys_map[F_PRESSED] = 'f';
    normal_keys_map[G_PRESSED] = 'g';
    normal_keys_map[H_PRESSED] = 'h';
    normal_keys_map[I_PRESSED] = 'i';
    normal_keys_map[J_PRESSED] = 'j';
    normal_keys_map[K_PRESSED] = 'k';
    normal_keys_map[L_PRESSED] = 'l';
    normal_keys_map[M_PRESSED] = 'm';
    normal_keys_map[N_PRESSED] = 'n';
    normal_keys_map[O_PRESSED] = 'o';
    normal_keys_map[P_PRESSED] = 'p';
    normal_keys_map[Q_PRESSED] = 'q';
    normal_keys_map[R_PRESSED] = 'r';
    normal_keys_map[S_PRESSED] = 's';
    normal_keys_map[T_PRESSED] = 't';
    normal_keys_map[U_PRESSED] = 'u';
    normal_keys_map[V_PRESSED] = 'v';
    normal_keys_map[W_PRESSED] = 'w';
    normal_keys_map[X_PRESSED] = 'x';
    normal_keys_map[Y_PRESSED] = 'y';
    normal_keys_map[Z_PRESSED] = 'z';
    normal_keys_map[LEFT_BRACKET_PRESSED] = '[';
    normal_keys_map[RIGHT_BRACKET_PRESSED] = ']';
    normal_keys_map[ENTER_PRESSED] = '\r';
    normal_keys_map[COMMA_PRESSED] = ',';
    normal_keys_map[SPACE_PRESSED] = ' ';
    normal_keys_map[BACKSP_PRESSED] = '\b';
    normal_keys_map[TAB_PRESSED] = '\t';
    normal_keys_map[PERIOD_PRESSED] = '.';
    normal_keys_map[FORWARD_SLASH_PRESSED] = '/';
    normal_keys_map[BACK_SLASH_PRESSED] = '\\';
    normal_keys_map[BACK_TICK_PRESSED] = '`';
    normal_keys_map[ONE_PRESSED] = '1';
    normal_keys_map[TWO_PRESSED] = '2';
    normal_keys_map[THREE_PRESSED] = '3';
    normal_keys_map[FOUR_PRESSED] = '4';
    normal_keys_map[FIVE_PRESSED] = '5';
    normal_keys_map[SIX_PRESSED] = '6';
    normal_keys_map[SEVEN_PRESSED] = '7';
    normal_keys_map[EIGHT_PRESSED] = '8';
    normal_keys_map[NINE_PRESSED] = '9';
    normal_keys_map[ZERO_PRESSED] = '0';
    normal_keys_map[HYPHEN_PRESSED] = '-';
    normal_keys_map[EQUALS_PRESSED] = '=';
    normal_keys_map[SEMI_PRESSED] = ';';
    normal_keys_map[SINGLE_QUOTE_PRESSED] = '\'';
}

// Convert a keyboard scanCode to an ASCII code
unsigned char unmap_key(unsigned char scanCode)
{
    // map scanCode to ASCII keyCode
    unsigned char keyCode = normal_keys_map[scanCode];

    // check for shift being held
    if (left_shift_held || right_shift_held)
    {
        // convert lowercase letters to uppercase
        if (keyCode <= 'z' && keyCode >= 'a')
        {
            // uppercase keyCode       
            keyCode -= 0x20;
            return keyCode;
        }

        // change any other keys to their shifted versions
        switch (keyCode)
        {
            case '`':
                return '~';
            case '1':
                return '!';
            case '2':
                return '@';
            case '3':
                return '#';
            case '4':
                return '$';
            case '5':
                return '%';
            case '6':
                return '^';
            case '7':
                return '&';
            case '8':
                return '*';
            case '9':
                return '(';
            case '0':
                return ')';
            case '-':
                return '_';
            case '=':
                return '+';
            case '[':
                return '{';
            case ']':
                return '}';
            case '\\':
                return '|';
            case ';':
                return ':';
            case '\'':
                return '"';
            case ',':
                return '<';
            case '.':
                return '>';
            case '/':
                return '?';
        }
    }

    return keyCode;
}

void keyboard_key_received(unsigned char scanCode)
{
    // check for shift keys being held / released
    if (scanCode == LEFT_SHIFT_PRESSED)
    {
        left_shift_held = true;
        return;
    }

    if (scanCode == RIGHT_SHIFT_PRESSED)
    {
        right_shift_held = true;
        return;
    }

    if (scanCode == LEFT_SHIFT_RELEASED)
    {
        left_shift_held = false;
        return;
    }

    if (scanCode == RIGHT_SHIFT_RELEASED)
    {
        right_shift_held = false;
        return;
    }

    // ignore any other keys being released
    if (key_released(scanCode))
        return;

    // send the ASCII code to the terminal
    Shell_Key_Received(unmap_key(scanCode));
}

void keyboard_special_key_received(unsigned char scanCode)
{
    switch (scanCode)
    {
    case DEL_PRESSED:
        Shell_Backspace_Pressed();
        break;

    case UP_PRESSED:
        Shell_Up_Pressed();
        break;

    default:
        break;
    }
}

bool keyboard_read_from_queue(uint16_t *pScanCode)
{
    if (keyReadIndex == keyWriteIndex)
        return false;

    *pScanCode = scanCodeBuffer[(keyReadIndex++) % KEYS_BUFFER_SIZE];

    return true;
}
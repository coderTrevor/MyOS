#pragma once
#include "../Interrupts/Interrupts.h"

#define KEYBOARD_INTERRUPT (HARDWARE_INTERRUPTS_BASE + 1) /* remapped interrupt value for keyboard */

// there's probably a more elegant way to do this...
#define ESC_PRESSED     0x01
#define ONE_PRESSED     0x02
#define TWO_PRESSED     0x03
#define THREE_PRESSED   0x04
#define FOUR_PRESSED    0x05
#define FIVE_PRESSED    0x06
#define SIX_PRESSED     0x07
#define SEVEN_PRESSED   0x08
#define EIGHT_PRESSED   0x09
#define NINE_PRESSED    0x0A
#define ZERO_PRESSED    0x0B
#define HYPHEN_PRESSED  0x0C
#define EQUALS_PRESSED  0x0D
#define BACKSP_PRESSED  0x0E
#define TAB_PRESSED     0x0F
#define Q_PRESSED       0x10
#define W_PRESSED       0x11
#define E_PRESSED       0x12
#define R_PRESSED       0x13
#define T_PRESSED       0x14
#define Y_PRESSED       0x15
#define U_PRESSED       0x16
#define I_PRESSED       0x17
#define O_PRESSED       0x18
#define P_PRESSED       0x19
#define LEFT_BRACKET_PRESSED    0x1A
#define RIGHT_BRACKET_PRESSED   0x1B
#define ENTER_PRESSED   0x1C
#define LEFT_CTRL_PRESSED       0x1D
#define A_PRESSED       0x1E
#define S_PRESSED       0x1F
#define D_PRESSED       0x20
#define F_PRESSED       0x21
#define G_PRESSED       0x22
#define H_PRESSED       0x23
#define J_PRESSED       0x24
#define K_PRESSED       0x25
#define L_PRESSED       0x26
#define SEMI_PRESSED    0x27
#define SINGLE_QUOTE_PRESSED    0x28
#define BACK_TICK_PRESSED       0x29
#define LEFT_SHIFT_PRESSED      0x2A
#define BACK_SLASH_PRESSED      0x2B
#define Z_PRESSED       0x2C
#define X_PRESSED       0x2D
#define C_PRESSED       0x2E
#define V_PRESSED       0x2F
#define B_PRESSED       0x30
#define N_PRESSED       0x31
#define M_PRESSED       0x32
#define COMMA_PRESSED   0x33
#define PERIOD_PRESSED  0x34
#define FORWARD_SLASH_PRESSED   0x35
#define RIGHT_SHIFT_PRESSED     0x36
#define KP_STAR_PRESSED         0x37
#define LEFT_ALT_PRESSED        0x38
#define SPACE_PRESSED   0x39
#define CAPS_LOCK_PRESSED       0x3A
#define F1_PRESSED      0x3B
#define F2_PRESSED      0x3C
#define F3_PRESSED      0x3D
#define F4_PRESSED      0x3E
// etc...
#define A_RELEASED      0x9E
#define LEFT_SHIFT_RELEASED     0xAA
#define RIGHT_SHIFT_RELEASED    0xB6

// "Special" scancodes which are preceded by a 0xE0
#define UP_PRESSED      0x48
#define DEL_PRESSED     0x53
#define DEL_RELEASED    0xD3

unsigned char unmap_key(unsigned char scanCode);

void init_key_map();

void keyboard_key_received(unsigned char scanCode);

void keyboard_special_key_received(unsigned char scanCode);

void keyboard_interrupt_handler(void);

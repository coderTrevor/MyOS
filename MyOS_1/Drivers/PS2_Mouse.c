#include "PS2_Mouse.h"
#include <stdint.h>
#include "../System_Specific.h"
#include "../printf.h"
#include "../Interrupts/Interrupts.h"
#include "../Interrupts/IDT.h"
#include "../Interrupts/PIC.h"
#include "../System_Specific.h"
#include "../misc.h"
#include "Keyboard.h"

bool mousePresent = false;
bool fourBytePackets = false;

MOUSE_STATE mouseState = { 0, 0, false, false, false };

// For restoring the GUI data underneath the cursor
int oldMouseX = 0;
int oldMouseY = 0;
PIXEL_32BIT oldColor;

uint8_t packetByte1;
uint8_t packetByte2;
uint8_t packetByte3;
uint8_t packetByte4;

uint8_t PS2_Read_Data()
{
    // TODO: support timeout
    // wait for bit 0 of port 0x60 to become clear before sending any data
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL))
        ;

    return inb(PS2_DATA_PORT);
}

void PS2_Write_Command(uint8_t command)
{
    // TODO: support timeout
    // wait for bit 1 of port 0x64 to become clear before sending any data
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL)
        ;

    outb(PS2_COMMAND_PORT, command);
}

void PS2_Write_Data(uint8_t data)
{
    // TODO: support timeout
    // wait for bit 1 of port 0x64 to become clear before sending any data
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL)
        ;
    outb(PS2_DATA_PORT, data);
}

void PS2_Write_To_Device(bool secondPort, uint8_t data)
{
    if (secondPort)
        PS2_Write_Command(PS2_COMMAND_WRITE_TO_PORT_2);

    PS2_Write_Data(data);

    uint8_t ack = PS2_Read_Data();
    if (ack != PS2_DEVICE_ACK)
        kprintf("Unexpected ack 0x%X\n", ack);
}

void _declspec(naked) Mouse_InterruptHandler()
{
    _asm pushad;

    ++interrupts_fired;

    //if (debugLevel)
    //terminal_writestring(" --------- mouse interrupt fired! -------\n");
    
    uint8_t status;
    status = inb(0x64);

    if (status & 0x20)
    {
        packetByte1 = inb(PS2_DATA_PORT);
        packetByte2 = inb(PS2_DATA_PORT);
        packetByte3 = inb(PS2_DATA_PORT);
        
        if (fourBytePackets)
            packetByte4 = inb(PS2_DATA_PORT);

        mouseState.leftButton = (packetByte1 & MOUSE_LEFT_BUTTON);
        mouseState.middleButton = (packetByte1 & MOUSE_MIDDLE_BUTTON);
        mouseState.rightButton = (packetByte1 & MOUSE_RIGHT_BUTTON);

        if (!(packetByte1 & MOUSE_ALWAYS_1))
            terminal_writestring("Not 1\n");

        // Ignore movement if x or y overflow bits are set
        if (!(packetByte1 & MOUSE_X_OVERFLOW) && !(packetByte1 & MOUSE_Y_OVERFLOW))
        {
            int32_t xDelta = packetByte2;
            int32_t yDelta = packetByte3;

            // if the sign bit is set, the number is a two's-complement negative number
            if ((packetByte1 & MOUSE_X_SIGN) == MOUSE_X_SIGN)
                xDelta |= 0xFFFFFF00;
            
            mouseState.mouseX += xDelta;

            if ((packetByte1 & MOUSE_Y_SIGN) == MOUSE_Y_SIGN)
                yDelta |= 0xFFFFFF00;
            
            mouseState.mouseY -= yDelta;

            if (mouseState.mouseX < 0)
                mouseState.mouseX = 0;
            if (mouseState.mouseX >= MAX_X_RES)
                mouseState.mouseX = MAX_X_RES - 1;

            if (mouseState.mouseY < 0)
                mouseState.mouseY = 0;
            if (mouseState.mouseY >= MAX_Y_RES)
                mouseState.mouseY = MAX_Y_RES - 1;
        }
        else
            terminal_writestring("Overflow\n");
    }
    //else
      //  kprintf("uh %d\n", inb(PS2_DATA_PORT));

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + 12);

    //if (debugLevel)
        //terminal_writestring(" --------- mouse interrupt done! -------\n");

    _asm
    {
        popad
        iretd
    }
}

// secondPort should be false for first port, true for second
uint8_t PS2_Reset_Device(bool secondPort)
{
    PS2_Write_To_Device(secondPort, PS2_DEVICE_RESET);

    PS2_Write_To_Device(secondPort, PS2_DEVICE_RESET);

    // mouse should respond with 0xFA (read by Write_To_Device), 0xAA, [mouse id]
    uint8_t testResults = PS2_Read_Data();
    if(testResults != PS2_DEVICE_SELF_TEST_OK)
        kprintf(" 0x%X\n", testResults);

    uint8_t mouseID = PS2_Read_Data();
    
    return mouseID;
}

// secondPort should be false for first port, true for second
void PS2_Disable_Scanning(bool secondPort)
{
    PS2_Write_To_Device(secondPort, PS2_COMMAND_DISABLE_SCANNING);
}

uint8_t PS2_Detect_Device(bool secondPort)
{
    // Disable scanning
    PS2_Disable_Scanning(secondPort);

    // Send Identify command
    PS2_Write_To_Device(secondPort, PS2_DEVICE_IDENTIFY);

    uint8_t deviceID = PS2_Read_Data();
    kprintf("deviceID 0x%X\n", deviceID);

    // TODO: Check for more bytes
    return 0;
}

void Mouse_Init(void)
{
    __asm cli
    
    // Disable port 1
    PS2_Write_Command(PS2_COMMAND_DISABLE_PORT_1);

    // Flush ps2 output buffer
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL)
        inb(PS2_DATA_PORT);
    
    // Enable Port 2
    PS2_Write_Command(PS2_COMMAND_ENABLE_PORT_2);

    // Reset mouse
    PS2_Reset_Device(true);

    // Identify the mouse
    PS2_Detect_Device(true);

    // Enable IRQ12
    // Setup an interrupt handler for this device
    // TODO: Check for and support IRQ sharing
    Set_IDT_Entry((unsigned long)Mouse_InterruptHandler, HARDWARE_INTERRUPTS_BASE + 12);

    // Tell the PIC to enable the NIC's IRQ
    IRQ_Enable_Line(12);

    // Modify status to enable IRQ reception and disable port 2 clock
    uint8_t status;
    PS2_Write_Command(PS2_COMMAND_GET_STATUS_BYTE);
    status = PS2_Read_Data();
    //kprintf("s: 0x%X\n", status);
    status |= PS2_STATUS_ENABLE_IRQ_12;
    status &= ~(PS2_STATUS_ENABLE_MOUSE_CLOCK);

    PS2_Write_Command(PS2_COMMAND_SET_STATUS_BYTE);
    PS2_Write_Data(status);

    // Enable packet streaming
    PS2_Write_To_Device(true, PS2_DEVICE_ENABLE_STREAMING);

    PS2_Write_Command(PS2_COMMAND_ENABLE_PORT_1);

    if (!textMode)
    {
        mouseState.mouseX = MAX_X_RES / 2;
        mouseState.mouseY = MAX_Y_RES / 2;
        oldColor.red = 0;
        oldColor.green = 0;
        oldColor.blue = 0;
        oldColor.alpha = 0;
    }

    // TODO: Check presence
    mousePresent = true;
    
    // Re-enable interrupts
    __asm sti
}
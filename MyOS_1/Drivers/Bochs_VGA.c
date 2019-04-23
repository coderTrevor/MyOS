#include "Bochs_VGA.h"
#include "../Terminal.h"
#include "../System_Specific.h"
#include "../misc.h"
#include "../Graphics/Display_HAL.h"
#include "PCI_Bus.h"

// TODO: Support multiple displays
uint8_t BGA_bus;
uint8_t BGA_slot;
uint8_t BGA_function;
uint32_t *BGA_linearFrameBuffer;
bool BGA_present;
unsigned int BGA_version;

bool BGA_CheckPresence()
{
    uint16_t version = BGA_ReadRegister(VBE_DISPI_INDEX_ID);

    switch (version)
    {
        case BOCHS_VGA_VERSION_LATEST:
        case BOCHS_VGA_VERSION_5:
        case BOCHS_VGA_VERSION_4:
        case BOCHS_VGA_VERSION_3:
        case BOCHS_VGA_VERSION_2:
        case BOCHS_VGA_VERSION_1:
            BGA_present = true;
            break;
        default:
            BGA_present = false;
    }

    if (BGA_present)
    {
        // See if this is an older version of the BGA
        if (version != BOCHS_VGA_VERSION_LATEST)
        {
            if(debugLevel)
            {
                terminal_writestring("     Older version of BGA detected. Version in use: ");
                terminal_print_ushort_hex(version);
                terminal_newline();
                return false;
            }
        }

        return true;
    }

    terminal_writestring("     Unrecognized version magic: ");
    terminal_print_ushort_hex(version);
    terminal_newline();
    
    return false;
}

void BGA_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    BGA_bus = bus;
    BGA_slot = slot;
    BGA_function = function;

    terminal_writestring("    Initializing Bochs Graphics Adapter driver...\n");

    if (!BGA_CheckPresence())
    {
        terminal_writestring("    BGA not present, or unrecognized version\n");
        return;
    }
    
    graphicsPresent = true;

    terminal_writestring("    Bochs Graphics Adapter initialized\n");
}

uint16_t BGA_ReadRegister(uint16_t registerIndex)
{
    outw(VBE_DISPI_IOPORT_INDEX, registerIndex);
    return inw(VBE_DISPI_IOPORT_DATA);
}

bool BGA_SetResolution(uint16_t width, uint16_t height, uint16_t bitDepth)
{
    // disable VBE
    BGA_WriteRegister(VBE_DISPI_ENABLED, VBE_DISPI_DISABLED);

    // TODO: Check capabilities and validate params
    // Write desired res
    BGA_WriteRegister(VBE_DISPI_INDEX_XRES, width);
    BGA_WriteRegister(VBE_DISPI_INDEX_YRES, height);
    BGA_WriteRegister(VBE_DISPI_INDEX_BPP, bitDepth);

    // re-enable VBE, set linear frame buffer
    BGA_WriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);// | VBE_DISPI_NOCLEARMEM);

    textMode = false;

    // TODO: Check that the resolution changed

    // Get the linear frame buffer address, which may have changed
    BGA_linearFrameBuffer = (uint32_t*)(PCI_GetBaseAddress0(BGA_bus, BGA_slot, BGA_function) & 0xFFFFFFF0);
    linearFrameBuffer = BGA_linearFrameBuffer;
    //terminal_print_ulong_hex(linearFrameBuffer);
    //terminal_newline();

    // TODO IMPORTANT: Give the framebuffer address a map in the page table

    // set HAL globals
    graphicsBpp = bitDepth;
    graphicsWidth = width;
    graphicsHeight = height;

    return true;
}

void BGA_WriteRegister(uint16_t registerIndex, uint16_t data)
{
    outw(VBE_DISPI_IOPORT_INDEX, registerIndex);
    outw(VBE_DISPI_IOPORT_DATA, data);
}
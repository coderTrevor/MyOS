#include "ISA_DMA.h"
#include "../multiboot.h"
#include "../Terminal.h"
#include "../misc.h"
#include "../System_Specific.h"

// DMA Buffer must be located somewhere in the first 16MB of physical memory and must be made accessible via paging
uint8_t *DMA_Buffer;

void DMA_ClearFlipFlop(uint8_t channel)
{
    if (!DMA_VerifyChannel(channel))
        return;

    // TODO: Test 16-bit regs
    if (channel > 3)
    {
        outb(DMA_REG_FLIP_FLOP_16BIT, 0);
        return;
    }

    outb(DMA_REG_FLIP_FLOP_8BIT, 0);
}

void DMA_DisableChannel(uint8_t channel)
{
    if (!DMA_VerifyChannel(channel))
        return;

    // TODO: Test 16-bit registers
    if (channel > 3)
    {
        outb(DMA_SINGLE_CHANNEL_MASK_REG_16BIT, DMA_SINGLE_DRQ_MASK_CHANNEL | (channel - 4));
        return;
    }

    // 8-bit
    outb(DMA_SINGLE_CHANNEL_MASK_REG_8BIT, DMA_SINGLE_DRQ_MASK_CHANNEL | channel);
}

void DMA_EnableChannel(uint8_t channel)
{
    if (!DMA_VerifyChannel(channel))
        return;

    // TODO: Test 16-bit registers
    if (channel > 3)
    {
        outb(DMA_SINGLE_CHANNEL_MASK_REG_16BIT, (channel - 4));
        return;
    }

    // 8-bit
    outb(DMA_SINGLE_CHANNEL_MASK_REG_8BIT, channel);
}

void DMA_InitBuffer()//(multiboot_info *multibootInfo)
{    
    // TODO: Give DMA_Buffer a place in memory that isn't reserved and is in the first 16 MB of RAM 
    // and give that ram a place in the page table.
    // For now, we'll use the last 64K of 12MB, which we currently identity map and set-aside for applications.
    // No applications built for MyOS are currently big enough that this will be a problem.
    DMA_Buffer = (uint8_t *)(12 * 1024 * 1024 - DMA_BUFFER_SIZE);
    for (int i = 0; i < DMA_BUFFER_SIZE; ++i)
        DMA_Buffer[i] = 0;

    // Ensure the DMA_Buffer doesn't cross the boundary of a 64k "page"
    if (((uint32_t)DMA_Buffer >> 16) != (((uint32_t)DMA_Buffer + DMA_BUFFER_SIZE - 1) >> 16))
        terminal_writestring("ERROR: DMA Buffer not allocated properly!\n");
}

void DMA_SetBuffer(uint8_t channel, uint8_t *buffer)
{
    if (!DMA_VerifyChannel(channel))
        return;

    if (channel > 3)
    {
        // TODO: Add support for 16-bit channels
        terminal_writestring("TODO: this isn't implemented\n");
        return;
    }

    // make sure buffer doesn't exceed 16MB
    if ((uint32_t)buffer & 0xFF000000)
    {
        terminal_writestring("ERROR: Buffer not allocated properly!\n");
        return;
    }
    // TODO: Check 64k page-alignment like in DMA_InitBuffer(?)

    // split buffer address into its byte-components
    uint8_t page = (uint8_t)((uint32_t)buffer >> 16);
    uint16_t low16 = (uint16_t)((uint32_t)buffer & 0xFFff);
    uint8_t lowByte = (uint8_t)(low16 & 0xff);
    uint8_t highByte = (uint8_t)(low16 >> 8);

    // select appropriate page and address ports
    uint8_t pagePort;
    uint8_t addressPort;

    switch (channel)
    {
        case 1:
            pagePort = DMA_CHANNEL_1_PAGE_REG;
            addressPort = DMA_CHANNEL_1_ADDRESS;
            break;
        case 2:
            pagePort = DMA_CHANNEL_2_PAGE_REG;
            addressPort = DMA_CHANNEL_2_ADDRESS;
            break;
        case 3:
            pagePort = DMA_CHANNEL_3_PAGE_REG;
            addressPort = DMA_CHANNEL_3_ADDRESS;
            break;

        default:
            terminal_writestring("TODO: Unsupported channel!\n");
            return;
    }
    
    outb(pagePort, page);         // output page number
    outb(addressPort, lowByte);   // output low-byte
    outb(addressPort, highByte);  // output "high"-byte
}

void DMA_SetMode(uint8_t channel, uint8_t options)
{
    if (!DMA_VerifyChannel(channel))
        return;

    // TODO: Test 16-bit registers
    if (channel > 3)
    {
        outb(DMA_MODE_REGISTER_16BIT, options | (channel - 4));
        return;
    }
    
    // 8-bit
    outb(DMA_MODE_REGISTER_8BIT, options | channel);
}

void DMA_SetTransferLength(uint8_t channel, uint32_t transferLength)
{
    if (!DMA_VerifyChannel(channel))
        return;

    // TODO: check length

    // DMA considers the length to be one more than the byte count given. This allows for representing a full 64K
    // within 16 bits
    uint16_t length = (uint16_t)(transferLength - 1);

    uint8_t countPort;

    // set the count port
    switch (channel)
    {
        case 1:
            countPort = DMA_CHANNEL_1_COUNT;
            break;
        case 2:
            countPort = DMA_CHANNEL_2_COUNT;
            break;
        case 3:
            countPort = DMA_CHANNEL_3_COUNT;
            break;

        default:
            terminal_writestring("TODO: Unsupported channel!\n");
            return;
    }

    outb(countPort, (uint8_t)(length & 0xFF));  // Low byte of transfer length
    outb(countPort, (uint8_t)(length >> 8));    // High byte of transfer length
}

bool DMA_VerifyChannel(uint8_t channel)
{
    if (channel > 7)
    {
        terminal_writestring("Invalid DMA channel requested, ");
        terminal_print_byte(channel);
        terminal_newline();
        return false;
    }
    
    if (debugLevel)
    {
        terminal_writestring("DMA Channel ");
        terminal_print_byte(channel);
        terminal_writestring(" selected.\n");
    }
    return true;
}
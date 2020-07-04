#include "IDE.h"
#include "../printf.h"
#include "../System_Specific.h"
// TODO: Support multiple IDE controllers
uint8_t IDE_bus;
uint8_t IDE_slot;
uint8_t IDE_function;
bool IDE_Present;

IDE_CHANNEL_REGISTERS channels[2];

uint8_t ide_buf[2048] = { 0 };
bool ide_irq_invoked = 0;
uint8_t atapi_packet[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void IDE_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    IDE_bus = bus;
    IDE_slot = slot;
    IDE_function = function;

    kprintf("    Initializing IDE controller driver...");

    // Get the operating mode from the programming interface. We only support compatibility mode right now.
    // TODO: Support PCI native and/or try writing to progIF to see if device can be put into compatibility mode
    uint8_t progIF = PCI_GetProgrammingInterface(bus, slot, function);
    if ((progIF & PROG_IF_PRIMARY_CHANNEL_PCI_NATIVE) || (progIF & PROG_IF_SECONDARY_CHANNEL_PCI_NATIVE))
    {
        kprintf("\n we don't yet support PCI-native IDE controllers; aborting.");
        IDE_Present = false;
        return;
    }

    // get the I/O ports - this is how it would be done for PCI-native
    /*uint32_t primaryChannelBase = PCI_GetBaseAddress0(bus, slot, function) & ~3;
    uint32_t primaryChannelControl = PCI_GetBaseAddress1(bus, slot, function) & ~3;
    uint32_t secondaryChannelBase = PCI_GetBaseAddress2(bus, slot, function) & ~3;
    uint32_t secondaryChannelControl = PCI_GetBaseAddress3(bus, slot, function) & ~3;*/

    // Use the compatibility-mode I/O addresses
    channels[0].base = IDE_COMPAT_PRIMARY_COMMAND_IO;
    channels[0].ctrl = IDE_COMPAT_PRIMARY_CONTROL_IO;
    channels[1].base = IDE_COMPAT_SECONDARY_COMMAND_IO;
    channels[1].ctrl = IDE_COMPAT_SECONDARY_CONTROL_IO;
    
    kprintf("  0x%X 0x%X 0x%X 0x%X", channels[0].base, channels[0].ctrl, channels[1].base, channels[1].ctrl);

    // Compatibility mode means we use IRQ's 14 and 15
    channels[0].irq = 14;
    channels[1].irq = 15;
    kprintf(" - IRQs %d&%d\n", channels[0].irq, channels[1].irq);

    IDE_Present = true;

    kprintf("    IDE controller driver initialized\n");
}

uint8_t ide_polling(uint8_t channel, uint8_t advanced_check)
{

    // (I) Delay 400 nanosecond for BSY to be set:
    // -------------------------------------------------
    for (int i = 0; i < 4; i++)
        ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.

    // (II) Wait for BSY to be cleared:
    // TODO: add timeout
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
        ; // Wait for BSY to be zero.

    if (advanced_check)
    {
        uint8_t state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.

        // (III) Check For Errors:
        if (state & ATA_SR_ERR)
            return 2; // Error.

        // (IV) Check If Device fault:        
        if (state & ATA_SR_DF)
            return 1; // Device Fault.

        // (V) Check DRQ:
        // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
        if ((state & ATA_SR_DRQ) == 0)
            return 3; // DRQ should be set
    }

    return 0; // No Error.
}

uint8_t ide_read(uint8_t channel, uint8_t reg) 
{
    uint8_t result;

    // If we're writing to register ATA_REG_SECCOUNT1 or ATA_REG_LBAx (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].disableInterrupts);

    if (reg <= ATA_REG_COMMAND)
        result = inb(channels[channel].base + reg - 0x00);
    else if (reg <= ATA_REG_LBA5)
        result = inb(channels[channel].base + reg - 0x06);
    else if (reg <= ATA_REG_DEVADDRESS)
        result = inb(channels[channel].ctrl + reg - 0x0A);
    else if (reg < 0x16)
        result = inb(channels[channel].bmide + reg - 0x0E);

    // If we're writing to register ATA_REG_SECCOUNT1 or ATA_REG_LBAx (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].disableInterrupts);
    
    return result;
}

void ide_write(uint8_t channel, uint8_t reg, uint8_t data)
{
    // If we're writing to register ATA_REG_SECCOUNT1 or ATA_REG_LBAx (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].disableInterrupts);
    
    if (reg <= ATA_REG_COMMAND)
        outb(channels[channel].base + reg - 0x00, data);
    else if (reg <= ATA_REG_LBA5)
        outb(channels[channel].base + reg - 0x06, data);
    else if (reg <= ATA_REG_DEVADDRESS)
        outb(channels[channel].ctrl + reg - 0x0A, data);
    else if (reg < 0x16)
        outb(channels[channel].bmide + reg - 0x0E, data);
    
    // If we're writing to register ATA_REG_SECCOUNT1 or ATA_REG_LBAx (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].disableInterrupts);
}
#include "IDE.h"
#include "../printf.h"
#include "../System_Specific.h"
#include "../Timers/System_Clock.h"
#include "../misc.h"
#include "../Interrupts/IDT.h"
#include "../Interrupts/Interrupts.h"

// TODO: Support multiple IDE controllers
uint8_t IDE_bus;
uint8_t IDE_slot;
uint8_t IDE_function;
bool IDE_Present;

IDE_CHANNEL_INFO channels[2];

uint8_t ide_buf[2048] = { 0 };
bool ide_irq_invoked = 0;
//uint8_t atapi_packet[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void _declspec(naked) IDE_primary_channel_interrupt_handler()
{
    __asm
    {
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring(" --------- IDE 0 interrupt fired! -------\n");

    ide_irq_invoked = true;

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + channels[0].irq);

    if (debugLevel)
        terminal_writestring(" --------- IDE 0 interrupt done! -------\n");

    _asm
    {
        pop ebp
        iretd
    }
}
void _declspec(naked) IDE_secondary_channel_interrupt_handler()
{
    __asm
    {
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring(" --------- IDE 1 interrupt fired! -------\n");

    ide_irq_invoked = true;

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + channels[1].irq);

    if (debugLevel)
        terminal_writestring(" --------- IDE 1 interrupt done! -------\n");

    _asm
    {
        pop ebp
        iretd
    }
}

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

    // Register the IRQ handlers
    Set_IDT_Entry((unsigned long)IDE_primary_channel_interrupt_handler,
                  HARDWARE_INTERRUPTS_BASE + channels[0].irq);
    Set_IDT_Entry((unsigned long)IDE_secondary_channel_interrupt_handler,
                  HARDWARE_INTERRUPTS_BASE + channels[1].irq);

    // Tell the PIC to enable the IRQ
    IRQ_Enable_Line(channels[0].irq);
    IRQ_Enable_Line(channels[1].irq);

    IDE_Present = true;

    channels[0].bmide = PCI_GetBaseAddress4(bus, slot, function) & ~3;       // Bus Master IDE
    channels[1].bmide = (PCI_GetBaseAddress4(bus, slot, function) & ~3) + 8; // Bus Master IDE

    // 2- Disable IRQs: TODO: Need to disable interrupts per-device
    IDE_WriteRegister(ATA_PRIMARY_CHANNEL, ATA_REG_CONTROL, ATA_CONTROL_REG_DISABLE_INTERRUPTS);
    IDE_WriteRegister(ATA_SECONDARY_CHANNEL, ATA_REG_CONTROL, ATA_CONTROL_REG_DISABLE_INTERRUPTS);

    // 3- Detect ATA-ATAPI Devices:
    int driveNumber = 0;
    for (uint8_t channel = 0; channel < 2; channel++)
    {
        for (uint8_t device = 0; device < 2; device++)
        {
            uint8_t err = 0, type = IDE_ATA, status;
            ide_devices[driveNumber].Present = 0; // Assuming that no drive here.

            // (I) Select Drive:
            IDE_WriteRegister(channel, ATA_REG_HDDEVSEL, 0xA0 | (device << 4)); // Select Drive.
            TimeDelayMS(1); // Wait 1ms for drive select to work.
            //return;
            // (II) Send ATA Identify Command:
            IDE_WriteRegister(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            TimeDelayMS(1);

            // (III) Polling:
            if (IDE_ReadRegister(channel, ATA_REG_STATUS) == 0)
                continue; // If Status = 0, No Device.
            
            // TODO: Timeout
            while (1)
            {
                status = IDE_ReadRegister(channel, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR))
                {
                    err = 1; break;
                } // If Err, Device is not ATA.
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                    break; // Everything is right.
            }

            // (IV) Probe for ATAPI Devices:
            if (err != 0)
            {
                unsigned char cl = IDE_ReadRegister(channel, ATA_REG_LBA1);
                unsigned char ch = IDE_ReadRegister(channel, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB)
                    type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                    type = IDE_ATAPI;
                else
                    continue; // Unknown Type (may not be a device).

                IDE_WriteRegister(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                TimeDelayMS(1);
            }

            // (V) Read Identification Space of the Device:
            IDE_ReadBuffer(channel, ATA_REG_DATA, (uint32_t *)ide_buf, 128);

            // (VI) Read Device Parameters:
            ide_devices[driveNumber].Present = 1;
            ide_devices[driveNumber].Type = type;
            ide_devices[driveNumber].Channel = channel;
            ide_devices[driveNumber].Drive = device;
            ide_devices[driveNumber].Signature = *((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE));
            ide_devices[driveNumber].Capabilities = *((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES));
            ide_devices[driveNumber].CommandSets = *((unsigned int *)(ide_buf + ATA_IDENT_COMMANDSETS));

            // (VII) Get Size:
            if (ide_devices[driveNumber].CommandSets & (1 << 26))
                // Device uses 48-Bit Addressing:
                ide_devices[driveNumber].Size = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
            else
                // Device uses CHS or 28-bit Addressing:
                ide_devices[driveNumber].Size = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA));

            // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
            // model string is stored with every two bytes swapped
            for (int k = 0; k < IDE_MODEL_STRING_LENGTH; k += 2)
            {
                ide_devices[driveNumber].Model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                ide_devices[driveNumber].Model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
            }
            ide_devices[driveNumber].Model[IDE_MODEL_STRING_LENGTH] = 0; // Terminate String.

            driveNumber++;
        }
    }

    // 4- Print Summary:
    for (int i = 0; i < 4; i++)
    {
        if (ide_devices[i].Present == 1)
        {
            kprintf("     Found %s Drive - %s\n",
                    ide_devices[i].Type ? "ATAPI" : "ATA",  // type
                    //ide_devices[i].Size / 1024 / 1024 / 2,  // size
                    ide_devices[i].Model);
        }
    }

    kprintf("    IDE controller driver initialized\n");
}

uint8_t IDE_PollUntilNotBSY(uint8_t channel, uint8_t advanced_check)
{

    // (I) Delay 400 nanosecond for BSY to be set:
    // -------------------------------------------------
    for (int i = 0; i < 4; i++)
        IDE_ReadRegister(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.

    // (II) Wait for BSY to be cleared:
    // TODO: add timeout
    while (IDE_ReadRegister(channel, ATA_REG_STATUS) & ATA_SR_BSY)
        ; // Wait for BSY to be zero.

    if (advanced_check)
    {
        uint8_t state = IDE_ReadRegister(channel, ATA_REG_STATUS); // Read Status Register.

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

uint8_t IDE_ReadRegister(uint8_t channel, uint8_t reg) 
{
    uint8_t result = 0;

    // If we're reading from register ATA_REG_SECCOUNT1 or ATA_REG_LBA3 - 5 (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        IDE_WriteRegister(channel, ATA_REG_CONTROL, 0x80 | channels[channel].disableInterrupts);

    if (reg <= ATA_REG_COMMAND)
        result = inb(channels[channel].base + reg - 0x00);
    else if (reg <= ATA_REG_LBA5)
        result = inb(channels[channel].base + reg - 0x06);
    else if (reg <= ATA_REG_DEVADDRESS)
        result = inb(channels[channel].ctrl + reg - 0x0A);
    else if (reg < 0x16)
        result = inb(channels[channel].bmide + reg - 0x0E);

    // If we're reading from register ATA_REG_SECCOUNT1 or ATA_REG_LBA3 - 5 (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        IDE_WriteRegister(channel, ATA_REG_CONTROL, channels[channel].disableInterrupts);
    
    return result;
}

// TODO: Adapted from OSDev.org, need to rewrite in my own style
void IDE_ReadBuffer(uint8_t channel, uint8_t reg, uint32_t *buffer, unsigned int quads)
{
    if (reg > 0x07 && reg < 0x0C)
        IDE_WriteRegister(channel, ATA_REG_CONTROL, 0x80 | channels[channel].disableInterrupts);

    for (unsigned int i = 0; i < quads; ++i)
    {
        if (reg < 0x08)
            buffer[i] = inl(channels[channel].base + reg - 0x00);
        else if (reg < 0x0C)
            buffer[i] = inl(channels[channel].base + reg - 0x06);
        else if (reg < 0x0E)
            buffer[i] = inl(channels[channel].ctrl + reg - 0x0A);
        else if (reg < 0x16)
            buffer[i] = inl(channels[channel].bmide + reg - 0x0E);
    }

    if (reg > 0x07 && reg < 0x0C)
        IDE_WriteRegister(channel, ATA_REG_CONTROL, channels[channel].disableInterrupts);
}

void IDE_WaitForIRQ() 
{
    while (!ide_irq_invoked)
        ;
    ide_irq_invoked = false;
}

void IDE_WriteRegister(uint8_t channel, uint8_t reg, uint8_t data)
{
    // If we're writing to register ATA_REG_SECCOUNT1 or ATA_REG_LBA3 - 5 (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        IDE_WriteRegister(channel, ATA_REG_CONTROL, 0x80 | channels[channel].disableInterrupts);
    
    if (reg <= ATA_REG_COMMAND)
        outb(channels[channel].base + reg - 0x00, data);
    else if (reg <= ATA_REG_LBA5)
        outb(channels[channel].base + reg - 0x06, data);
    else if (reg <= ATA_REG_DEVADDRESS)
        outb(channels[channel].ctrl + reg - 0x0A, data);
    else if (reg < 0x16)
        outb(channels[channel].bmide + reg - 0x0E, data);
    
    // If we're writing to register ATA_REG_SECCOUNT1 or ATA_REG_LBA3 - 5 (the second device on the channel)
    if (reg >= ATA_REG_SECCOUNT1 && reg <= ATA_REG_LBA5)
        IDE_WriteRegister(channel, ATA_REG_CONTROL, channels[channel].disableInterrupts);
}
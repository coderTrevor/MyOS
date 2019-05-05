// Driver file for the intel 8254x line of network cards. Particularly the 82540em.

#include "e1000.h"
#include "../Terminal.h"
#include "PCI_Bus.h"
#include "../printf.h"
#include "../System_Specific.h"
#include "../paging.h"
#include "../Networking/Ethernet.h"
#include "../misc.h"

// TODO: Support multiple NIC's
//uint16_t e1000_base_port;
uint32_t e1000_mmAddress;
uint8_t  e1000_IRQ;
uint8_t  mac_addr[6];

/*bool e1000_Get_IO_Base(uint8_t bus, uint8_t slot, uint8_t function)
{
    bool ioAddrFound = false;

    uint32_t barValue = 0;
    //= PCI_GetBaseAddress0(bus, slot, function);

    // Check each PCI BAR until we find the one with the IO Register Base Address
    for (uint8_t barOffset = BAR0_OFFSET; barOffset <= BAR5_OFFSET; barOffset += 4)
    {
        barValue = PCI_ConfigReadDWord(bus, slot, function, barOffset);

        //kprintf("0x%lX\n", barValue);

        if (barValue & IOBIT)
        {
            ioAddrFound = true;
            break;
        }
    }

    if (!ioAddrFound)
        return false;

    // ignore the lowest three bits of bar to find IO Register Base Address
    e1000_base_port = barValue & ~7;
    
    return true;
}*/

bool e1000_Get_MMIO(uint8_t bus, uint8_t slot, uint8_t function)
{
    // Get value stored in BAR0
    uint32_t bar0 = PCI_GetBaseAddress0(bus, slot, function);

    // See if we're using MMIO (We should be)
    if ((bar0 & BAR_MMIO_OR_IO_BIT) == BAR_USING_IO)
    {
        terminal_writestring("\n      BAR0 isn't using MMIO. This driver doesn't handle that.\n");
        return false;
    }

    e1000_mmAddress = bar0 & BAR_ADDRESS;

    if ((bar0 & BAR_ADDRESS_TYPE) == BAR_64BIT)
    {
        // bar1 (upper address) should be 0 since we want to use 32-bit access
        uint32_t upperAddress = PCI_GetBaseAddress1(bus, slot, function);
        if (upperAddress != 0)
        {
            uint64_t address = (((uint64_t)upperAddress << 4) + e1000_mmAddress);
            kprintf("\n      64-bit MMIO isn't supported right now. Device is mapped to 0x%llX\n", address);
            return false;
        }
    }
    else
    {
        if ((bar0 & BAR_ADDRESS_TYPE) != BAR_32BIT)
        {
            terminal_writestring("\n      Invalid memory address size in BAR0\n");
            return false;
        }
    }

    return true;
}

void e1000_Get_Mac()
{
    // Read the MAC low bytes
    uint32_t macLow = e1000_Read_Register(REG_MAC_LOW);
    uint32_t macHigh = e1000_Read_Register(REG_MAC_HIGH);
    
    /* We have to swap some bytes around. From the intel manual:
    For a MAC address of 12 - 34 - 56 - 78 - 90 - AB, words 2:0 load as follows(note that these words are byteswapped) :
        Word 0 = 3412
        Word 1 = 7856
        Word 2 - AB90
        */
    uint16_t word0 = SwapBytes16((uint16_t)macLow);
    uint16_t word1 = SwapBytes16((uint16_t)(macLow >> 16));
    uint16_t word2 = SwapBytes16((uint16_t)(macHigh));

    if(debugLevel)
        kprintf("\nword0: %X\nword1: %X\nword2: %x\n", word0, word1, word2);

    memcpy(&mac_addr[0], &word0, 2);
    memcpy(&mac_addr[2], &word1, 2);
    memcpy(&mac_addr[4], &word2, 2);
}

void e1000_Net_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    (void)bus; (void)slot; (void)function;

    terminal_writestring("    Initializing e1000 driver...\n");
        
    /*if (!e1000_Get_IO_Base(bus, slot, function))
    {
        terminal_writestring("     Error: Unable to find IO Register Base Address\n");
        return;
    }*/

    if (!e1000_Get_MMIO(bus, slot, function))
        return;

    kprintf("     MMIO Address: 0x%lX", e1000_mmAddress);
    //kprintf("     Base Address: 0x%lX", e1000_base_port);

    // Map the MMIO in the page table
    uint32_t mmioPage = e1000_mmAddress / FOUR_MEGABYTES;
    pageDir[mmioPage] = ((mmioPage * FOUR_MEGABYTES)
                         | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);

    // get the IRQ
    e1000_IRQ = PCI_GetInterruptLine(bus, slot, function);
    kprintf(" - IRQ %d", e1000_IRQ);

    // make sure an IRQ line is being used
    if (e1000_IRQ == 0xFF)
    {
        terminal_writestring("\n      Can't use I/O APIC interrupts. Aborting.\n");
        return;
    }

    // Get the MAC address
    e1000_Get_Mac();
    terminal_writestring(" - MAC: ");
    EthernetPrintMAC(mac_addr);


    /*// Reset the virtio-network device
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_RESET_DEVICE);

    // Set the acknowledge status bit
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DEVICE_ACKNOWLEDGED);

    // Set the driver status bit
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED);

    // Read the feature bits
    uint32_t features = VNet_Read_Register(REG_DEVICE_FEATURES);

    // Make sure the features we need are supported
    if ((features & REQUIRED_FEATURES) != REQUIRED_FEATURES)
    {
        // uh-oh
        terminal_writestring("\n      Required features are not supported by device. Aborting.\n");
        VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DEVICE_ERROR);
        return;
    }

    // Tell the device what features we'll be using
    VNet_Write_Register(REG_GUEST_FEATURES, REQUIRED_FEATURES);

    // We must omit these next steps since we're supporting legacy devices
    // Tell the device the features have been negotiated
    /*VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED | STATUS_FEATURES_OK);

    // Make sure the device is ok with those features
    if ((VNet_Read_Register(REG_DEVICE_STATUS) & STATUS_FEATURES_OK) != STATUS_FEATURES_OK)
    {
    // uh-oh
    terminal_writestring("\n      Failed to negotiate features with device. Aborting.\n");
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DEVICE_ERROR);
    return;
    }*/
    /*
    // Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
    // reading and possibly writing the device’s virtio configuration space, and population of virtqueues.

    // enable PCI bus mastering
    PCI_EnableBusMastering(bus, slot, function);

    // Init virtqueues (see 4.1.5.1.3 of virtio-v1.0-cs04.pdf)
    // Since we don't negotiate VIRTIO_NET_F_MQ, we can expect 3 virtqueues: receive, transmit, and control
    VirtIO_Net_Init_Virtqueue(&receiveQueue, 0);
    VirtIO_Net_Init_Virtqueue(&transmitQueue, 1);
    //VirtIO_Net_Init_Virtqueue(&controlQueue,  2);

    // Setup the receive queue
    VirtIO_Net_SetupReceiveBuffers();

    // Setup an interrupt handler for this device
    // TODO: Check for and support IRQ sharing
    Set_IDT_Entry((unsigned long)VirtIO_Net_InterruptHandler, HARDWARE_INTERRUPTS_BASE + vNet_IRQ);

    // Tell the PIC to enable the NIC's IRQ
    IRQ_Enable_Line(vNet_IRQ);

    // Tell the device it's initialized
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DRIVER_READY);

    // Remind the device that it has receive buffers. Hey VirtualBox! Why aren't you using these?
    VNet_Write_Register(REG_QUEUE_NOTIFY, RECEIVE_QUEUE_INDEX);

    // Register this NIC with the ethernet subsystem
    EthernetRegisterNIC_SendFunction(VirtIO_Net_SendPacket);
    EthernetRegisterNIC_MAC(mac_addr);

    terminal_writestring("\n     Requesting IP address via DHCP...");

    //ARP_SendRequest(IPv4_PackIP(10, 0, 2, 2), mac_addr);
    DHCP_Send_Discovery(mac_addr);
    */
    terminal_writestring("\n    e1000 driver initialized.\n");
}

uint32_t e1000_Read_Register(uint32_t regOffset)
{
    volatile uint32_t data = *(uint32_t*)(e1000_mmAddress + regOffset);
    return data;
}
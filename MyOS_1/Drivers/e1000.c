// Driver file for the intel 8254x line of network cards. Particularly the 82540em.

#include "e1000.h"
#include "../Terminal.h"
#include "PCI_Bus.h"
#include "../printf.h"
#include "../System_Specific.h"
#include "../paging.h"
#include "../Networking/Ethernet.h"
#include "../misc.h"
#include "../Networking/DHCP.h"

// TODO: Support multiple NIC's
//uint16_t e1000_base_port;
uint32_t e1000_mmAddress;
uint8_t  e1000_IRQ;
uint8_t  mac_addr[6];

TX_DESC_LEGACY *txDescriptorList = 0;

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
    uint16_t word0 = (uint16_t)macLow;
    uint16_t word1 = (uint16_t)(macLow >> 16);
    uint16_t word2 = (uint16_t)macHigh;

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

    // Read the control register and set (or unset) the appropriate bits to reset the NIC
    uint32_t ctrlReg = e1000_Read_Register(REG_CTRL);
    
    // Set the link up
    ctrlReg |= CTRL_SET_LINK_UP | CTRL_AUTO_SPEED_DETECT;

    // Clear the PHY Reset bit
    ctrlReg &= ~(CTRL_PHY_RESET | CTRL_INVERT_LOSS_OF_SIGNAL);

    e1000_Write_Register(REG_CTRL, ctrlReg);

    e1000_TX_Init();

    // enable PCI bus mastering
    PCI_EnableBusMastering(bus, slot, function);

    // Setup an interrupt handler for this device
    // TODO: Check for and support IRQ sharing
    //Set_IDT_Entry((unsigned long)VirtIO_Net_InterruptHandler, HARDWARE_INTERRUPTS_BASE + vNet_IRQ);

    // Tell the PIC to enable the NIC's IRQ
    //IRQ_Enable_Line(vNet_IRQ);

    // Register this NIC with the ethernet subsystem
    EthernetRegisterNIC_SendFunction(e1000_SendPacket);
    EthernetRegisterNIC_MAC(mac_addr);

    terminal_writestring("\n     Requesting IP address via DHCP...");

    //ARP_SendRequest(IPv4_PackIP(10, 0, 2, 2), mac_addr);
    DHCP_Send_Discovery(mac_addr);
    
    terminal_writestring("\n    e1000 driver initialized.\n");
}

uint32_t e1000_Read_Register(uint32_t regOffset)
{
    volatile uint32_t data = *(uint32_t*)(e1000_mmAddress + regOffset);
    return data;
}

void e1000_SendPacket(Ethernet_Header *packet, uint16_t dataSize)
{
    if (debugLevel)
        terminal_writestring("e1000_SendPacket called\n");

    // TODO: (maybe) convert packet to a physical address and use it directly

    // Allocate a new buffer for the packet
    uint8_t *netBuffer = malloc(dataSize);

    // Copy packet to buffer
    memcpy(netBuffer, packet, dataSize);

    // Determine which transmit descriptor we'll be using from the value of the tail register
    uint32_t index = e1000_Read_Register(REG_TDT);

    if(debugLevel)
        kprintf("transmit descriptor index: %d\n", index);

    txDescriptorList[index].bufferAddress = (uint64_t)netBuffer;
    txDescriptorList[index].checksumOffset = 0;
    txDescriptorList[index].checksumStart = 0;
    txDescriptorList[index].command = TDESC_CMD_EOP;
    txDescriptorList[index].length = dataSize;
    txDescriptorList[index].special = 0;
    txDescriptorList[index].status = 0;

    // Increment tail index value
    index = (index + 1) % TX_DESCRIPTORS;

    e1000_Write_Register(REG_TDT, index);
}

void e1000_TX_Init()
{
    // Perform transmit initialization (From the Intel Manual:)

    // Allocate memory for the transmit descriptor list (16-byte aligned)
    void *txDescListMemory = malloc(sizeof(TX_DESC_LEGACY) * TX_DESCRIPTORS + 15);

    // Ensure the descriptor list is aligned on a 16-byte boundary
    if ((uint32_t)txDescListMemory % 16 != 0)
        txDescriptorList = (TX_DESC_LEGACY *)((uint32_t)txDescListMemory + 16 - ((uint32_t)txDescListMemory % 16));
    else
        txDescriptorList = (TX_DESC_LEGACY *)txDescListMemory;

    // Zero out the descriptor list
    memset(txDescriptorList, 0, sizeof(TX_DESC_LEGACY) * TX_DESCRIPTORS);

    // Program the Transmit Descriptor Base Address TDBAL register with the address of the region
    e1000_Write_Register(REG_TDBAL, (uint32_t)txDescriptorList);

    // Set the Transmit Descriptor Length (TDLEN) register to the size (in bytes) of the descriptor ring.
    // This value must be 128 - byte aligned (with 16 descriptors, it will be)
    uint32_t tdLength = TX_DESCRIPTORS * sizeof(TX_DESC_LEGACY);
    
    if (tdLength % 128 != 0)
        terminal_writestring("Error: TDLEN isn't properly calculated!\n");
    
    e1000_Write_Register(REG_TDLEN, tdLength);

    // The Transmit Descriptor Head and Tail (TDH/TDT) registers are initialized (by hardware) to 0b
    // after a power - on or a software initiated Ethernet controller reset. Software should write 0b to both
    // these registers to ensure this.
    e1000_Write_Register(REG_TDH, 0);
    e1000_Write_Register(REG_TDT, 0);

    /* Initialize the Transmit Control Register (TCTL) for desired operation to include the following:
        -Set the Enable (TCTL.EN) bit to 1b for normal operation.
        -Set the Pad Short Packets (TCTL.PSP) bit to 1b.
        -Configure the Collision Threshold (TCTL.CT) to the desired value. Ethernet standard is 10h.
This setting only has meaning in half duplex mode.
        -Configure the Collision Distance (TCTL.COLD) to its expected value. For full duplex
operation, this value should be set to 40h. For gigabit half duplex, this value should be set to
200h. For 10/100 half duplex, this value should be set to 40h. */

    // TODO: Pay attention to the duplex mode
    uint32_t tctl = TCTL_TX_ENABLE | TCTL_PAD_SHORT_PACKET | TCTL_DEFAULT_COLLISION_THRESHOLD;
    tctl |= TCTL_DEFAULT_COLLISION_DISTANCE | TCTL_RETRANSMIT_ON_LATE_COLLISION;
    e1000_Write_Register(REG_TCTL, tctl);

    /*  -Program the Transmit IPG (TIPG) register with the following decimal values to get the minimum
legal Inter Packet Gap:
            IPGT = 10
            IPGR1 = 10
            IPGR2 = 10
    */
    
    // (QEMU seems to ignore this register)
    e1000_Write_Register(REG_TIPG, TIPG_DEFAULTS);
}

void e1000_Write_Register(uint32_t regOffset, uint32_t value)
{
    volatile uint32_t *ptr = (uint32_t*)(e1000_mmAddress + regOffset);
    *ptr = value;
}
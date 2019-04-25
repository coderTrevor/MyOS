#include "Virtio_Net.h"
#include "../printf.h"
#include "PCI_Bus.h"
#include "../Terminal.h"
#include "../Networking/DHCP.h"
#include "../System_Specific.h"
#include "../Interrupts/PIC.h"
#include "../Interrupts/IDT.h"
#include "../Interrupts/Interrupts.h"
#include "../misc.h"
#include "../Networking/ARP.h"
#include "../Networking/Ethernet.h"
#include "../multiboot.h"

// TODO: Support multiple NIC's

/*uint8_t vNetBus;
uint8_t vNetSlot;
uint8_t vNetFunction;*/

uint16_t vNet_base_port;
uint8_t  vNet_IRQ;

// TEMPTEMP - there should be one MAC for each card that the system may have
uint8_t mac_addr[6];

inline uint32_t VNet_Read_Register(uint16_t reg)
{
    // if 4-byte register 
    if (reg <= REG_QUEUE_SIZE)
    {
        return inl(vNet_base_port + reg);
    }
    else
    {
        // if 2-byte register
        if (reg <= REG_QUEUE_NOTIFY)
            return inw(vNet_base_port + reg);
        else // 1-byte register
            return inb(vNet_base_port + reg);
    }
}

inline void VNet_Write_Register(uint16_t reg, uint32_t data)
{
    // if 4-byte register 
    if (reg <= REG_QUEUE_SIZE)
    {
        outl(vNet_base_port + reg, data);
    }
    else
    {
        // if 2-byte register
        if (reg <= REG_QUEUE_NOTIFY)
            outw(vNet_base_port + reg, (uint16_t)data);
        else // 1-byte register
            outb(vNet_base_port + reg, (uint8_t)data);
    }
}

// from virtio spec 2.4.2 Legacy Interfaces
/*#define ALIGN(x) (((x) + qalign) & ~qalign)
static inline uint32_t virtq_size(unsigned int qsz)
{
    return ALIGN(sizeof(struct virtq_desc)*qsz + sizeof(u16)*(3 + qsz))
        + ALIGN(sizeof(u16) * 3 + sizeof(struct virtq_used_elem)*qsz);
}*/

uint8_t *VirtIO_Net_Init_Virtqueue(uint16_t index, uint16_t queueSize)
{
    // determine size of virtqueue in bytes (see 2.4 Virtqueues in virtio spec)

    // virtqueues consist of:
    // Descriptor table
    // Available Ring
    // Used Ring
    //  - the above structures have alignment requirements we need to ensure we're fulfilling.

    // descriptorTablSize must be aligned on a 16-byte boundary. Since the virtqueue itself must be aligned on a 4096-byte boundary,
    // this alignment will be guaranteed.
    uint32_t descriptorTableSize = 16 * queueSize;

    // availableRingSize must always be aligned on a 2-byte boundary, which it always will because descriptorSize will be aligned to 
    // a 16-byte boundary and its size will be a multiple of 16.
    uint32_t availableRingSize = 2 * queueSize + 6;

    // usedRingSize must be aligned on a 4-byte boundary, which it probably won't be because of the 6 bytes added to availableRingSize
    uint32_t availableRingPadding = 0;
    if (availableRingSize % 4 != 0)
        availableRingPadding = 4 - (availableRingSize % 4);

    uint32_t usedRingSize = 8 * queueSize + 6;

    uint32_t virtqueueSize = descriptorTableSize + availableRingSize + availableRingPadding + usedRingSize;

    // Allocate memory for virtqueue + extra bytes for 4096-byte alignment
    uint8_t *virtqueue_mem;// TEMPTEMP - why can't the linker find malloc? = malloc(virtqueueSize + 4095);

    // Get a 4096-byte aligned block of memory
    uint8_t *virtqueue = virtqueue_mem;
    if ((uint32_t)virtqueue % 4096)
    {
        virtqueue = (uint8_t*)((uint32_t)virtqueue + 4096 - (uint32_t)virtqueue % 4096);
    }

    // Zero virtqueue memory
    memset(virtqueue, 0, virtqueueSize);

    return virtqueue;
}

void VirtIO_Net_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    // get the I/O port
    vNet_base_port = PCI_GetBaseAddress0(bus, slot, function) & ~3;

    terminal_writestring("    Initializing virtio-net driver...\n     Base address: ");
    terminal_print_ushort_hex(vNet_base_port);

    // get the IRQ
    vNet_IRQ = PCI_GetInterruptLine(bus, slot, function);
    kprintf(" - IRQ %d", vNet_IRQ);

    // make sure an IRQ line is being used
    if (vNet_IRQ == 0xFF)
    {
        terminal_writestring("\n      Can't use I/O APIC interrupts. Aborting.\n");
        return;
    }

    // Reset the virtio-network device
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

    // Tell the device the features have been negotiated
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED | STATUS_FEATURES_OK);

    // Make sure the device is ok with those features
    if ((VNet_Read_Register(REG_DEVICE_STATUS) & STATUS_FEATURES_OK) != STATUS_FEATURES_OK)
    {
        // uh-oh
        terminal_writestring("\n      Failed to negotiate features with device. Aborting.\n");
        VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DEVICE_ERROR);
        return;
    }
    // Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
    // reading and possibly writing the device’s virtio configuration space, and population of virtqueues.

    // store the MAC address
    uint16_t macReg = REG_MAC_1;
    for (int i = 0; i < 6; ++i, ++macReg)
        mac_addr[i] = (uint8_t)VNet_Read_Register(macReg);

    terminal_writestring(" - MAC: ");
    EthernetPrintMAC(mac_addr);

    // Init virtqueues (see 4.1.5.1.3 of virtio-v1.0-cs04.pdf)
    uint16_t currentQueue = 0;
    int queues = 0;
    int16_t queueSize = -1;
    while (queueSize != 0)
    {
        // access the current queue
        VNet_Write_Register(REG_QUEUE_SELECT, currentQueue);

        // get the size of the current queue
        queueSize = (uint16_t)VNet_Read_Register(REG_QUEUE_SIZE);

        if(queueSize && debugLevel)
            kprintf("\n      queue %d has size %d", currentQueue, queueSize);

        // Allocate and initialize the queue
        uint8_t *vq = VirtIO_Net_Init_Virtqueue(currentQueue, queueSize);
        kprintf("\nqueue: %d: %X", currentQueue, vq);

        ++currentQueue;
    }

    queues = currentQueue - 1;

    // Setup an interrupt handler for this device
    // TODO: Check for and support IRQ sharing
    Set_IDT_Entry((unsigned long)VirtIO_Net_InterruptHandler, HARDWARE_INTERRUPTS_BASE + vNet_IRQ);

    // Tell the PIC to enable the NIC's IRQ
    IRQ_Enable_Line(vNet_IRQ);

    if(debugLevel)
        kprintf("\n     Found %d virtqueues", queues);

    // Tell the device it's initialized
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DRIVER_READY);

    /*
    // Register this NIC with the ethernet subsystem
    EthernetRegisterNIC_SendFunction(RTL_8139_SendPacket);
    EthernetRegisterNIC_MAC(mac_addr);

    // TEMPTEMP - everything that follows is temporary / test code:
    terminal_writestring("     Requesting IP address via DHCP...\n");

    //ARP_Send_Request(IPv4_PackIP(10, 0, 2, 2), mac_addr);
    DHCP_Send_Discovery(mac_addr);
    */
    terminal_writestring("\n    virtio-net driver initialized.\n");
}

void _declspec(naked) VirtIO_Net_InterruptHandler()
{
    _asm pushad;

    /* do something */
    ++interrupts_fired;

    //if (debugLevel)
        terminal_writestring(" --------- virtio-net interrupt fired! -------\n");

    // get the interrupt status
    /*isr = inw(rtl8139_base_port + RTL_ISR);

    if (isr & ~(RTL_ISR_RX_OK | RTL_ISR_TX_OK))
    {
        terminal_writestring("rtl8139 Error: Unhandled ISR (TODO) ");
        terminal_print_ulong_hex(isr);
        for (;;)
            __halt();
    }
    //terminal_print_ulong_hex(isr);

    outw(rtl8139_base_port + RTL_ISR, 0xFFFF);

    // check the status and see if it's a transmit success
    if (isr & RTL_ISR_TX_OK)
    {
        if (debugLevel)
            terminal_writestring("     Packet sent successfully!\n");

        // clear the interrupt
        //outw(rtl8139_base_port + RTL_ISR, RTL_TX_OK);
    }

    // check the status and see if it's a received success
    if (isr & RTL_ISR_RX_OK)
    {
        RTL_8139_ReceivePacket();
    }
    */
    //outw(rtl8139_base_port + RTL_ISR, RTL_RX_OK);
    /*if (isr & RTL_TX_OK)

    // clear all interrupts
    outw(rtl8139_base_port + RTL_ISR, 0xFFFF);*/
    //isr = inw(rtl8139_base_port + RTL_ISR);
    //terminal_print_ulong_hex(isr);

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + vNet_IRQ);

    //if (debugLevel)
        terminal_writestring(" --------- virtio-net interrupt done! -------\n");

    _asm
    {
        popad
        iretd
    }
}
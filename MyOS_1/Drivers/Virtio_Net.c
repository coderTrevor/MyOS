#include "Virtio_Net.h"
#include "../System_Specific.h"
#include "../printf.h"
#include "../Interrupts/PIC.h"
#include "../Interrupts/IDT.h"
#include "../Interrupts/Interrupts.h"
#include "PCI_Bus.h"
#include "../Networking/Ethernet.h"
#include "../Networking/DHCP.h"
#include "../misc.h"

// TODO: Support multiple NIC's

/*uint8_t vNetBus;
uint8_t vNetSlot;
uint8_t vNetFunction;*/

uint16_t vNet_base_port;
uint8_t  vNet_IRQ;

// TEMPTEMP - there should be one MAC for each card that the system may have
uint8_t mac_addr[6];

virtq receiveQueue;
virtq transmitQueue;
virtq controlQueue; // (we don't use this one)

#define RECEIVE_QUEUE_INDEX     0
#define TRANSMIT_QUEUE_INDEX    1
#define CONTROL_QUEUE_INDEX     2

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


// TODO: Error-checking
void VirtIO_Net_Allocate_Virtqueue(virtq *virtqueue, uint16_t queueSize)
{
    // Zero virtqueue
    memset(virtqueue, 0, sizeof(virtq));

    // determine size of virtqueue in bytes (see 2.4 Virtqueues in virtio spec)

    // virtqueues consist of:
    // Descriptor table
    // Available Ring
    // Used Ring
    //  - the above structures have alignment requirements we need to ensure we're fulfilling.

    // descriptorTablSize must be aligned on a 16-byte boundary. Since the virtqueue itself must be aligned on a 4096-byte boundary,
    // this alignment will be guaranteed.
    uint32_t descriptorTableSize = 16 * queueSize;

    // availableRingSize must always be aligned on a 2-byte boundary, which it always will be because descriptorSize will be aligned to 
    // a 16-byte boundary and its size will be a multiple of 16.
    uint32_t availableRingSize = 2 * queueSize + 6;

    // usedRingSize must be aligned on a 4096-byte boundary (because this is a legacy driver), which it probably won't be
    uint32_t availableRingPadding = 0;
    if ((availableRingSize + descriptorTableSize) % 4096 != 0)
        availableRingPadding = 4096 - ((availableRingSize + descriptorTableSize) % 4096);

    uint32_t usedRingSize = 8 * queueSize + 6;

    uint32_t virtqueueByteSize = descriptorTableSize + availableRingSize + availableRingPadding + usedRingSize;

    if(debugLevel)
        kprintf("\n       virtqueueByteSize: %d", virtqueueByteSize);

    // Allocate memory for virtqueue + extra bytes for 4096-byte alignment
    uint8_t *virtqueue_mem = malloc(virtqueueByteSize + 4095);
    
    // Zero virtqueue memory
    memset(virtqueue_mem, 0, virtqueueByteSize + 4095);

    // Get a 4096-byte aligned block of memory
    //virtq *virtqueue = virtqueue_mem;
    if ((uint32_t)virtqueue_mem % 4096)
    {
        virtqueue_mem = (uint8_t*)((uint32_t)virtqueue_mem + 4096 - (uint32_t)virtqueue_mem % 4096);
    }

    // setup elements of virtqueue
    virtqueue->elements = queueSize;
    // descriptors will point to the first byte of virtqueue_mem
    virtqueue->descriptors = (virtq_descriptor *)virtqueue_mem;
    // driverArea (AKA Available Ring) will follow descriptors
    virtqueue->driverArea = (virtq_driver_area *)((uint32_t)virtqueue_mem + descriptorTableSize);
    // deviceArea will follow driver area + padding bytes
    virtqueue->deviceArea = (virtq_device_area *)((uint32_t)virtqueue->driverArea + availableRingSize + availableRingPadding);

    //return virtqueue;
}


// TODO: Error-checking, etc
void VirtIO_Net_Init_Virtqueue(virtq *virtqueue, uint16_t queueIndex)
{
    int16_t queueSize = -1;

    // access the current queue
    VNet_Write_Register(REG_QUEUE_SELECT, queueIndex);

    // get the size of the current queue
    queueSize = (uint16_t)VNet_Read_Register(REG_QUEUE_SIZE);

    if (!queueSize)
        return;

    if (debugLevel)
        kprintf("\n      queue %d has %d elements", queueIndex, queueSize);

    // Allocate and initialize the queue
    VirtIO_Net_Allocate_Virtqueue(virtqueue, queueSize);

    if (debugLevel)
    {
        kprintf("\n       queue %d: 0x%X", queueIndex, virtqueue->descriptors);
        kprintf("         queue size: %d", virtqueue->elements);
        kprintf("\n       driverArea: 0x%X", virtqueue->driverArea);
        kprintf("\n       deviceArea: 0x%X", virtqueue->deviceArea);
    }

    // TODO: Convert virtual address to physical address
    // (This isn't required now because all addresses are identity mapped)

    // Write address divided by 4096 to address register
    VNet_Write_Register(REG_QUEUE_ADDRESS, (uint32_t)virtqueue->descriptors / 4096);
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

    // Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
    // reading and possibly writing the device’s virtio configuration space, and population of virtqueues.

    // store the MAC address
    uint16_t macReg = REG_MAC_1;
    for (int i = 0; i < 6; ++i, ++macReg)
        mac_addr[i] = (uint8_t)VNet_Read_Register(macReg);

    terminal_writestring(" - MAC: ");
    EthernetPrintMAC(mac_addr);

    // Init virtqueues (see 4.1.5.1.3 of virtio-v1.0-cs04.pdf)
    // Since we don't negotiate VIRTIO_NET_F_MQ, we can expect 3 virtqueues: receive, transmit, and control
    VirtIO_Net_Init_Virtqueue(&receiveQueue,  0);
    VirtIO_Net_Init_Virtqueue(&transmitQueue, 1);
    VirtIO_Net_Init_Virtqueue(&controlQueue,  2);    

    // Setup the receive queue
    VirtIO_Net_SetupReceiveBuffer();

    // Setup an interrupt handler for this device
    // TODO: Check for and support IRQ sharing
    Set_IDT_Entry((unsigned long)VirtIO_Net_InterruptHandler, HARDWARE_INTERRUPTS_BASE + vNet_IRQ);

    // Tell the PIC to enable the NIC's IRQ
    IRQ_Enable_Line(vNet_IRQ);

    // Tell the device it's initialized
    VNet_Write_Register(REG_DEVICE_STATUS, STATUS_DRIVER_READY);
        
    // Register this NIC with the ethernet subsystem
    EthernetRegisterNIC_SendFunction(VirtIO_Net_SendPacket);
    EthernetRegisterNIC_MAC(mac_addr);

    terminal_writestring("\n     Requesting IP address via DHCP...");

    //ARP_Send_Request(IPv4_PackIP(10, 0, 2, 2), mac_addr);
    DHCP_Send_Discovery(mac_addr);

    terminal_writestring("\n    virtio-net driver initialized.\n");
}

void _declspec(naked) VirtIO_Net_InterruptHandler()
{
    _asm pushad;

    /* do something */
    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring(" --------- virtio-net interrupt fired! -------\n");

    // Get the interrupt status (This will also reset the isr status register)
    uint32_t isr;
    isr = VNet_Read_Register(REG_ISR_STATUS);

    // TODO: Support configuration changes (doubt this will ever happen)
    if (isr & VIRTIO_ISR_CONFIG_CHANGED)
        terminal_writestring("TODO: VirtIO-Net configuration has changed\n");

    // Check for used queues
    if (isr & VIRTIO_ISR_VIRTQ_USED)
    {
        //terminal_writestring("Virtq used\n");

        // see if the transmit queue has been used
        while (transmitQueue.deviceArea->index >= transmitQueue.lastUsedIndex + 1)
        {
            if(debugLevel)
                terminal_writestring("Transmit success\n");
            transmitQueue.lastUsedIndex++;
        }

        // see if the receive queue has been used
        if (receiveQueue.deviceArea->index >= receiveQueue.lastUsedIndex + 1)
        {
            VirtIO_Net_ReceivePacket();
            //receiveQueue.lastUsedIndex = receiveQueue.deviceArea->index;
        }
    }

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + vNet_IRQ);

    if (debugLevel)
        terminal_writestring(" --------- virtio-net interrupt done! -------\n");

    _asm
    {
        popad
        iretd
    }
}

void VirtIO_Net_SetupReceiveBuffer()
{
    const uint16_t bufferSize = 1526; // as per virtio specs

    // Allocate and add 16 buffers to receive queue
    for (uint16_t i = 0; i < 16; ++i)
    {
        uint8_t *buffer = malloc(bufferSize);

        // Add buffer to the descriptor table
        receiveQueue.descriptors[i].address = (uint64_t)buffer;
        receiveQueue.descriptors[i].flags = VIRTQ_DESC_F_DEVICE_WRITE_ONLY;/* | VIRTQ_DESC_F_NEXT;*/
        receiveQueue.descriptors[i].length = bufferSize;
        receiveQueue.descriptors[i].next = 0; // i + 1;

        // If this is the last descriptor we'll be using
        if (i == 15)
        {
            receiveQueue.descriptors[i].flags = VIRTQ_DESC_F_DEVICE_WRITE_ONLY;
            receiveQueue.descriptors[i].next = 0;
        }

        // Add index of descriptor to the driver ring
        receiveQueue.driverArea->ringArray[i] = i;
        receiveQueue.driverArea->index++;
    }

    VNet_Write_Register(REG_QUEUE_NOTIFY, RECEIVE_QUEUE_INDEX);
}

void VirtIO_Net_SendPacket(Ethernet_Header *packet, uint16_t dataSize)
{
    if(debugLevel)
        terminal_writestring("VirtIO_Net_SendPacket called\n");

    uint16_t bufferSize = dataSize + sizeof(virtio_net_hdr);

    // Allocate a buffer for the packet & header
    virtio_net_hdr *netBuffer = malloc(bufferSize);

    // Set parameters of netBuffer
    memset(netBuffer, 0, sizeof(virtio_net_hdr));

    // Copy packet to buffer
    memcpy((void*)((uint32_t)netBuffer + sizeof(virtio_net_hdr)), packet, dataSize);

    uint16_t index = transmitQueue.driverArea->index % transmitQueue.elements;

    if(debugLevel)
        kprintf("transmit queue index: %d\n", index);

    transmitQueue.descriptors[index].address = (uint64_t)netBuffer;
    transmitQueue.descriptors[index].flags = 0;
    transmitQueue.descriptors[index].length = bufferSize;
    transmitQueue.descriptors[index].next = 0;

    transmitQueue.driverArea->ringArray[index] = index;

    transmitQueue.driverArea->index++;

    transmitQueue.driverArea->flags = VIRTQ_AVAIL_F_NO_INTERRUPT;

    /*uint16_t bufferSize = sizeof(virtio_net_hdr);

    // Allocate a buffer for the header
    virtio_net_hdr *netBuffer = malloc(bufferSize);

    // Set parameters of netBuffer
    memset(netBuffer, 0, sizeof(virtio_net_hdr));

    uint16_t index = transmitQueue.driverArea->index % transmitQueue.elements;
    uint16_t index2 = (index + 1) % transmitQueue.elements;

    //if(debugLevel)
    kprintf("Index %d\n", index);
    kprintf("Index2 %d\n", index2);

    // net header
    transmitQueue.descriptors[index].address = (uint64_t)netBuffer;
    transmitQueue.descriptors[index].flags = VIRTQ_DESC_F_NEXT;
    transmitQueue.descriptors[index].length = bufferSize;
    transmitQueue.descriptors[index].next = index2;

    transmitQueue.driverArea->ringArray[index] = index;

    // ethernet packet
    transmitQueue.descriptors[index2].address = (uint64_t)packet;
    transmitQueue.descriptors[index2].flags = 0;
    transmitQueue.descriptors[index2].length = dataSize;
    transmitQueue.descriptors[index2].next = 0;

    //transmitQueue.driverArea->ringArray[index2] = index2;

    
    transmitQueue.driverArea->index++;// += 2;*/

    VNet_Write_Register(REG_QUEUE_NOTIFY, TRANSMIT_QUEUE_INDEX);
}

void VirtIO_Net_ReceivePacket()
{
    if (debugLevel)
        terminal_writestring("     Packet received successfully!\n");

    while (receiveQueue.lastUsedIndex <= receiveQueue.deviceArea->index - 1)
    {
        // get index to current descriptor
        uint16_t index = receiveQueue.deviceArea->ringArray[receiveQueue.lastUsedIndex % receiveQueue.elements].index;

        // get pointer to the beginning of the packet
        uint32_t *rxBegin = (uint32_t*)(receiveQueue.descriptors[index].address);

        // get packet status (placed at beginning of the packet by NIC)
        uint32_t rx_status = *rxBegin;

        // get the packet length
        //uint16_t rx_size = rx_status >> 16;

        // TODO: Check size, status, etc
        Ethernet_Header *packet = (Ethernet_Header *)((uint32_t)rxBegin + sizeof(virtio_net_hdr));

        EthernetProcessReceivedPacket(packet, mac_addr);

        // place the used descriptor index back in the available ring
        receiveQueue.driverArea->ringArray[receiveQueue.driverArea->index++] = index;

        receiveQueue.lastUsedIndex++;
    }

    // notify the device that we've updated the availaible ring index
    VNet_Write_Register(REG_QUEUE_NOTIFY, RECEIVE_QUEUE_INDEX);
}
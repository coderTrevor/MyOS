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
#include "../Networking/ARP.h"
#include "../Networking/IPv4.h"

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
    // Zero virtqueue structure
    memset(virtqueue, 0, sizeof(virtq));

    // determine size of virtqueue in bytes (see 2.4 Virtqueues in virtio spec)

    // virtqueues consist of:
    // Descriptor table
    // Available Ring
    // Used Ring
    //  - the above structures have alignment requirements we need to ensure we're fulfilling.

    // descriptor table must be aligned on a 16-byte boundary. Since the virtqueue itself must be aligned on a 4096-byte boundary,
    // this alignment will be guaranteed.
    uint32_t descriptorTableSize = 16 * queueSize;

    // driver area (AKA available ring) must be aligned on a 2-byte boundary, which it always will be because descriptorSize will be aligned to 
    // a 4096-byte boundary and its size will be a multiple of 16.
    uint32_t driverAreaSize = 2 * queueSize + 6;

    // device area (AKA used ring) must be aligned on a 4096-byte boundary (because this is a legacy driver), which it probably won't be
    uint32_t driverAreaPadding = 0;
    if ((driverAreaSize + descriptorTableSize) % 4096 != 0)
        driverAreaPadding = 4096 - ((driverAreaSize + descriptorTableSize) % 4096);

    uint32_t deviceAreaSize = 8 * queueSize + 6;

    uint32_t virtqueueByteSize = descriptorTableSize + driverAreaSize + driverAreaPadding + deviceAreaSize;

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
    virtqueue->deviceArea = (virtq_device_area *)((uint32_t)virtqueue->driverArea + driverAreaSize + driverAreaPadding);

    virtqueue->byteSize = virtqueueByteSize;
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

    //if (debugLevel)
    {
        kprintf("\n       queue %d: 0x%X", queueIndex, virtqueue->descriptors);
        kprintf("         queue size: %d", virtqueue->elements);
        kprintf("\n       driverArea: 0x%X", virtqueue->driverArea);
        kprintf("\n       deviceArea: 0x%X", virtqueue->deviceArea);
    }

    // TODO: Convert virtual address to physical address
    // (This isn't required now because all addresses malloc returns are identity mapped)

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

    // enable PCI bus mastering
    PCI_EnableBusMastering(bus, slot, function);

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

    terminal_writestring("\n    virtio-net driver initialized.\n");
}

void _declspec(naked) VirtIO_Net_InterruptHandler()
{
    _asm pushad;

    ++interrupts_fired;

    //if (debugLevel)
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
        while (transmitQueue.deviceArea->index != transmitQueue.lastDeviceAreaIndex)
        {
            //if(debugLevel)
                terminal_writestring("Transmit success\n");
            transmitQueue.lastDeviceAreaIndex++;
        }

        // see if the receive queue has been used
        if (receiveQueue.deviceArea->index != receiveQueue.lastDeviceAreaIndex)
        {
            VirtIO_Net_ReceivePacket();
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

void VirtIO_Net_SetupReceiveBuffers()
{
    const uint16_t bufferSize = 1526; // as per virtio specs

    // Allocate and add 16 buffers to receive queue
    for (uint16_t i = 0; i < 16; ++i)
    {
        uint8_t *buffer = malloc(bufferSize);

        // Add buffer to the descriptor table
        receiveQueue.descriptors[i].address = (uint64_t)buffer;
        receiveQueue.descriptors[i].flags = VIRTQ_DESC_F_DEVICE_WRITE_ONLY;
        receiveQueue.descriptors[i].length = bufferSize;
        receiveQueue.descriptors[i].next = 0;

        // Add index of descriptor to the driver ring
        receiveQueue.driverArea->ringBuffer[i] = i;
    }

    // Update next available index
    receiveQueue.driverArea->index = 16;

    VNet_Write_Register(REG_QUEUE_NOTIFY, RECEIVE_QUEUE_INDEX);
}

void VirtIO_Net_SendPacket(Ethernet_Header *packet, uint16_t dataSize)
{
    if(debugLevel)
        terminal_writestring("VirtIO_Net_SendPacket called\n");

    // This commented-out code sends the entire packet as one descriptor.
    // This works in Qemu but the packet never gets sent in VirtualBox
    /*uint16_t bufferSize = dataSize + sizeof(virtio_net_hdr);

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

    transmitQueue.driverArea->ringBuffer[index] = index;

    transmitQueue.driverArea->index++;

    transmitQueue.driverArea->flags = VIRTQ_AVAIL_F_NO_INTERRUPT;*/

    // Send packet using two descriptor entries. One for net header, another for the packet
    // This two-descriptor scheme seems to be what Virtualbox expects for some reason.
   uint16_t bufferSize = sizeof(virtio_net_hdr);

    // Allocate a buffer for the header
    virtio_net_hdr *netBuffer = malloc(bufferSize);

    // Set parameters of netBuffer
    memset(netBuffer, 0, sizeof(virtio_net_hdr));

    // Get indices for the next two descriptors
    uint16_t descIndex = transmitQueue.nextDescriptor % transmitQueue.elements;
    uint16_t descIndex2 = (descIndex + 1) % transmitQueue.elements;
    transmitQueue.nextDescriptor += 2;

    // Get index for the next entry into the available-ring
    uint16_t index = transmitQueue.driverArea->index % transmitQueue.elements;

    if (debugLevel)
    {
        kprintf("\ndescIndex %d", descIndex);
        kprintf("\ndescIndex2 %d", descIndex2);
        kprintf("\nIndex %d", index);
    }

    // fill descriptor with net header
    transmitQueue.descriptors[descIndex].address = (uint64_t)netBuffer;
    transmitQueue.descriptors[descIndex].flags = VIRTQ_DESC_F_NEXT;
    transmitQueue.descriptors[descIndex].length = bufferSize;
    transmitQueue.descriptors[descIndex].next = descIndex2;

    // copy packet to new buffer, because packetBuffer won't be a physical address
    uint8_t *packetBuffer = malloc(dataSize);
    memcpy(packetBuffer, packet, dataSize);
    // (TODO: malloc returns identity-mapped addresses for now but later we'll need a function to convert virtual to physical)

    // fill descriptor with ethernet packet
    transmitQueue.descriptors[descIndex2].address = (uint64_t)packetBuffer;
    transmitQueue.descriptors[descIndex2].flags = 0;
    transmitQueue.descriptors[descIndex2].length = dataSize;
    transmitQueue.descriptors[descIndex2].next = 0;

    // Add descriptor chain to the available ring
    transmitQueue.driverArea->ringBuffer[index] = descIndex;

    // Increase available ring index and notify the device
    transmitQueue.driverArea->index++;

    VNet_Write_Register(REG_QUEUE_NOTIFY, TRANSMIT_QUEUE_INDEX);
}

// This function is solely for debugging and trying to understand why VirtualBox won't show me received packets
// scan receive queue for non-zero data
void VirtIO_Net_ScanRQ()
{
    // check for device failure
    if (VNet_Read_Register(REG_DEVICE_STATUS) & STATUS_DEVICE_ERROR)
        terminal_writestring("Virtio-net device has encountered an error!\n");

    kprintf("receive used index: %d\n", receiveQueue.deviceArea->index);

    uint32_t addr = (uint32_t)&receiveQueue.descriptors[0];
    uint32_t lastAddr = addr + receiveQueue.byteSize;
    //addr = receiveQueue.driverArea;

    // dump non-zero parts of virtqueue
    terminal_dump_nonzero_memory(addr, lastAddr);

    // dump non-zero parts of all 16 receive buffers
    for (int i = 0; i < 16; ++i)
    {
        addr = (uint32_t)receiveQueue.descriptors[i].address;
        lastAddr = addr + receiveQueue.descriptors[i].length;
        terminal_dump_nonzero_memory(addr, lastAddr);
    }
}

void VirtIO_Net_ReceivePacket()
{
    while(receiveQueue.deviceArea->index != receiveQueue.lastDeviceAreaIndex)
    {
        terminal_writestring("     Packet received successfully!\n");

        // Get the index of the current descriptor from the device's ring buffer
        uint16_t ringBufferIndex = receiveQueue.lastDeviceAreaIndex % receiveQueue.elements;
        uint16_t descIndex = (uint16_t)receiveQueue.deviceArea->ringBuffer[ringBufferIndex].index;

        // Get pointer to the beginning of the buffer
        uint32_t *rxBegin = (uint32_t*)((uint32_t)receiveQueue.descriptors[descIndex].address);

        // Skip over virtio_net_hdr to get a pointer to the packet
        Ethernet_Header *packet = (Ethernet_Header *)((uint32_t)rxBegin + sizeof(virtio_net_hdr));

        EthernetProcessReceivedPacket(packet, mac_addr);

        // Place the used descriptor index back in the available ring (driver area)
        receiveQueue.driverArea->ringBuffer[receiveQueue.driverArea->index++] = descIndex;

        receiveQueue.lastDeviceAreaIndex++;
    }

    // notify the device that we've updated the availaible ring index
    VNet_Write_Register(REG_QUEUE_NOTIFY, RECEIVE_QUEUE_INDEX);
}
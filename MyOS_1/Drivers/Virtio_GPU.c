#include "Virtio_GPU.h"
#include "Bochs_VGA.h"
#include "../Terminal.h"
#include "../System_Specific.h"
#include "../misc.h"
#include "../Graphics/Display_HAL.h"
#include "PCI_Bus.h"
#include "Virtio.h"
#include "../printf.h"
#include "../paging.h"
#include "../Interrupts/PIC.h"
#include "../Interrupts/IDT.h"
#include "../Interrupts/Interrupts.h"

virtq controlQueue;
virtq cursorQueue;

uint32_t vGPU_commonConfigBaseAddress;
uint32_t vGPU_commonConfigOffset;
bool     vGPU_commonConfigIsIO;
volatile virtio_pci_common_cfg *pCommonConfig = NULL;

uint32_t vGPU_notificationBaseAddress;
uint32_t vGPU_notificationOffset;
bool     vGPU_notificationIsIO;
volatile virtio_pci_notify_cap *pNotifyCap = NULL;
volatile uint16_t *pNotificationArea = NULL;

uint32_t vGPU_ISRBaseAddress;
uint32_t vGPU_ISROffset;
bool     vGPU_ISRIsIO;
volatile uint8_t *pISR = NULL;

uint8_t  vGPU_IRQ;


// TODO: Support multiple displays (I'm not sure there would ever be multiple devices, but if so, support those too)

uint16_t VGPU_GetQueueSize()
{
    return pCommonConfig->queue_size;
}

void VGPU_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    terminal_writestring("    Initializing virtio-gpu driver...\n");

    // Read the pointer to the capabilities list
    uint8_t capPointer = PCI_GetCapabilitiesPointer(bus, slot, function);

    virtio_pci_cap caps = { 0 };
    caps.cap_next = capPointer;
    
    while(caps.cap_next)
    {
        VirtIO_Read_PCI_Capabilities(&caps, bus, slot, function, caps.cap_next);

        if (caps.cfg_type == VIRTIO_PCI_CAP_COMMON_CFG)
        {
            vGPU_commonConfigOffset = caps.offset;

            uint32_t bar = PCI_GetBAR(bus, slot, function, caps.bar);
            vGPU_commonConfigIsIO = bar & PCI_BAR_IS_IO;

            if (vGPU_commonConfigIsIO)
                vGPU_commonConfigBaseAddress = bar & PCI_BAR_IO_MASK;
            else
            {
                vGPU_commonConfigBaseAddress = bar & PCI_BAR_MMIO_MASK;
                pCommonConfig = (virtio_pci_common_cfg *)((uint32_t)vGPU_commonConfigBaseAddress + vGPU_commonConfigOffset);

                // Map the MMIO in the page table
                uint32_t mmioPage = (uint32_t)pCommonConfig / FOUR_MEGABYTES;
                pageDir[mmioPage] = ((mmioPage * FOUR_MEGABYTES)
                                     | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);
            }
        }
        if (caps.cfg_type == VIRTIO_PCI_CAP_NOTIFY_CFG)
        {
            //kprintf("notifty cfg found\n");

            vGPU_notificationOffset = caps.offset;

            kprintf("Off: %d\n", vGPU_notificationOffset);

            uint32_t bar = PCI_GetBAR(bus, slot, function, caps.bar);
            vGPU_notificationIsIO = bar & PCI_BAR_IS_IO;

            if (vGPU_notificationIsIO)
                vGPU_notificationBaseAddress = bar & PCI_BAR_IO_MASK;
            else
            {
                vGPU_notificationBaseAddress = bar & PCI_BAR_MMIO_MASK;
                pNotifyCap = (virtio_pci_notify_cap *)((uint32_t)vGPU_notificationBaseAddress + vGPU_notificationOffset);

                // Map the MMIO in the page table
                uint32_t mmioPage = (uint32_t)pNotifyCap / FOUR_MEGABYTES;
                pageDir[mmioPage] = ((mmioPage * FOUR_MEGABYTES)
                                     | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);

                kprintf("muilt: %d\n", pNotifyCap->notify_off_multiplier);
                kprintf("off: %d\n", pNotifyCap->cap.offset);

                pNotificationArea = (uint16_t*)vGPU_notificationBaseAddress + pNotifyCap->cap.offset;
                //*pNotificationArea = 0;
            }

        }
        if (caps.cfg_type == VIRTIO_PCI_CAP_ISR_CFG)
        {
            vGPU_ISROffset = caps.offset;

            uint32_t bar = PCI_GetBAR(bus, slot, function, caps.bar);
            vGPU_ISRIsIO = bar & PCI_BAR_IS_IO;

            if (vGPU_ISRIsIO)
                vGPU_ISRBaseAddress = bar & PCI_BAR_IO_MASK;
            else
            {
                vGPU_ISRBaseAddress = bar & PCI_BAR_MMIO_MASK;
                pISR = (uint8_t *)((uint32_t)vGPU_ISRBaseAddress + vGPU_ISROffset);

                // Map the MMIO in the page table
                uint32_t mmioPage = (uint32_t)pISR / FOUR_MEGABYTES;
                pageDir[mmioPage] = ((mmioPage * FOUR_MEGABYTES)
                                     | DIRECTORY_ENTRY_PRESENT | DIRECTORY_ENTRY_USER_ACCESSIBLE | DIRECTORY_ENTRY_WRITABLE | DIRECTORY_ENTRY_4MB);
            }
        }
    }
    
    kprintf("     Common Config: 0x%X (%s)", pCommonConfig, vGPU_commonConfigIsIO ? "I/O" : "MMIO");

    if (vGPU_commonConfigIsIO)
    {
        terminal_writestring("\nTODO: I/O access isn't implemented yet. Aborting.\n");
        return;
    }

    // get the IRQ
    vGPU_IRQ = PCI_GetInterruptLine(bus, slot, function);
    //kprintf(" - IRQ %d\n", vGPU_IRQ);

    // make sure an IRQ line is being used
    if (vGPU_IRQ == 0xFF)
    {
        terminal_writestring("      Can't use I/O APIC interrupts. Aborting.\n");
        return;
    }


    /*uint16_t irqWord = PCI_ConfigReadWord(bus, slot, function, INTERRUPT_LINE_OFFSET);

    // set IRQ to 9
    irqWord = (0xFF00 & irqWord) | 9;

    PCI_ConfigWriteWord(bus, slot, function, INTERRUPT_LINE_OFFSET, irqWord);
    */
    vGPU_IRQ = PCI_GetInterruptLine(bus, slot, function);
    kprintf(" - IRQ %d\n", vGPU_IRQ);

    // Reset the virtio-gpu device
    VGPU_WriteStatus(STATUS_RESET_DEVICE);

    // Set the acknowledge status bit
    VGPU_WriteStatus(STATUS_DEVICE_ACKNOWLEDGED);

    // Set the driver status bit
    VGPU_WriteStatus(STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED);

    // Read the feature bits
    volatile uint64_t features = VGPU_ReadFeatures();

    // Make sure the features we need are supported
    if ((features & REQUIRED_FEATURES) != REQUIRED_FEATURES)
    {
        // uh-oh
        terminal_writestring("\n      Required features are not supported by device. Aborting.\n");
        VGPU_WriteStatus(STATUS_DEVICE_ERROR);
        return;
    }

    // Tell the device what features we'll be using
    VGPU_WriteFeatures(REQUIRED_FEATURES);

    if (VGPU_ReadFeatures() != REQUIRED_FEATURES)
    {
        kprintf("Not good. Features are in fact: %llX\n", VGPU_ReadFeatures());
    }

    // Tell the device we're ok with the features we've negotiated
    VGPU_WriteStatus(STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED | STATUS_FEATURES_OK);

    /* Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
        reading and possibly writing the device’s virtio configuration space, and population of virtqueues. */
    VGPU_Init_Virtqueue(&controlQueue, VIRTIO_GPU_CONTROL_Q_INDEX);
    VGPU_Init_Virtqueue(&cursorQueue, VIRITO_GPU_CURSOR_Q_INDEX);

    // Setup an interrupt handler for this device

    // Are we using IRQ 9 or 11?
    if (vGPU_IRQ == 9 || vGPU_IRQ == 11)
    {
        // Support IRQ sharing
        Interrupts_Add_Shared_Handler(VGPU_SharedInterruptHandler, vGPU_IRQ);
    }
    else
    {
        // No irq sharing
        Set_IDT_Entry((unsigned long)VGPU_InterruptHandler, HARDWARE_INTERRUPTS_BASE + vGPU_IRQ);

        // Tell the PIC to enable the GPU's IRQ
        IRQ_Enable_Line(vGPU_IRQ);
    }

    // Set the "driver ready" status bit
    VGPU_WriteStatus(STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED | STATUS_FEATURES_OK | STATUS_DRIVER_READY);

    pCommonConfig->queue_select = VIRTIO_GPU_CONTROL_Q_INDEX;
    pCommonConfig->queue_enable = true;
    
    // Send get display info control command
    virtio_gpu_ctrl_hdr *ctrlHeader = malloc(sizeof(virtio_gpu_ctrl_hdr));
    memset(ctrlHeader, 0, sizeof(virtio_gpu_ctrl_hdr));

    ctrlHeader->type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
    
    // Get indices for the next two descriptors
    uint16_t descIndex = controlQueue.nextDescriptor % controlQueue.elements;
    ++controlQueue.nextDescriptor;

    // Get index for the next entry into the available-ring
    uint16_t index = controlQueue.driverArea->index % controlQueue.elements;

    //if (debugLevel)
    {
        kprintf("\ndescIndex %d", descIndex);
        kprintf("\nIndex %d", index);
    }

    // fill descriptor
    controlQueue.descriptors[descIndex].address = (uint64_t)ctrlHeader;
    controlQueue.descriptors[descIndex].flags = 0;
    controlQueue.descriptors[descIndex].length = sizeof(virtio_gpu_ctrl_hdr);
    controlQueue.descriptors[descIndex].next = 0;

    // Add descriptor chain to the available ring
    controlQueue.driverArea->ringBuffer[index] = descIndex;
    
    // Increase available ring index and notify the device
    controlQueue.driverArea->index++;

    // Setup buffers for device
    VGPU_SetupDeviceBuffers();

    //VNet_Write_Register(REG_QUEUE_NOTIFY, TRANSMIT_QUEUE_INDEX);
    //pCommonConfig->
    *pNotificationArea = VIRTIO_GPU_CONTROL_Q_INDEX;
    
    graphicsPresent = true;

    terminal_writestring("    virtio-gpu initialized\n");
}

// TODO: Error-checking, etc
void VGPU_Init_Virtqueue(virtq *virtqueue, uint16_t queueIndex)
{
    int16_t queueSize = -1;

    // access the current queue
    VGPU_SelectQueue(queueIndex);

    // get the size of the current queue
    queueSize = VGPU_GetQueueSize();

    if (!queueSize)
        return;

    if (debugLevel)
        kprintf("\n      queue %d has %d elements", queueIndex, queueSize);

    // Allocate and initialize the queue
    VirtIO_Allocate_Virtqueue(virtqueue, queueSize);

    if (debugLevel)
    {
        kprintf("\n       queue %d: 0x%X", queueIndex, virtqueue->descriptors);
        kprintf("         queue size: %d", virtqueue->elements);
        kprintf("\n       driverArea: 0x%X", virtqueue->driverArea);
        kprintf("\n       deviceArea: 0x%X", virtqueue->deviceArea);
    }

    // TODO: Convert virtual address to physical address
    // (This isn't required now because all addresses malloc returns are identity mapped)

    // Set descriptor address
    VGPU_SetQueueAddresses(virtqueue);
}

void _declspec(naked) VGPU_InterruptHandler()
{
    _asm pushad;

    ++interrupts_fired;

    //if (debugLevel)
    terminal_writestring(" --------- TODO: VGPU interrupt fired! -------\n");

    uint8_t status;
    status = *pISR;
    // Get the interrupt status (This will also reset the isr status register)
    /*uint32_t isr;
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
    if (debugLevel)
    terminal_writestring("Transmit success\n");
    transmitQueue.lastDeviceAreaIndex++;
    }

    // see if the receive queue has been used
    if (receiveQueue.deviceArea->index != receiveQueue.lastDeviceAreaIndex)
    {
    VirtIO_Net_ReceivePacket();
    }
    }*/

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + vGPU_IRQ);

    if (debugLevel)
        terminal_writestring(" --------- VGPU interrupt done! -------\n");

    _asm
    {
        popad
        iretd
    }
}

bool VGPU_SharedInterruptHandler(void)
{
    ++interrupts_fired;
    bool retVal;
    retVal = false;

    //if (debugLevel)
        terminal_writestring(" --------- VGPU interrupt fired! -------\n");

    // Get the interrupt status (This will also reset the isr status register)
    uint8_t isr;
    isr = *pISR;

    // TODO: Support configuration changes (doubt this will ever happen)
    if (isr & VIRTIO_ISR_CONFIG_CHANGED)
    {
        terminal_writestring("TODO: VirtIO-GPU configuration has changed\n");
        retVal = true;
    }

    // Check for used queues
    if (isr & VIRTIO_ISR_VIRTQ_USED)
    {
        terminal_writestring("Virtq used\n");

       /* // see if the transmit queue has been used
        while (transmitQueue.deviceArea->index != transmitQueue.lastDeviceAreaIndex)
        {
        if (debugLevel)
        terminal_writestring("Transmit success\n");
        transmitQueue.lastDeviceAreaIndex++;
        }

        // see if the receive queue has been used
        if (receiveQueue.deviceArea->index != receiveQueue.lastDeviceAreaIndex)
        {
        VirtIO_Net_ReceivePacket();
        }*/

        retVal = true;
    }

    if (debugLevel)
        terminal_writestring(" --------- VGPU interrupt done! -------\n");
    
    return retVal;
    //return false;
}

/*
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
*/

uint64_t VGPU_ReadFeatures(void)
{
    uint64_t features;

    pCommonConfig->device_feature_select = 0;
    features = pCommonConfig->device_feature;
    
    pCommonConfig->device_feature_select = 1;
    features |= ((uint64_t)pCommonConfig->device_feature) << 32;

    return features;
}

inline uint8_t VGPU_ReadStatus(void)
{
    return pCommonConfig->device_status;
}

inline void VGPU_SelectQueue(uint16_t queueIndex)
{
    pCommonConfig->queue_select = queueIndex;
}

void VGPU_SetupDeviceBuffers()
{
    const uint16_t bufferSize = 4096;

    // Allocate and add 16 buffers for the device to use
   // for (uint16_t i = 0; i < 16; ++i)
    {
        uint8_t *buffer = malloc(bufferSize);
        uint16_t nextDesc = controlQueue.nextDescriptor++;

        // Add buffer to the descriptor table
        controlQueue.descriptors[nextDesc].address = (uint64_t)buffer;
        controlQueue.descriptors[nextDesc].flags = VIRTQ_DESC_F_DEVICE_WRITE_ONLY | (1 << 7);
        controlQueue.descriptors[nextDesc].length = bufferSize;
        controlQueue.descriptors[nextDesc].next = 0;

        // Add index of descriptor to the driver ring
        controlQueue.driverArea->ringBuffer[controlQueue.driverArea->index] = nextDesc;//.driverArea->ringBuffer[i].index = i;
        //controlQueue.driverArea->ringBuffer[i].length = bufferSize;
    }
      
    // Update next available index
    controlQueue.driverArea->index++;

    //controlQueue.nextDescriptor = 16;

    // Notify the device of the updated queue
    //VNet_Write_Register(REG_QUEUE_NOTIFY, RECEIVE_QUEUE_INDEX);
    *pNotificationArea = VIRTIO_GPU_CONTROL_Q_INDEX;
}

inline void VGPU_SetQueueAddresses(virtq *virtqueue)
{
    pCommonConfig->queue_desc = (uint64_t)virtqueue->descriptors;
    pCommonConfig->queue_driver = (uint64_t)virtqueue->driverArea;
    pCommonConfig->queue_device = (uint64_t)virtqueue->deviceArea;
    
}

inline void VGPU_WriteFeatures(uint64_t features)
{
    pCommonConfig->driver_feature_select = 0;
    pCommonConfig->driver_feature = (uint32_t)(features & 0xFFffFFff);

    pCommonConfig->driver_feature_select = 1;
    pCommonConfig->driver_feature = 0;// (uint32_t)(features >> 32);
}

inline void VGPU_WriteStatus(uint8_t status)
{
    pCommonConfig->device_status = status;
}
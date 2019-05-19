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

virtq controlQueue;
virtq cursorQueue;

uint32_t vGPU_commonConfigBaseAddress;
uint32_t vGPU_commonConfigOffset;
bool     vGPU_commonConfigIsIO;
uint8_t  vGPU_IRQ;
volatile virtio_pci_common_cfg *pCommonConfig;

// TODO: Support multiple displays (I'm not sure there would ever be multiple devices, but if so, support those too)

void VGPU_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    //BGA_bus = bus;
    //BGA_slot = slot;
    //BGA_function = function;

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
    }
    
    kprintf("     Common Config: 0x%X (%s)", pCommonConfig, vGPU_commonConfigIsIO ? "I/O" : "MMIO");

    if (vGPU_commonConfigIsIO)
    {
        terminal_writestring("\nTODO: I/O access isn't implemented yet. Aborting.\n");
        return;
    }

    // get the IRQ
    vGPU_IRQ = PCI_GetInterruptLine(bus, slot, function);
    kprintf(" - IRQ %d\n", vGPU_IRQ);

    // make sure an IRQ line is being used
    if (vGPU_IRQ == 0xFF)
    {
        terminal_writestring("      Can't use I/O APIC interrupts. Aborting.\n");
        return;
    }

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

    // Tell the device we're ok with the features we've negotiated
    VGPU_WriteStatus(STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED | STATUS_FEATURES_OK);

    /* Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
        reading and possibly writing the device’s virtio configuration space, and population of virtqueues. */

    // Set the "driver ready" status bit
    VGPU_WriteStatus(STATUS_DEVICE_ACKNOWLEDGED | STATUS_DRIVER_LOADED | STATUS_FEATURES_OK | STATUS_DRIVER_READY);

    graphicsPresent = true;

    terminal_writestring("    virtio-gpu initialized\n");
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

inline void VGPU_WriteFeatures(uint64_t features)
{
    pCommonConfig->driver_feature_select = 0;
    pCommonConfig->driver_feature = (uint32_t)(features & 0xFFffFFff);

    pCommonConfig->driver_feature_select = 1;
    pCommonConfig->driver_feature = (uint32_t)(features >> 32);
}

inline void VGPU_WriteStatus(uint8_t status)
{
    pCommonConfig->device_status = status;
}
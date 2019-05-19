#include <stdint.h>
#include "Virtio.h"
#include "../misc.h"
#include "../printf.h"
#include "PCI_Bus.h"


// TODO: Error-checking
void VirtIO_Allocate_Virtqueue(virtq *virtqueue, uint16_t queueSize)
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

    if (debugLevel)
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

// Read the capabilities list, one dword at a time
void VirtIO_Read_PCI_Capabilities(virtio_pci_cap *caps, uint8_t bus, uint8_t slot, uint8_t function, uint8_t capPointer)
{
    uint8_t offset;
    for (offset = 0; offset < sizeof(virtio_pci_cap); offset += sizeof(uint32_t))
    {
        uint32_t currentDWord = PCI_ConfigReadDWord(bus, slot, function, capPointer + offset);
        memcpy((void *)((uint32_t)caps + offset), &currentDWord, sizeof(uint32_t));
    }

    // size of virtio_pci_cap should be a multiple of 4
    if (offset != sizeof(virtio_pci_cap))
        terminal_writestring("virtio_pci_cap has unexpected size!\n");

    //kprintf("cfg_type: 0x%X\n", caps->cfg_type);

    //kprintf("cap_vndr: 0x%X\ncap_next: 0x%X\ncap_len: 0x%X\ncfg_type: 0x%X\n", caps->cap_vndr, caps->cap_next, caps->cap_len, caps->cfg_type);
    //kprintf("bar: %d\noffset: 0x%X\nlength: 0x%X\n", caps->bar, caps->offset, caps->length);
}
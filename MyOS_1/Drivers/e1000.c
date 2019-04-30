// Driver file for the intel 8254x line of network cards. Particularly the 82540em.

#include "e1000.h"
#include "../Terminal.h"

void e1000_Net_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    (void)bus; (void)slot; (void)function;
    // get the I/O port
    //vNet_base_port = PCI_GetBaseAddress0(bus, slot, function) & ~3;

    terminal_writestring("    Initializing e1000 driver...");/*     Base address : ");
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
    /*
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
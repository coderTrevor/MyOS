#include "PCI_Bus.h"
#include <stdint.h>
#include "../System_Specific.h"
#include "../Terminal.h"
#include "RTL_8139.h"
#include "Bochs_VGA.h"
#include "Virtio_Net.h"

char *PCI_GetVendorName(uint16_t vendorID)
{
    switch (vendorID)
    {
        case 0x1022:
            return "AMD";
        case 0x106B:
            return "Apple Inc.";
        case PCI_VENDOR_REALTEK:
            return "Realtek";
        case PCI_VENDOR_RED_HAT:
            return "Red Hat, Inc.";
        case 0x1234:
            return "QEMU";
        case 0x15AD:
            return "VMWare";
        case 0x80ee:
            return "VirtualBox";
        case 0x8086:
            return "Intel";
    }

    return "Unknown vendor";
}

char *PCI_GetClassName(uint8_t baseClass)
{
    switch (baseClass)
    {
        case 0x00:
            return "Unclassified";
        case 0x01:
            return "Mass Storage Controller";
        case 0x02:
            return "Network Controller";
        case 0x03:
            return "Display Controller";
        case 0x04:
            return "Multimedia Controller";
        case 0x05:
            return "Memory Controller";
        case 0x06:
            return "Bridge Device";
        case 0x07:
            return "Simple Communication Controller";
        case 0x08:
            return "Base System Peripheral";
        case 0x09:
            return "Input Device Controller";
        case 0x0A:
            return "Docking Station";
        case 0x0B:
            return "Processor";
        case 0x0C:
            return "Serial Bus Controller";
        case 0x0D:
            return "Wireless Controller";
        case 0x0E:
            return "Intelligent Controller";
        case 0x0F:
            return "Satellite Communication Controller";
        case 0x10:
            return "Encryption Controller";
        case 0x11:
            return "Signal Processing Controller";
        case 0x12:
            return "Processing Accelerator";
        case 0x13:
            return "Non-Essential Instrumentation";
        case 0x40:
            return "Co-Processor";
        case 0xFF:
            return "Unassigned Class";
    }

    return "[Unknown PCI class]";
}

uint16_t PCI_GetCommand(uint8_t bus, uint8_t device, uint8_t function)
{
    return PCI_ConfigReadWord(bus, device, function, COMMAND_OFFSET);
}

char *PCI_GetSubclassName(uint8_t baseClass, uint8_t subclass)
{
    // from https://wiki.osdev.org/PCI
    switch (baseClass)
    {
        case 0x00:
            // unclassified
            if (subclass == 0)
                return "Non-VGA-Compatible";
            if (subclass == 1)
                return "VGA-Compatible";
            break;
        case 0x01:
            // mass storage controller
            if (subclass == 0)
                return "SCSI Bus Controller";
            if (subclass == 1)
                return "IDE Controller";
            if (subclass == 2)
                return "Floppy Disk Controller";
            if (subclass == 3)
                return "IPI Bus Controller";
            if (subclass == 4)
                return "RAID Controller";
            if (subclass == 5)
                return "ATA Controller";
            if (subclass == 6)
                return "Serial ATA";
            if (subclass == 7)
                return "Serial Attached SCSI";
            if (subclass == 8)
                return "Non-Volatile Memory Controller";
            if (subclass == 0x80)
                return "\"Other\"";
            break;
        case 0x02:
            // Network Controller
            switch (subclass)
            {
                case 0x00:
                    return "Ethernet Controller";
                case 0x01:
                    return "Token Ring Controller";
                case 0x02:
                    return "FDDI Controller";
                case 0x03:
                    return "ATM Controller";
                case 0x04:
                    return "ISDN Controller";
                case 0x05:
                    return "WorldFip Controller";
                case 0x06:
                    return "PICMG 2.14 Multi Computing";
                case 0x07:
                    return "Infiniband Controller";
                case 0x08:
                    return "Fabric Controller";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x03:
            // Display Controller
            if (subclass == 0x00)
                return "VGA Compatible Controller";
            if (subclass == 0x01)
                return "XGA Controller";
            if (subclass == 0x02)
                return "3D Controller(Not VGA - Compatible)";
            if (subclass == 0x80)
                return "\"Other\"";
            break;
        case 0x04:
            // Multimedia Controller
            switch (subclass)
            {
                case 0x00:
                    return "Multimedia Video Controller";
                case 0x01:
                    return "Multimedia Audio Controller";
                case 0x02:
                    return "Computer Telephony Device";
                case 0x03:
                    return "Audio Device";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x05:
            // Memory Controller
            if (subclass == 0x00)
                return "RAM Controller";
            if (subclass == 0x01)
                return "Flash Controller";
            if (subclass == 0x80)
                return "\"Other\"";
            break;
        case 0x06:
            // Bridge Device
            switch (subclass)
            {
                case 0x00:
                    return "Host Bridge";
                case 0x01:
                    return "ISA Bridge";
                case 0x02:
                    return "EISA Bridge";
                case 0x03:
                    return "MCA Bridge";
                case 0x04:
                    return "PCI - to - PCI Bridge";
                case 0x05:
                    return "PCMCIA Bridge";
                case 0x06:
                    return "NuBus Bridge";
                case 0x07:
                    return "CardBus Bridge";
                case 0x08:
                    return "RACEway Bridge";
                case 0x09:
                    return "PCI - to - PCI Bridge";
                case 0x0A:
                    return "InfiniBand - to - PCI Host Bridge";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x07:
            // Simple Communication Controller
            switch (subclass)
            {
                case 0x00:
                    return "Serial Controller";
                case 0x01:
                    return "Parallel Controller";
                case 0x02:
                    return "Multiport Serial Controller";
                case 0x03:
                    return "Modem";
                case 0x04:
                    return "IEEE 488.1 / 2 (GPIB)Controller";
                case 0x05:
                    return "Smart Card";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x08:
            // Base System Peripheral
            switch (subclass)
            {
                case 0x00:
                    return "PIC";
                case 0x01:
                    return "DMA Controller";
                case 0x02:
                    return "Timer";
                case 0x03:
                    return "RTC Controller";
                case 0x04:
                    return "PCI Hot-Plug Controller";
                case 0x05:
                    return "SD Host Controller";
                case 0x06:
                    return "IOMMU";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x09:
            // Input Device Controller
            switch (subclass)
            {
                case 0x00:
                    return "Keyboard Controller";
                case 0x01:
                    return "Digitizer Pen";
                case 0x02:
                    return "Mouse Controller";
                case 0x03:
                    return "Scanner Controller";
                case 0x04:
                    return "Gameport Controller";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x0A:
            // Docking Station
            switch (subclass)
            {
                case 0x00:
                    return "Generic";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x0B:
            // Processor
            switch (subclass)
            {
                case 0x00:
                    return "386";
                case 0x01:
                    return "486";
                case 0x02:
                    return "Pentium";
                case 0x10:
                    return "Alpha";
                case 0x20:
                    return "PowerPC";
                case 0x30:
                    return "MIPS";
                case 0x40:
                    return "Co-Processor";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x0C:
            // Serial Bus Controller
            switch (subclass)
            {
                case 0x00:
                    return "FireWire (IEEE 1394) Controller";
                case 0x01:
                    return "ACCESS Bus";
                case 0x02:
                    return "SSA";
                case 0x03:
                    return "USB Controller";
                case 0x04:
                    return "Fibre Channel";
                case 0x05:
                    return "SMBus";
                case 0x06:
                    return "InfiniBand";
                case 0x07:
                    return "IPMI Interface";
                case 0x08:
                    return "SERCOS Interface (IEC 61491)";
                case 0x09:
                    return "CANbus";
            }
            break;
        case 0x0D:
            // Wireless Controller
            switch (subclass)
            {
                case 0x00:
                    return "iRDA Compatible Controller";
                case 0x01:
                    return "Consumer IR Controller";
                case 0x10:
                    return "RF Controller";
                case 0x11:
                    return "Bluetooth Controller";
                case 0x12:
                    return "Broadband Controller";
                case 0x20:
                    return "Ethernet Controller (802.1a)";
                case 0x21:
                    return "Ethernet Controller (802.1b)";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x0E:
            // Intelligent Controller
            if (subclass == 0)
                return "I20";
            break;
        case 0x0F:
            // Satellite Communication Controller
            switch (subclass)
            {
                case 0x01:
                    return "Satellite TV Controller";
                case 0x02:
                    return "Satellite Audio Controller";
                case 0x03:
                    return "Satellite Voice Controller";
                case 0x04:
                    return "Satellite Data Controller";
            }
            break;
        case 0x10:
            // Encryption Controller
            switch (subclass)
            {
                case 0x00:
                    return "Network and Computing Encrpytion/Decryption";
                case 0x10:
                    return "Entertainment Encryption/Decryption";
                case 0x80:
                    return "Other Encryption/Decryption";
            }
            break;
        case 0x11:
            // Signal Processing Controller
            switch (subclass)
            {
                case 0x00:
                    return "DPIO Modules";
                case 0x01:
                    return "Performance Counters";
                case 0x10:
                    return "Communication Synchronizer";
                case 0x20:
                    return "Signal Processing Management";
                case 0x80:
                    return "\"Other\"";
            }
            break;
        case 0x12:
            // Processing Accelerator
            return "";
            break;
        case 0x13:
            // Non-Essential Instrumentation
            return "";
            break;
        case 0x40:
            // Co-Processor
            return "";
            break;
    }

    return "[Unknown PCI subclass]";
}

void PCI_CheckAllBuses(void)
{
    uint16_t bus;
    uint8_t device;

    for (bus = 0; bus < 256; ++bus)
    {
        for (device = 0; device < 32; ++device)
        {
            PCI_CheckDevice((uint8_t)bus, device);
        }
    }
}

void PCI_CheckDevice(uint8_t bus, uint8_t device) 
{
    uint8_t function = 0;

    uint16_t vendorID = PCI_GetVendorID(bus, device, 0);
    
    if (vendorID == 0xFFFF) 
        return;        // Device doesn't exist
    
    terminal_writestring("Found PCI Device with vendor ID ");
    terminal_print_ushort_hex(vendorID);
    terminal_writestring(", (");
    terminal_writestring(PCI_GetVendorName(vendorID));
    terminal_writestring(")\n");

    PCI_CheckFunction(bus, device, function, vendorID);

    uint8_t headerType = PCI_GetHeaderType(bus, device, 0);

    if (headerType & MULTIFUNCTION)
    {
        // It is a multi-function device, so check remaining functions 
        for (function = 1; function < 8; ++function)
        {
            vendorID = PCI_GetVendorID(bus, device, function);
            if (vendorID != 0xFFFF)
            {
                PCI_CheckFunction(bus, device, function, vendorID);
            }
        }
    }
}

void PCI_Init()
{
    terminal_writestring("Initializing PCI bus...\n");
    PCI_CheckAllBuses();
    terminal_writestring("PCI bus initialized\n");
}

bool PCI_IsBusMasteringEnabled(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_GetCommand(bus, slot, function) & BUS_MASTER_ENABLED;
}

void PCI_CheckFunction(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendorID) 
{
    uint8_t baseClass;
    uint8_t subClass;
    //uint8_t secondaryBus;

    uint16_t classes = PCI_GetClassCodes(bus, device, function);
    uint16_t deviceID = PCI_GetDeviceID(bus, device, function);

    // base class is the upper 8 bits
    baseClass = (classes & 0xFF00) >> 8;

    // subclass is the lower 8 bits
    subClass = classes & 0xFF;

    // print string from the base class and subclass
    terminal_writestring("   Fnctn ");
    terminal_print_int(function);
    terminal_writestring(" - ");
    terminal_print_ushort_hex(deviceID);
    terminal_writestring(" - ");
    terminal_writestring( PCI_GetClassName(baseClass) );
    terminal_writestring(" - ");
    terminal_writestring( PCI_GetSubclassName(baseClass, subClass) );
    terminal_newline();

    // try to load a driver for the device
    PCI_DelegateToDriver(bus, device, function, vendorID, deviceID);

    /*if ((baseClass == 0x06) && (subClass == 0x04))
    {
        secondaryBus = getSecondaryBus(bus, device, function);
        checkBus(secondaryBus);
    }*/
}

// Inelegant hack
void PCI_DelegateToDriver(uint8_t bus, uint8_t slot, uint8_t function, uint16_t vendorID, uint16_t deviceID)
{
    if (vendorID == PCI_VENDOR_REALTEK)
    {
        if (deviceID == PCI_DEVICE_RTL_8139)
            RTL_8139_Init(bus, slot, function);
        return;
    }

    if (vendorID == PCI_VENDOR_QEMU)
    {
        if (deviceID == PCI_DEVICE_BGA)
            BGA_Init(bus, slot, function);
        return;
    }

    if (vendorID == PCI_VENDOR_VBOX)
    {
        if (deviceID == PCI_DEVICE_VBOX_BGA)
            BGA_Init(bus, slot, function);
        return;
    }

    if (vendorID == PCI_VENDOR_RED_HAT)
    {
        if (deviceID == PCI_DEVICE_VIRTIO_NET)
            VirtIO_Net_Init(bus, slot, function);
        return;
    }
}

void PCI_EnableBusMastering(uint8_t bus, uint8_t slot, uint8_t function)
{
    // read in the old command word
    uint16_t command = PCI_ConfigReadWord(bus, slot, function, COMMAND_OFFSET);

    // ensure the bus mastering bit is set
    command |= BUS_MASTER_ENABLED;

    // write out the command word
    PCI_SetCommand(bus, slot, function, command);
}

uint32_t PCI_GetBaseAddress0(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadDWord(bus, slot, function, BAR0_OFFSET);
}

uint32_t PCI_GetBaseAddress1(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadDWord(bus, slot, function, BAR1_OFFSET);
}

uint32_t PCI_GetBaseAddress2(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadDWord(bus, slot, function, BAR2_OFFSET);
}

uint32_t PCI_GetBaseAddress3(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadDWord(bus, slot, function, BAR3_OFFSET);
}

uint32_t PCI_GetBaseAddress4(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadDWord(bus, slot, function, BAR4_OFFSET);
}

uint32_t PCI_GetBaseAddress5(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadDWord(bus, slot, function, BAR5_OFFSET);
}


uint16_t PCI_GetClassCodes(uint8_t bus, uint8_t device, uint8_t function)
{
    return PCI_ConfigReadWord(bus, device, function, CLASSES_OFFSET);
}

uint16_t PCI_GetDeviceID(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadWord(bus, slot, function, DEVICE_ID_OFFSET);
}

uint16_t PCI_ConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    /* create configuration address as per Figure 1 (on the OSDev wiki)*/
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
        (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(PCI_CONFIG_ADDRESS, address);

    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

void PCI_ConfigWriteDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data)
{
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
        (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(PCI_CONFIG_ADDRESS, address);

    /* write out the data */
    outl(PCI_CONFIG_DATA, data);
}

uint32_t PCI_ConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    /* create configuration address */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
        (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(PCI_CONFIG_ADDRESS, address);

    /* read in the data */
    return inl(PCI_CONFIG_DATA);
}

uint8_t PCI_GetHeaderType(uint8_t bus, uint8_t slot, uint8_t function)
{
    // Header type will be the lower 8 bytes starting at offset 0x0E
    return PCI_ConfigReadWord(bus, slot, function, HEADER_TYPE_OFFSET) & 0xFF;
}

uint8_t PCI_GetInterruptLine(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadWord(bus, slot, function, INTERRUPT_LINE_OFFSET) & 0xFF;
}

uint16_t PCI_GetVendorID(uint8_t bus, uint8_t slot, uint8_t function)
{
    return PCI_ConfigReadWord(bus, slot, function, VENDOR_ID_OFFFSET);
}

void PCI_SetCommand(uint8_t bus, uint8_t device, uint8_t function, uint16_t command)
{
    // read in the uint32_t associated with command (command and status)
    uint32_t dataDWORD = PCI_ConfigReadDWord(bus, device, function, COMMAND_OFFSET);

    // modify the command part of the data, overwriting the old command
    dataDWORD = (dataDWORD & 0xFFFF0000) | command;

    // write out the new dword
    PCI_ConfigWriteDWord(bus, device, function, COMMAND_OFFSET, dataDWORD);
}
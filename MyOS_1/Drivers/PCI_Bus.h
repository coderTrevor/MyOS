#pragma once
#include <stdbool.h>
#include <stdint.h>

#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

#define VENDOR_ID_OFFFSET   0
#define DEVICE_ID_OFFSET    0x02
#define COMMAND_OFFSET      0x04
#define CLASSES_OFFSET      0x0A
#define HEADER_TYPE_OFFSET  0x0E
#define BAR0_OFFSET         0x10
#define BAR1_OFFSET         0x14
#define BAR2_OFFSET         0x18
#define BAR3_OFFSET         0x1C
#define BAR4_OFFSET         0x20
#define BAR5_OFFSET         0x24
#define INTERRUPT_LINE_OFFSET   0x3C
#define MULTIFUNCTION       0x80

#define BUS_MASTER_ENABLED  0x04

#define PCI_VENDOR_REALTEK  0x10EC
#define PCI_DEVICE_RTL_8139 0x8139

#define PCI_VENDOR_QEMU     0x1234
#define PCI_DEVICE_BGA      0x1111

#define PCI_VENDOR_VBOX     0x80EE
#define PCI_DEVICE_VBOX_BGA 0xBEEF

#define PCI_VENDOR_RED_HAT    0x1AF4
#define PCI_DEVICE_VIRTIO_NET 0x1000
#define PCI_DEVICE_VIRTIO_GPU 0x1050

#define PCI_VENDOR_INTEL    0x8086
#define PCI_DEVICE_82540EM  0x100E

void PCI_CheckAllBuses(void);

void PCI_CheckDevice(uint8_t bus, uint8_t device);

void PCI_CheckFunction(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendorID);

uint32_t PCI_ConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

uint16_t PCI_ConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

void PCI_ConfigWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data);

void PCI_DelegateToDriver(uint8_t bus, uint8_t slot, uint8_t function, uint16_t vendorID, uint16_t deviceID);

void PCI_EnableBusMastering(uint8_t bus, uint8_t slot, uint8_t function);

uint32_t PCI_GetBaseAddress0(uint8_t bus, uint8_t slot, uint8_t function);
uint32_t PCI_GetBaseAddress1(uint8_t bus, uint8_t slot, uint8_t function);
uint32_t PCI_GetBaseAddress2(uint8_t bus, uint8_t slot, uint8_t function);
uint32_t PCI_GetBaseAddress3(uint8_t bus, uint8_t slot, uint8_t function);
uint32_t PCI_GetBaseAddress4(uint8_t bus, uint8_t slot, uint8_t function);
uint32_t PCI_GetBaseAddress5(uint8_t bus, uint8_t slot, uint8_t function);

uint16_t PCI_GetClassCodes(uint8_t bus, uint8_t device, uint8_t function);

char *PCI_GetClassName(uint8_t baseClass);

uint16_t PCI_GetCommand(uint8_t bus, uint8_t device, uint8_t function);

uint16_t PCI_GetDeviceID(uint8_t bus, uint8_t slot, uint8_t function);

uint8_t PCI_GetHeaderType(uint8_t bus, uint8_t slot, uint8_t function);

uint8_t PCI_GetInterruptLine(uint8_t bus, uint8_t slot, uint8_t function);

char *PCI_GetSubclassName(uint8_t baseClass, uint8_t subClass);

uint16_t PCI_GetVendorID(uint8_t bus, uint8_t slot, uint8_t function);

char *PCI_GetVendorName(uint16_t vendorID);

void PCI_Init();

void PCI_SetCommand(uint8_t bus, uint8_t device, uint8_t function, uint16_t command);

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define IOADDR  0x00
#define IODATA  0x04

// Register definitions
        // register     offset
#define REG_CTRL        0x00
#define REG_STATUS      0x08
#define REG_EECD        0x10
#define REG_EERD        0x14
#define REG_FLA         0x1C
#define REG_CTRL_EXT    0x18
#define REG_MDIC        0x20
#define REG_FCAL        0x28
#define REG_FCAH        0x2C
#define REG_FCT         0x30
#define REG_VET         0x38
#define REG_FCTTV       0x170
// ...
#define REG_MAC_LOW     0x5400  /* AKA Receive Address Low (n), RAL(8*n)  */
#define REG_MAC_HIGH    0x5404  /* AKA Receive Address High (n), RAH(8*n) */


// defines for PCI BAR bits
#define BAR_MMIO_OR_IO_BIT   1
#define BAR_USING_IO         1
#define BAR_ADDRESS_TYPE     6
#define BAR_64BIT            4
#define BAR_32BIT            0
#define BAR_ADDRESS          (~0x0F)

void e1000_Net_Init(uint8_t bus, uint8_t slot, uint8_t function);

uint32_t e1000_Read_Register(uint32_t reg);

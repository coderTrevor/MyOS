#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../Networking/Ethernet.h"

#define IOADDR  0x00
#define IODATA  0x04

// Register definitions (despite what the offsets might suggest, all registers are 32-bit unless otherwise noted)
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
#define REG_TCTL        0x400
#define REG_TIPG        0x410
// ...
#define REG_TDBAL       0x3800
#define REG_TDBAH       0x3804
#define REG_TDLEN       0x3808
#define REG_TDH         0x3810
#define REG_TDT         0x3818
// ...
#define REG_MAC_LOW     0x5400  /* AKA Receive Address Low(n), RAL(8*n)  */
#define REG_MAC_HIGH    0x5404  /* AKA Receive Address High(n), RAH(8*n) */


// defines for PCI BAR bits
#define BAR_MMIO_OR_IO_BIT   1
#define BAR_USING_IO         1
#define BAR_ADDRESS_TYPE     6
#define BAR_64BIT            4
#define BAR_32BIT            0
#define BAR_ADDRESS          (~0x0F)


// defines for CTRL register
#define CTRL_FULL_DUPLEX            0x01
#define CTRL_HALF_DUPLEX            0x00
#define CTRL_AUTO_SPEED_DETECT      0x10
#define CTRL_SET_LINK_UP            0x20
#define CTRL_INVERT_LOSS_OF_SIGNAL  0x40
#define CTRL_SPEED_10_MBS           0x00
#define CTRL_SPEED_100_MBS          0x100
#define CTRL_SPEED_200_MBS          0x200
#define CTRL_FORCE_SPEED            0x800
#define CTRL_FORCE_DUPLEX           0x1000
#define CTRL_PHY_RESET              0x80000000

// defines for TCTL register
#define TCTL_TX_ENABLE                      2
#define TCTL_PAD_SHORT_PACKET               8
#define TCTL_DEFAULT_COLLISION_THRESHOLD    (0x10 << 4)   /* CT = 10h, CT is in bits 11:4 */
#define TCTL_DEFAULT_COLLISION_DISTANCE     (0x40 << 12)
#define TCTL_RETRANSMIT_ON_LATE_COLLISION   0x1000000

// defines for TIPG register
#define TIPG_DEFAULTS   (10 | (10 << 10) | (10 << 20))

// Layout of transmit descriptor (TDESC) 
typedef struct TX_DESC_LEGACY
{
    uint64_t bufferAddress;
    uint16_t length;
    uint8_t  checksumOffset;
    uint8_t  command;
    uint8_t  status;
    uint8_t  checksumStart;
    uint16_t  special;
} TX_DESC_LEGACY;

// defines for command bits of tdesc
#define TDESC_CMD_EOP               1
#define TDESC_CMD_INSERT_FCS        2
#define TDESC_CMD_INSERT_CHECKSUM   4
#define TDESC_CMD_REPORT_STATUS     8
#define TDESC_CMD_REPORT_PACKET_SENT    16  /* used by 82544GC/EI only */

// defines for status bits
#define TDESC_STATUS_DESCRIPTOR_DONE    1

#define TX_DESCRIPTORS  768

void e1000_Net_Init(uint8_t bus, uint8_t slot, uint8_t function);

uint32_t e1000_Read_Register(uint32_t reg);

void e1000_SendPacket(Ethernet_Header *packet, uint16_t dataSize);

void e1000_TX_Init();

void e1000_Write_Register(uint32_t regOffset, uint32_t value);

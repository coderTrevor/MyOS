#pragma once

#include "PCI_Bus.h"

// Programming interface bits (from the PCI IDE specification)
// (No idea what "modes" the spec is referring to - it's not a great spec)
#define PROG_IF_PRIMARY_CHANNEL_PCI_NATIVE      0x01
#define PROG_IF_PRIMARY_CHANNEL_BOTH_MODES      0x02
#define PROG_IF_SECONDARY_CHANNEL_PCI_NATIVE    0x04
#define PROG_IF_SECONDARY_CHANNEL_BOTH_MODES    0x08
#define PROG_IF_BUS_MASTER_CAPABLE              0x80

#define IDE_COMPAT_PRIMARY_COMMAND_IO           0x1F0
#define IDE_COMPAT_PRIMARY_CONTROL_IO           0x3F6
#define IDE_COMPAT_SECONDARY_COMMAND_IO         0x170
#define IDE_COMPAT_SECONDARY_CONTROL_IO         0x376

// Some defines from OSdev wiki:

// STATUS
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// ERRORS
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

// COMMANDS
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC
// ATAPI COMMANDS
#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

// NOT SURE
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define ATA_PRIMARY_CHANNEL      0
#define ATA_SECONDARY_CHANNEL    1
#define ATA_CHANNEL_DEVICE0      0x00
#define ATA_CHANNEL_DEVICE1      0x01

// ATA-ATAPI "Task-File":
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// defines for control register
#define ATA_CONTROL_REG_DISABLE_INTERRUPTS  2
#define ATA_CONTROL_REG_SOFTWARE_RESET      4
// Other registers are reserved or 0

typedef struct IDE_CHANNEL_INFO
{
    uint16_t base;  // I/O Base.
    uint16_t ctrl;  // Control Base
    uint16_t bmide; // Bus Master IDE
    uint8_t  disableInterrupts;
    uint8_t irq;
} IDE_CHANNEL_INFO;

#define IDE_MODEL_STRING_LENGTH 40 /* not including null-terminator */

struct ide_device {
    uint8_t  Present;     // 0 (Empty) or 1 (This Drive really exists).
    uint8_t  Channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
    uint8_t  Drive;       // 0 (Master Drive) or 1 (Slave Drive).
    uint16_t Type;        // 0: ATA, 1:ATAPI.
    uint16_t Signature;   // Drive Signature
    uint16_t Capabilities;// Features.
    uint32_t CommandSets; // Command Sets Supported.
    uint32_t Size;        // Size in Sectors.
    uint8_t  Model[IDE_MODEL_STRING_LENGTH + 1];   // Model string
} ide_devices[4];



// TODO: Support multiple IDE controllers
extern uint8_t IDE_bus;
extern uint8_t IDE_slot;
extern uint8_t IDE_function;
extern bool IDE_Present;


void IDE_Init(uint8_t bus, uint8_t slot, uint8_t function);

uint8_t IDE_PollUntilNotBSY(uint8_t channel, uint8_t advanced_check);

uint8_t IDE_ReadRegister(uint8_t channel, uint8_t reg);

void IDE_ReadBuffer(uint8_t channel, uint8_t reg, uint32_t *buffer, unsigned int quads);

void IDE_WriteRegister(uint8_t channel, uint8_t reg, uint8_t data);

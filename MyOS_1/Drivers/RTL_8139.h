#pragma once
#include <stdint.h>
#include "../Networking/Ethernet.h"

// RTL 8139 registers:
#define RTL_MAC0_5      0x00

#define TX_0_STATUS     0x10
#define TX_0            0x20

#define TX_1_STATUS     0x14
#define TX_1            0x24

#define TX_2_STATUS     0x18
#define TX_2            0x28

#define TX_3_STATUS     0x1C
#define TX_3            0x2C

#define RTL_MAR0_7      0x08
#define RBSTART         0x30
#define RTL_CMD         0x37
#define RTL_RX_BUF_PTR  0x38
#define RTL_IMR         0x3C
#define RTL_ISR         0x3E
#define RTL_RCR         0x44
#define RTL_CONFIG_1    0x52

// Interrupt Status Bits for RTL_ISR
#define RTL_ISR_RX_OK       0x0001
#define RTL_ISR_RX_ERR      0x0002
#define RTL_ISR_TX_OK       0x0004
#define RTL_ISR_TX_ERR      0x0008
/*#define RxOverflow = 0x0010,
#define RxUnderrun = 0x0020,
#define RxFIFOOver = 0x0040,
#define PCSTimeout = 0x4000,
#define PCIErr = 0x8000,*/

#define RTL_POWER_ON    0

// Command bits
#define RTL_CMD_RX_BUFF_EMPTY   0x01
#define RTL_CMD_TX_ENABLE       0x04
#define RTL_CMD_RX_ENABLE       0x08
#define RTL_CMD_RESET           0x10

#define OWN_BIT         0x2000

#define RTL_RECEIVE_BUFFER_SIZE (8192 + 16 + 1500)
/* Size of the in-memory receive ring. */
#define RX_BUF_LEN_IDX	0			/* 0==8K, 1==16K, 2==32K, 3==64K */
#define RX_BUF_LEN (8192 << RX_BUF_LEN_IDX)

void RTL_8139_Init(uint8_t bus, uint8_t slot, uint8_t function);

void rtl_8139_InterruptHandler();

void RTL_8139_ReceivePacket();

void RTL_8139_SendPacket(Ethernet_Header *packet, uint16_t dataSize);

bool rtl_8139_SharedInterruptHandler();
#pragma once
#include <stdint.h>
#include "Ethernet.h"

#pragma pack(push, 1)
typedef struct ARP_Header
{
    uint16_t hardwareType;
    uint16_t protocolType;
    uint8_t  hardwareSize;
    uint8_t  protocolSize;
    uint16_t opcode;
    uint8_t  data[1];
} ARP_Header;

typedef struct ARP_IPv4
{
    uint8_t sourceMAC[6];
    uint32_t sourceIP;
    uint8_t destinationMAC[6];
    uint32_t destinationIP;
} ARP_IPv4;
#pragma pack(pop, 1)

#define ARP_REQUEST     1
#define ARP_REPLY       2
#define RARP_REQUEST    3
#define RARP_REPLY      4

#define ARP_ETHERNET_HW_TYPE    0x0100 /* 1, but byte-swapped 1 */
#define ARP_IP_PROTOCOL_TYPE    0x0008 /* 0x800, but byte-swapped */

#define ARP_OPCODE_REQUEST  0x0100 /* 1, but byte-swapped */
#define ARP_OPCODE_REPLY    2

#define ARP_HEADER_SIZE 8

Ethernet_Header *ARP_CreatePacket(uint8_t *sourceMAC, uint16_t *pPacketSize, void **data);

bool ARP_GetMAC(uint32_t ip, char *destMAC);

void ARP_RegisterIP(uint32_t ip, char *macAddress);

void ARP_Send_Request(uint32_t targetIP, uint8_t *sourceMAC);
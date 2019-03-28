#pragma once

#include <stdint.h>

#pragma pack(push, 1)
typedef struct UDP_Header
{
    uint16_t sourcePort;
    uint16_t destinationPort;
    uint16_t length;
    uint16_t checksum;
    uint8_t data[1];
} UDP_Header;
#pragma pack(pop, 1)

#define UDP_HEADER_SIZE 8

Ethernet_Header *UDP_Create_Packet(uint32_t targetIP, uint16_t sourcePort, uint16_t destPort, uint16_t *pPacketSize, uint8_t *sourceMAC, void **dataPtr);

void UDP_ProcessReceivedPacket(UDP_Header *packet, uint8_t *sourceMAC);

void UDP_Send_Packet(Ethernet_Header *packet, uint8_t *dataStart, uint16_t dataSize);

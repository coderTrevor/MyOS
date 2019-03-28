#pragma once
#include <stdint.h>
#include <stdbool.h>

// Some of this is taken from http://www.saminiir.com/lets-code-tcp-ip-stack-1-ethernet-arp/

#define ETHERNET_ADDRESS_LENGTH 6
#define IP_ADDRESS_LENGTH       4

#define ETHERTYPE_ARP   0x0608  /* ARP ethertype 0x0806 but byte-swapped */
#define ETHERTYPE_IPv4  0x0008  /* IPv4 ethertype 0x0800 but byte-swapped */

extern bool NIC_Present;

extern uint8_t NIC_MAC[6];

#pragma pack(push, 1)
typedef struct Ethernet_Header
{
    uint8_t  destinationMAC[6];
    uint8_t  sourceMAC[6];
    uint16_t etherType;
    uint8_t  data[1];
} Ethernet_Header;
#pragma pack(pop, 1)

#define ETHERNET_HEADER_SIZE    14

uint16_t SwapBytes16(uint16_t data);

uint32_t SwapBytes32(uint32_t data);

Ethernet_Header *EthernetCreatePacket(uint16_t etherType, uint16_t *pPacketSize, uint8_t *sourceMAC, void **dataPtr);

void EthernetSendPacket(Ethernet_Header *packet, uint16_t dataSize);

void EthernetPrintMAC(uint8_t *macAddress);

void EthernetProcessReceivedPacket(Ethernet_Header *packet, uint8_t *sourceMAC);

void (*SendCallback)(Ethernet_Header *packet, uint16_t dataSize); // callback function for sending a packet

void EthernetRegisterNIC_MAC(uint8_t *MAC);

void EthernetRegisterNIC_SendFunction(void(*RTL_8139_SendPacket)(Ethernet_Header *packet, uint16_t dataSize));
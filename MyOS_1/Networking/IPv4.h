#pragma once

#include <stdint.h>
#include "Ethernet.h"

#define IP_VERSION_4    4
#define IPv4_MINIMUM_HEADER_LENGTH  20
#define IPv4_TIME_TO_LIVE   64
#define IPv4_PROTO_UDP      0x11

#pragma pack(push, 1)
typedef struct ARP_IPv4_Packet
{
    uint8_t  sourceMAC[6];
    uint32_t sourceIP;
    uint8_t  destinationMAC[6];
    uint32_t destinationIP;
} ARP_IPv4_Packet;
#pragma pack(pop, 1)

#pragma pack(push, 1)
// I could use bitfields here but I don't trust them (I ended up using them in the end, against my better judgement, and they DID cause some problems
// but I ended up keeping them [for now])
// MSVC will complain about bit-fields with uint8_t, but using int screws up the packing
#pragma warning(disable:4214)
typedef struct IPv4_Header
{
    uint8_t internetHeaderLength : 4; // header-length is number of 32-bit words, usually this will be five
    uint8_t version : 4;
    uint8_t typeOfService;
    uint16_t totalLength; // header and data
    uint16_t identification;
    //uint16_t flags : 3;
    //uint16_t fragmentOffset : 13;
    uint16_t fragmentOffsetAndFlags;
    uint8_t timeToLive;
    uint8_t protocol;
    uint16_t headerChecksum;
    uint32_t sourceIP;
    uint32_t destinationIP;
    uint8_t data[1];
} IPv4_Header;
#pragma warning(default:4214)
#pragma pack(pop, 1)

extern uint32_t clientIP;  // our IP address
extern uint32_t gatewayIP; // IP address of the router
extern uint32_t DNS_Servers[4];
extern uint32_t subnetMask;    // honestly, I've never known what this is even for. Maybe I'll find out soon.

Ethernet_Header *IPv4_CreatePacket(uint32_t targetIP, uint16_t *pPacketSize, uint8_t *sourceMAC, void **dataPtr);

uint32_t IPv4_PackIP(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth);

void IPv4_PrintIP(uint32_t ip);

void IPv4_ProcessReceivedPacket(IPv4_Header *ipHeader, uint8_t *sourceMAC);

void IPv4_SetGateway(uint32_t router);

void IPv4_SetClientIP(uint32_t client);

// TODO: Support multiple servers
void IPv4_SetDNS_Servers(uint32_t server1);

void IPv4_SetSubnetMask(uint32_t subnet);

void IPv4_UpdateHeaderChecksum(IPv4_Header *header);
#pragma once
#include "Ethernet.h"
#include <stdint.h>

#define DHCP_OPCODE_DISCOVER    1
#define DHCP_OPCODE_OFFER       2
#define DHCP_OPCODE_REQUEST     1
#define DHCP_OPCODE_ACKNOWLEDGE 2
#define DHCP_HARDWARE_ETHERNET  1
#define DHCP_ETHERNET_LENGTH    6

#define DHCP_MAGIC_COOKIE       0x63825363 
#define DHCP_OPTION_MESSAGE_TYPE    0x35
#define DHCP_MSG_TYPE_DISCOVER      0x01
#define DHCP_MSG_TYPE_OFFER         0x02
#define DHCP_MSG_TYPE_REQUEST       0x03
#define DHCP_MSG_TYPE_ACK           0x05
#define DHCP_OPTION_SUBNET_MASK     1
#define DHCP_OPTION_ROUTER          3
#define DHCP_OPTION_DNS_SERVER      6
#define DHCP_OPTION_REQUEST_IP      50
#define DHCP_OPTION_IP_LEASE_TIME   51
#define DHCP_OPTION_SERVER_IP       54
#define DHCP_OPTION_END             0xFF

#define DHCP_DISCOVER_LENGTH        (sizeof(DHCP_HEADER) - 72)

#pragma pack(push, 1)
typedef struct DHCP_HEADER
{
    uint8_t opcode;
    uint8_t hardwareType;
    uint8_t hardwareAddressLength;
    uint8_t hops;
    uint32_t transactionID;
    uint16_t seconds;
    uint16_t flags;
    uint32_t clientIP;
    uint32_t yourIP; // client IP
    uint32_t serverIP;
    uint32_t relayIP; // sometimes referred to as gateway address
    uint8_t clientHardwareAddress[16]; // MAC address for ethernet
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[76];
} DHCP_HEADER;
#pragma pack(pop, 1)

void DHCP_ProcessReply(DHCP_HEADER *reply, uint8_t *sourceMAC);

void DHCP_Send_Discovery(uint8_t *sourceMAC);

//void DHCP_Send_Request(uint8_t *sourceMAC);
#include "ARP.h"
#include "../Terminal.h"
#include "../misc.h"

// ARP table (Full table TODO)
// for now, we'll only be communicating with one other system
uint32_t serverIP = 0;
char serverMAC[6];

Ethernet_Header *ARP_CreatePacket(uint8_t *sourceMAC, uint16_t *pPacketSize, void **data)
{
    *pPacketSize += ARP_HEADER_SIZE;
    ARP_Header *arpHeader;
    Ethernet_Header *packet = EthernetCreatePacket(ETHERTYPE_ARP, pPacketSize, sourceMAC, &arpHeader);
    
    arpHeader->hardwareType = ARP_ETHERNET_HW_TYPE;
    arpHeader->protocolType = ARP_IP_PROTOCOL_TYPE;
    arpHeader->hardwareSize = ETHERNET_ADDRESS_LENGTH;
    arpHeader->protocolSize = IP_ADDRESS_LENGTH;
    arpHeader->opcode = ARP_OPCODE_REQUEST;

    *data = arpHeader->data;

    return packet;
}

// return true if we can find a match for the given IP
bool ARP_GetMAC(uint32_t ip, char *destMAC)
{
    if (ip != serverIP)
        return false;

    memcpy(destMAC, serverMAC, 6);
    return true;
}

void ARP_ProcessPacket(ARP_Header *packet)
{
    if (packet->opcode == ARP_OPCODE_REQUEST)
    {
        terminal_writestring("ARP_REQUEST received\n");
    }
    else if (packet->opcode == ARP_OPCODE_REPLY)
    {
        terminal_writestring("ARP_REPLY received\n");
    }
    else
    {
        terminal_writestring("Unknown ARP opcode: ");
        terminal_print_int(packet->opcode);
        terminal_newline();
    }
}

void ARP_RegisterIP(uint32_t ip, char *macAddress)
{
    if (serverIP != 0)
    {
        terminal_writestring("ARP - Warning - table not implemented; overwriting previous ARP entry\n");
    }

    serverIP = ip;
    memcpy(serverMAC, macAddress, 6);
}

void ARP_SendRequest(uint32_t targetIP, uint8_t *sourceMAC)
{
    uint16_t packetSize = sizeof(ARP_IPv4);
    ARP_IPv4 *ipv4_packet;

    Ethernet_Header *packet = ARP_CreatePacket(sourceMAC, &packetSize, &ipv4_packet);

    memcpy(ipv4_packet->sourceMAC, sourceMAC, 6);
    ipv4_packet->sourceIP = 0;
    memset(&ipv4_packet->destinationMAC, 0, 4);
    ipv4_packet->destinationIP = targetIP;
    
    EthernetSendPacket(packet, packetSize);
}
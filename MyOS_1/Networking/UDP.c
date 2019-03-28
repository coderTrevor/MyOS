#include "Ethernet.h"
#include "IPv4.h"
#include "UDP.h"
#include "../Console_VGA.h"
#include "../misc.h"
#include "TFTP.h"
#include "DHCP.h"

Ethernet_Header *UDP_Create_Packet(uint32_t targetIP, uint16_t sourcePort, uint16_t destPort, uint16_t *pPacketSize, uint8_t *sourceMAC, void **dataPtr)
{
    uint16_t originalPacketSize = *pPacketSize;

    // increase packet size by 8 bytes for the UDP header
    *pPacketSize += UDP_HEADER_SIZE;
    UDP_Header *udpHeader;
    Ethernet_Header *header = IPv4_CreatePacket(targetIP, pPacketSize, sourceMAC, &udpHeader);

    udpHeader->sourcePort = SwapBytes16(sourcePort);
    udpHeader->destinationPort = SwapBytes16(destPort);
    udpHeader->length = SwapBytes16(originalPacketSize + UDP_HEADER_SIZE);
    udpHeader->checksum = 0;

    *dataPtr = udpHeader->data;

    return header;
}

// This may be useful but for now I send packets straight to the ethernet layer
// dataStart is the start of the data encapsulated within the UDP packet
/*void UDP_Send_Packet(Ethernet_Header *packet, uint8_t *dataStart, uint16_t dataSize)
{
    UDP_Header *udpHeader = (UDP_Header *)((unsigned long)dataStart - UDP_HEADER_SIZE);

    // TODO: Update udpHeader->checksum

    // it seems like we can skip the IP layer and go straight to ethernet
}*/

void UDP_ProcessReceivedPacket(UDP_Header *packet, uint8_t *sourceMAC)
{
    uint16_t size = SwapBytes16(packet->length);
    uint16_t sourcePort = SwapBytes16(packet->sourcePort);
    uint16_t destinationPort = SwapBytes16(packet->destinationPort);
    
    if (debugLevel)
    {
        terminal_writestring("UDP packet with ");
        terminal_print_int(size);
        terminal_writestring(" bytes received.\n");
    }

    if (destinationPort == 68)
    {
        //terminal_writestring("DHCP Packet received\n");
        DHCP_ProcessReply((DHCP_HEADER *)packet->data, sourceMAC);
    }

    if (sourcePort == TFTP_PORT)
    {
        TFTP_ProcessPacket((TFTP_Header *)packet->data, sourcePort, destinationPort, size - UDP_HEADER_SIZE, sourceMAC);
    }
}
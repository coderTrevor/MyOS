#include "Ethernet.h"
#include "../misc.h"
#include "../Console_VGA.h"
#include "IPv4.h"
#include "ARP.h"

// TEMPTEMP - until we create a memory allocator
uint8_t packetBuffer[1024];

// TEMPTEMP - we should really have some kind of array for multiple NIC's
bool NIC_Present = false;
uint8_t NIC_MAC[6];

// swap the bytes of a 16-bit value and return the result
uint16_t SwapBytes16(uint16_t data)
{
    uint16_t returnVal;
    uint8_t *dst = (uint8_t *)&returnVal;
    uint8_t *src = (uint8_t *)&data;
    dst[0] = src[1];
    dst[1] = src[0];

    return returnVal;
}

// swap the bytes of a 32-bit value and return the result
uint32_t SwapBytes32(uint32_t data)
{
    uint32_t returnVal;
    uint8_t *dst = (uint8_t *)&returnVal;
    uint8_t *src = (uint8_t *)&data;
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];

    return returnVal;
}

Ethernet_Header *EthernetCreatePacket(uint16_t etherType, uint16_t *pPacketSize, uint8_t *sourceMAC, void **dataPtr)
{
    // add size of ethernet header to packet size
    *pPacketSize += ETHERNET_HEADER_SIZE;

    Ethernet_Header *packet = (Ethernet_Header *)packetBuffer;

    // TODO: Translate MAC from target IP
    packet->destinationMAC[0] = 0xFF;
    packet->destinationMAC[1] = 0xFF;
    packet->destinationMAC[2] = 0xFF;
    packet->destinationMAC[3] = 0xFF;
    packet->destinationMAC[4] = 0xFF;
    packet->destinationMAC[5] = 0xFF;

    // copy source MAC
    memcpy(packet->sourceMAC, sourceMAC, 6);

    packet->etherType = etherType;

    *dataPtr = packet->data;
    
    return packet;
}

void EthernetSendPacket(Ethernet_Header *packet, uint16_t dataSize)
{    
    if (!NIC_Present)
    {
        terminal_writestring("ERROR: Can't send packet without a network card present!\n");
        return;
    }

    // TODO: Support multiple NIC's and send this packet to the NIC owning the MAC in the packet
    SendCallback(packet, dataSize);
}

void EthernetPrintMAC(uint8_t *macAddress)
{
    for (int i = 0; i < 5; ++i)
    {
        terminal_print_byte_hex(macAddress[i]);
        terminal_putchar(':');
    }
    terminal_print_byte_hex(macAddress[5]);
}

// we need to be passed the MAC, because the packet may have a broadcast address for destination
void EthernetProcessReceivedPacket(Ethernet_Header *packet, uint8_t *ourMAC)
{
    /*for (int i = 0; i < 16; ++i)
    {
        terminal_print_byte_hex(((uint8_t*)packet)[i]);
        terminal_putchar(' ');
    }*/
    /*terminal_writestring("Packet received for ");
    EthernetPrintMAC(packet->destinationMAC);
    terminal_newline();*/
    if (packet->etherType == ETHERTYPE_IPv4)
    {
        //terminal_writestring("    IPv4 packet received.\n");
        IPv4_Header *ipHeader = (IPv4_Header *)(packet->data);
        IPv4_ProcessReceivedPacket(ipHeader, ourMAC);
    }
    else if (packet->etherType == ETHERTYPE_ARP)
    {
        terminal_writestring("    ARP packet received.\n");
        //ARP_Header *arpHeader = (ARP_Header *)(packet->data);
        // TODO: Process ARP received
    }
}

// The NIC driver should call this function to register its send function
// TODO: Support more than one NIC
void EthernetRegisterNIC_SendFunction(void(*SendPacket)(Ethernet_Header *packet, uint16_t dataSize))
{
    SendCallback = SendPacket;
    NIC_Present = true;
}

// TODO: Support more than one NIC
void EthernetRegisterNIC_MAC(uint8_t *MAC)
{
    memcpy(NIC_MAC, MAC, 6);
}
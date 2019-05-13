// Driver for the Realtek 8139 NIC
// This driver is hacky crap. It will only support one NIC, and probably not very well
// Reference: https://github.com/szhou42/osdev/blob/master/src/kernel/drivers/rtl8139.c
// Reference: http://www.jbox.dk/sanos/source/sys/dev/rtl8139.c.html
// some code copied or adapted from Linux' driver written by Donald Becker
// some code copied or adapted from sanos' driver written by Michael Ringgaard
#include "PCI_Bus.h"
#include "../Terminal.h"
#include "../Networking/DHCP.h"
#include "../System_Specific.h"
#include "RTL_8139.h"
#include "../Interrupts/PIC.h"
#include "../Interrupts/IDT.h"
#include "../Interrupts/Interrupts.h"
#include "../misc.h"
#include "../Networking/ARP.h"
#include "../Networking/Ethernet.h"
#include "../multiboot.h"

uint16_t rtl8139_base_port;
uint8_t  rtl8139_IRQ;

// TEMPTEMP - This should be an allocated buffer once the memory allocator is working
uint8_t rxBuffer[RTL_RECEIVE_BUFFER_SIZE];
// TEMPTEMP - there should be one MAC for each card that the system may have
uint8_t mac_addr[6];
//uint8_t testPacket[8092];

// from the Linux driver:
uint16_t cur_rx;		/* Index into the Rx buffer of next Rx pkt. */
unsigned char *rx_ring = rxBuffer;

// which of the 4 transmit buffers to use for the next transmission (rtl 8139 uses a round-robin style)
uint8_t nextTX;

void RTL_8139_Init(uint8_t bus, uint8_t slot, uint8_t function)
{
    terminal_writestring("    Initializing RTL 8139 driver...\n     Base address: ");

    nextTX = 0;
    cur_rx = 0;
    rx_ring = rxBuffer;

    // get the I/O port
    rtl8139_base_port = PCI_GetBaseAddress0(bus, slot, function) & ~3;
    
    terminal_print_ushort_hex(rtl8139_base_port);

    // get the IRQ
    rtl8139_IRQ = PCI_GetInterruptLine(bus, slot, function);
    terminal_writestring(" - IRQ ");
    terminal_print_int(rtl8139_IRQ);

    // make sure an IRQ line is being used
    if (rtl8139_IRQ == 0xFF)
    {
        terminal_writestring("\n      Can't use I/O APIC interrupts. Aborting.\n");
        return;
    }

    // Setup an interrupt handler for this device
    // TODO: Support IRQ sharing
    Set_IDT_Entry((unsigned long)rtl_8139_InterruptHandler, HARDWARE_INTERRUPTS_BASE + rtl8139_IRQ);

    // Tell the PIC to enable the NIC's IRQ
    IRQ_Enable_Line(rtl8139_IRQ);

    // enable PCI bus mastering
    PCI_EnableBusMastering(bus, slot, function);

    // set LWAKE + LWPTN to active high, powering on the rtl 8139
    outb(rtl8139_base_port + RTL_CONFIG_1, RTL_POWER_ON);

    // perform a software reset
    outb(rtl8139_base_port + RTL_CMD, RTL_CMD_RESET);

    // wait until the reset has complete (TODO: time out if there's a problem)
    while ((inb(rtl8139_base_port + RTL_CMD) & RTL_CMD_RESET) != 0)
    {
        // wait for reset to complete
    }

    // setup the receive buffer

    // test rxBuffer access
    // translate buffer to physical address (TODO: Improve in many ways)
    // This only works because we've identity mapped the first four megs of RAM
    uint8_t *rxBufferPtr = (uint8_t*)((uint32_t)rxBuffer - BASE_ADDRESS + LOADBASE);// 0xC0000000 + 0x100000);
    //terminal_print_ulong_hex(rxBufferPtr);
    //terminal_newline();

    // TEMPTEMP - Make sure we can read and write to buffer
    for (int i = 0; i < RTL_RECEIVE_BUFFER_SIZE; ++i)
        rxBufferPtr[i] = (i & 0x5A);

    for (int i = 0; i < RTL_RECEIVE_BUFFER_SIZE; ++i)
    {
        if (rxBufferPtr[i] != (i & 0x5A))
            terminal_writestring("RTL 8139 driver can't write to receive buffer!\n");
    }
    
    // Init the receive buffer for real now
    //terminal_print_ushort_hex(rtl8139_base_port + RBSTART);
    outl(rtl8139_base_port + RBSTART, (uintptr_t)rxBufferPtr);

    // Set IMR to enable Transmit OK and Receive OK interrupts
    outw(rtl8139_base_port + RTL_IMR, 0x0005); // Sets the TOK and ROK bits high
    
    // configure receive buffer, RCR
    // we want to accept broadcast, accept multicast, accept physical match, and accept all packets
    // we also want to set the wrap bit to one, so we don't have to worry about the receive buffer wrapping around
    outl(rtl8139_base_port + RTL_RCR, 0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

    // set the RE and TE bits high
    outb(rtl8139_base_port + RTL_CMD, 0x0C);

    // The OSDev forum has many anecdotes of actual hardware not working if the RCR Register is written before
    // the command register. This seems strange to me, so I write to the register before and after enabling Reception
    // to cover all the bases
    outl(rtl8139_base_port + RTL_RCR, 0xf | (1 << 7));

    // Read the MAC Address
    uint32_t mac_part1 = inl(rtl8139_base_port + 0x00);
    uint16_t mac_part2 = (uint16_t)(inl(rtl8139_base_port + 0x04));

    mac_addr[0] = (uint8_t)(mac_part1 >> 0);
    mac_addr[1] = (uint8_t)(mac_part1 >> 8);
    mac_addr[2] = (uint8_t)(mac_part1 >> 16);
    mac_addr[3] = (uint8_t)(mac_part1 >> 24);

    mac_addr[4] = (uint8_t)(mac_part2 >> 0);
    mac_addr[5] = (uint8_t)(mac_part2 >> 8);

    terminal_writestring(" - MAC: ");
    EthernetPrintMAC(mac_addr);

    terminal_newline();

    // Register this NIC with the ethernet subsystem
    EthernetRegisterNIC_SendFunction(RTL_8139_SendPacket);
    EthernetRegisterNIC_MAC(mac_addr);

    // Request an IP via DHCP
    terminal_writestring("     Requesting IP address via DHCP...\n");

    //ARP_Send_Request(IPv4_PackIP(10, 0, 2, 4), mac_addr);
    DHCP_Send_Discovery(mac_addr);

    terminal_writestring("    RTL 8139 driver initialized.\n");
}

uint32_t isr;
void _declspec(naked) rtl_8139_InterruptHandler()
{
    _asm pushad;

    /* do something */
    ++interrupts_fired;

    if(debugLevel)
        terminal_writestring(" --------- rtl 8139 interrupt fired! -------\n");

    // get the interrupt status
    isr = inw(rtl8139_base_port + RTL_ISR);

    if (isr & ~(RTL_ISR_RX_OK | RTL_ISR_TX_OK))
    {
        terminal_writestring("rtl8139 Error: Unhandled ISR (TODO) ");
        terminal_print_ulong_hex(isr);
        for(;;)
            __halt();
    }
    //terminal_print_ulong_hex(isr);

    outw(rtl8139_base_port + RTL_ISR, 0xFFFF);
    
    // check the status and see if it's a transmit success
    if (isr & RTL_ISR_TX_OK)
    {
        if(debugLevel)
            terminal_writestring("     Packet sent successfully!\n");

        // clear the interrupt
        //outw(rtl8139_base_port + RTL_ISR, RTL_TX_OK);
    }

    // check the status and see if it's a received success
    if (isr & RTL_ISR_RX_OK)
    {
        RTL_8139_ReceivePacket();
    }

    //outw(rtl8139_base_port + RTL_ISR, RTL_RX_OK);
    /*if (isr & RTL_TX_OK)

    // clear all interrupts
    outw(rtl8139_base_port + RTL_ISR, 0xFFFF);*/
    //isr = inw(rtl8139_base_port + RTL_ISR);
    //terminal_print_ulong_hex(isr);

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + rtl8139_IRQ);

    if(debugLevel)
        terminal_writestring(" --------- rtl 8139 interrupt done! -------\n");

    _asm
    {
        popad
        iretd
    }
}

void RTL_8139_ProcessRxPacket()
{
    if(debugLevel)
        terminal_writestring("     Packet received successfully!\n");

    //outw(rtl8139_base_port + RTL_ISR, RTL_RX_OK);

    // get offset of packet start within the ring buffer
    uint32_t ring_offset = cur_rx % RX_BUF_LEN;

    // get pointer to the beginning of the packet
    uint32_t *rxBegin = (uint32_t*)((uint32_t)(rxBuffer)+ring_offset);

    // get packet status (placed at beginning of the packet by NIC)
    uint32_t rx_status = *rxBegin;
    
    // get the packet length
    uint16_t rx_size = rx_status >> 16;

    // TODO: Check size, status, etc

    Ethernet_Header *packet = (Ethernet_Header *)((uint8_t*)rxBegin + 4);
    //terminal_print_ulong_hex(packet);
    //terminal_newline();
    EthernetProcessReceivedPacket(packet, mac_addr);

    // update ring buffer info
    // no clue what's going on here, just copied code:
    cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
    outpw(rtl8139_base_port + RTL_RX_BUF_PTR, cur_rx - 16);
    //cur_rx %= RX_BUF_LEN;
}

void RTL_8139_ReceivePacket()
{
    // keep processing packets until the receive buffer is empty
    while (!(inb(rtl8139_base_port + RTL_CMD) & RTL_CMD_RX_BUFF_EMPTY))
    {
        RTL_8139_ProcessRxPacket();
    }

    // translate buffer to physical address (TODO: Improve in many ways)
    // This only works because we've identity mapped the first four megs of RAM
    //uint8_t *rxBufferPtr = (uint8_t*)((uint32_t)rxBuffer - 0xC0000000 + 0x100000);
    //outl(rtl8139_base_port + RBSTART, (uintptr_t)rxBufferPtr);
    //outw(rtl8139_base_port + 0x38, 0);

    // clear the interrupt
    //outw(rtl8139_base_port + RTL_ISR, RTL_RX_OK);
}

void RTL_8139_SendPacket(Ethernet_Header *packet, uint16_t dataSize)
{
    // TEMPTEMP HACKHACK - find a physical address for our temporary tx buffer
    uint32_t physicalBufferAddress = (uint32_t)packet - BASE_ADDRESS + LOADBASE;// 0xC0000000 + 0x100000;

    if (debugLevel)
    {
        terminal_writestring("SendPacket called, nextTX is ");
        terminal_print_int(nextTX);
        terminal_newline();
    }

    // determine which TX / STATUS register pair we'll be using
    uint8_t txReg;
    uint8_t statusReg;
    switch (nextTX)
    {
        case 0:
            txReg = TX_0;
            statusReg = TX_0_STATUS;
            break;
        case 1:
            txReg = TX_1;
            statusReg = TX_1_STATUS;
            break;
        case 2:
            txReg = TX_2;
            statusReg = TX_2_STATUS;
            break;
        default:
            txReg = TX_3;
            statusReg = TX_3_STATUS;
            break;
    }

    // advance nextTX
    nextTX = (nextTX + 1) % 4;

    uint32_t command = inl(rtl8139_base_port + statusReg);

    // copy the buffer's physical address to the TX reg
    outl(rtl8139_base_port + txReg, physicalBufferAddress);

    // set data size, which goes in the lowest 13 bits of command
    command = (command & ~(0x1FFF)) | dataSize;

    // clear the own bit of the command register to start the transfer
    command &= ~(OWN_BIT);

    // write out the command to the command / status register
    outl(rtl8139_base_port + statusReg, command);

}

// Here's Linux's receive function:
/* The data sheet doesn't describe the Rx ring at all, so I'm guessing at the
field alignments and semantics. */
// ^No shit! Why is this card considered easy to program? Does everyone just copy the Linux driver?
/*static int rtl8129_rx(struct device *dev)
{
    struct rtl8129_private *tp = (struct rtl8129_private *)dev->priv;
    long ioaddr = dev->base_addr;
    unsigned char *rx_ring = tp->rx_ring;
    u16 cur_rx = tp->cur_rx;

    if (rtl8129_debug > 4)
        printk(KERN_DEBUG"%s: In rtl8129_rx(), current %4.4x BufAddr %4.4x,"
               " free to %4.4x, Cmd %2.2x.\n",
               dev->name, cur_rx, inw(ioaddr + RxBufAddr),
               inw(ioaddr + RxBufPtr), inb(ioaddr + ChipCmd));

    while ((inb(ioaddr + ChipCmd) & 1) == 0) {
        int ring_offset = cur_rx % RX_BUF_LEN;
        u32 rx_status = *(u32*)(rx_ring + ring_offset);
        int rx_size = rx_status >> 16;

        if (rtl8129_debug > 4) {
            int i;
            printk(KERN_DEBUG"%s:  rtl8129_rx() status %4.4x, size %4.4x, cur %4.4x.\n",
                   dev->name, rx_status, rx_size, cur_rx);
            printk(KERN_DEBUG"%s: Frame contents ", dev->name);
            for (i = 0; i < 70; i++)
                printk(" %2.2x", rx_ring[ring_offset + i]);
            printk(".\n");
        }
        if (rx_status & RxTooLong) {
            if (rtl8129_debug > 0)
                printk(KERN_NOTICE"%s: Oversized Ethernet frame, status %4.4x!\n",
                       dev->name, rx_status);
            tp->stats.rx_length_errors++;
        }
        else if (rx_status &
            (RxBadSymbol | RxRunt | RxTooLong | RxCRCErr | RxBadAlign)) {
            if (rtl8129_debug > 1)
                printk(KERN_DEBUG"%s: Ethernet frame had errors,"
                       " status %4.4x.\n", dev->name, rx_status);
            tp->stats.rx_errors++;
            if (rx_status & (RxBadSymbol | RxBadAlign))
                tp->stats.rx_frame_errors++;
            if (rx_status & (RxRunt | RxTooLong)) tp->stats.rx_length_errors++;
            if (rx_status & RxCRCErr) tp->stats.rx_crc_errors++;
            // Reset the receiver, based on RealTek recommendation. (Bug?) 
            tp->cur_rx = 0;
            outb(CmdTxEnb, ioaddr + ChipCmd);
            outb(CmdRxEnb | CmdTxEnb, ioaddr + ChipCmd);
            outl((RX_FIFO_THRESH << 13) | (RX_BUF_LEN_IDX << 11) |
                (RX_DMA_BURST << 8), ioaddr + RxConfig);
        }
        else {
            // Malloc up new buffer, compatible with net-2e.
            // Omit the four octet CRC from the length. 
            struct sk_buff *skb;

            skb = dev_alloc_skb(rx_size + 2);
            if (skb == NULL) {
                printk(KERN_WARNING"%s: Memory squeeze, deferring packet.\n",
                       dev->name);
                // We should check that some rx space is free.
                //If not, free one and mark stats->rx_dropped++. 
                tp->stats.rx_dropped++;
                break;
            }
            skb->dev = dev;
            skb_reserve(skb, 2);	// 16 byte align the IP fields.
            if (ring_offset + rx_size + 4 > RX_BUF_LEN) {
                int semi_count = RX_BUF_LEN - ring_offset - 4;
                memcpy(skb_put(skb, semi_count), &rx_ring[ring_offset + 4],
                       semi_count);
                memcpy(skb_put(skb, rx_size - semi_count), rx_ring,
                       rx_size - semi_count);
                if (rtl8129_debug > 4) {
                    int i;
                    printk(KERN_DEBUG"%s:  Frame wrap @%d",
                           dev->name, semi_count);
                    for (i = 0; i < 16; i++)
                        printk(" %2.2x", rx_ring[i]);
                    printk(".\n");
                    memset(rx_ring, 0xcc, 16);
                }
            }
            
            skb->protocol = eth_type_trans(skb, dev);
            netif_rx(skb);
#if LINUX_VERSION_CODE > 0x20119
            tp->stats.rx_bytes += rx_size;
#endif
            tp->stats.rx_packets++;
        }

        cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
        outw(cur_rx - 16, ioaddr + RxBufPtr);
    }
    if (rtl8129_debug > 4)
        printk(KERN_DEBUG"%s: Done rtl8129_rx(), current %4.4x BufAddr %4.4x,"
               " free to %4.4x, Cmd %2.2x.\n",
               dev->name, cur_rx, inw(ioaddr + RxBufAddr),
               inw(ioaddr + RxBufPtr), inb(ioaddr + ChipCmd));
    tp->cur_rx = cur_rx;
    return 0;
}*/
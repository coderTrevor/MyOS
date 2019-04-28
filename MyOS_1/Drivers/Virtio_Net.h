#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../Networking/Ethernet.h"

// Registers (generic virtio) - note: this is considered the "legacy interface" and is defined in 4.1.4.8 of the virtio spec
#define REG_DEVICE_FEATURES 0x00
#define REG_GUEST_FEATURES  0x04
#define REG_QUEUE_ADDRESS   0x08
#define REG_QUEUE_SIZE      0x0C
#define REG_QUEUE_SELECT    0x0E
#define REG_QUEUE_NOTIFY    0x10
#define REG_DEVICE_STATUS   0x12
#define REG_ISR_STATUS      0x13
// Network-device-specific registers:
#define	REG_MAC_1           0x14
#define	REG_MAC_2           0x15
#define	REG_MAC_3           0x16
#define	REG_MAC_4           0x17
#define	REG_MAC_5           0x18
#define	REG_MAC_6           0x19
#define REG_NIC_STATUS      0x1A

// REG_DEVICE_STATUS flags
#define STATUS_RESET_DEVICE         0       /* Can't be combined with other flags; reset is accomplished by writing 0 to the status register. */
#define	STATUS_DEVICE_ACKNOWLEDGED  0x01
#define	STATUS_DRIVER_LOADED        0x02
#define	STATUS_DRIVER_READY         0x04
#define STATUS_FEATURES_OK          0x08
#define	STATUS_DEVICE_ERROR         0x40
#define	STATUS_DRIVER_FAILED        0x80

// Feature bits (See 5.1.3 of virtio-v1.0-cs04.pdf)
#define VIRTIO_NET_F_CSUM                   0
#define VIRTIO_NET_F_GUEST_CSUM             1
#define VIRTIO_NET_F_CTRL_GUEST_OFFLOADS    2
#define VIRTIO_NET_F_MAC                    0x20
#define VIRTIO_NET_F_GUEST_TSO4             0x80
#define VIRTIO_NET_F_GUEST_TSO6             0x100
#define VIRTIO_NET_F_GUEST_ECN              0x200
#define VIRTIO_NET_F_GUEST_UFO              0x400
#define VIRTIO_NET_F_HOST_TSO4              0x800
#define VIRTIO_NET_F_HOST_TSO6              0x1000
#define VIRTIO_NET_F_HOST_ECN               0x2000
#define VIRTIO_NET_F_HOST_UFO               0x4000
#define VIRTIO_NET_F_MRG_RXBUF              0x8000
#define VIRTIO_NET_F_STATUS                 0x10000
#define VIRTIO_NET_F_CTRL_VQ                0x20000
#define VIRTIO_NET_F_CTRL_RX                0x40000
#define VIRTIO_NET_F_CTRL_VLAN              0x80000
#define VIRTIO_NET_F_GUEST_ANNOUNCE         0x200000
#define VIRTIO_NET_F_MQ                     0x400000
#define VIRTIO_NET_F_CTRL_MAC_ADDR          0x800000

// These are the features required by this driver
#define REQUIRED_FEATURES (/*VIRTIO_NET_F_CSUM |*/ VIRTIO_NET_F_MAC)

// ISR bits
#define VIRTIO_ISR_VIRTQ_USED               1
#define VIRTIO_ISR_CONFIG_CHANGED           2

// Flags for virtqueue descriptors
/* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT                   1
/* This marks a buffer as device write-only (otherwise device read-only). */
#define VIRTQ_DESC_F_DEVICE_WRITE_ONLY      2
#define VIRTQ_DESC_F_DEVICE_READ_ONLY       0
/* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT               4

typedef struct virtq_descriptor
{
    /* Address (guest-physical). */
    uint64_t address;
    uint32_t length;
    
    /* The flags as indicated above. */
    uint16_t flags;
    /* Next field if flags & NEXT */
    uint16_t next;
} virtq_descriptor;


// Flag for virtq_driver_area:
#define VIRTQ_AVAIL_F_NO_INTERRUPT          1

// The driver area AKA Available Ring is only written by the driver and read by the device
typedef struct virtq_driver_area
{
    uint16_t flags;
    // index indicates where the driver will put the next entry in the ring (wrapping to the beginning of the ring as needed). It's value only ever increases.
    uint16_t index;
    // ringBuffer is a ring-buffer that is queue-size elements long. Each entry refers to the head of a descriptor chain.
    uint16_t ringBuffer[1];
    // ringBuffer will be allocated in-place and take up (2 * (queue size)) bytes

    // uint16_t used_event would be present after ringBuffer, but only if VIRTIO_F_EVENT_IDX is negotiated. (we don't use it here)
} virtq_driver_area, virtq_available_ring;



typedef struct virtq_device_element
{
    // index is the index into the descriptor table of used descriptor chain
    uint32_t index;
    // length is the total length of bytes written to the descriptor chain
    uint32_t length;
} virtq_device_element, virtq_used_elem;

// Flag for virtq_device_area
#define VIRTQ_USED_F_NO_NOTIFY              1

// The device area AKA Used Ring is only written by the device and read by the driver
typedef struct virtq_device_area
{
    uint16_t flags;
    uint16_t index;
    virtq_device_element ringBuffer[1];
    // ringBuffer will be allocated in-place and take up (4 * (queue size)) bytes. Each element describes a used descriptor, including the index and length.

    // uint16_t avail_event will be present after ringBuffer. It's only used if VIRTIO_F_EVENT_IDX is negotiated. (we don't use it here)
} virtq_device_area, virtq_used_ring;

typedef struct virtq
{
    uint16_t elements;              // number of queue elements AKA queue size
    virtq_descriptor *descriptors;  // will be an elements-long array of descriptors
    virtq_driver_area *driverArea;  // extra data supplied by the driver to the device. AKA Available Ring
    virtq_device_area *deviceArea;  // extra data supplied by the device to the driver. AKA Used Ring
    uint16_t nextDescriptor;        // index of the next slot in the descriptor table we should use
    uint16_t lastDeviceAreaIndex;   // the last value of deviceArea->index we've seen. If this value doesn't equal deviceArea->index, 
                                    // the device has added used descriptors to deviceArea->ringBuffer.
    uint32_t byteSize; // for debug
} virtq;


// Flags for virtio_net_hdr
#define VIRTIO_NET_HDR_F_NEEDS_CSUM 1
#define VIRTIO_NET_HDR_F_DATA_VALID 2
#define VIRTIO_NET_HDR_F_RSC_INFO   4

// Values for virtio_net_hdr gso_type
#define VIRTIO_NET_HDR_GSO_NONE     0
#define VIRTIO_NET_HDR_GSO_TCPV4    1
#define VIRTIO_NET_HDR_GSO_UDP      3
#define VIRTIO_NET_HDR_GSO_TCPV6    4
#define VIRTIO_NET_HDR_GSO_ECN      0x80

// Every packet sent or received must be preceded by a virtio_net_hdr
typedef struct virtio_net_hdr
{
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    // uint16_t num_buffers;  num_buffer is not part of struct if VIRTIO_NET_F_MRG_RXBUF isn't negotiated
} virtio_net_hdr;



void VirtIO_Net_Init(uint8_t bus, uint8_t slot, uint8_t function);

void VirtIO_Net_InterruptHandler();

void VirtIO_Net_ScanRQ();

void VirtIO_Net_SendPacket(Ethernet_Header *packet, uint16_t dataSize);

void VirtIO_Net_SetupReceiveBuffers();

void VirtIO_Net_ReceivePacket();
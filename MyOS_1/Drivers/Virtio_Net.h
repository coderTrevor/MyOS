#pragma once

#include <stdint.h>
#include <stdbool.h>

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
#define REQUIRED_FEATURES (VIRTIO_NET_F_CSUM | VIRTIO_NET_F_MAC)

void VirtIO_Net_Init(uint8_t bus, uint8_t slot, uint8_t function);

void VirtIO_Net_InterruptHandler();
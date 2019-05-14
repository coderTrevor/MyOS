#pragma once

#include <stdint.h>


// Feature bits
#define VIRTIO_GPU_F_VIRGL  1
#define VIRTIO_GPU_F_EDID   2


void Virtio_GPU_Init(uint8_t bus, uint8_t slot, uint8_t function);
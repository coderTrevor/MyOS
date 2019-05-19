#pragma once

#include <stdint.h>

// Queue numbers
#define VIRTIO_GPU_CONTROL_Q_INDEX  0
#define VIRITO_GPU_CURSOR_Q_INDEX   1

// Feature bits
#define VIRTIO_GPU_F_VIRGL  1
#define VIRTIO_GPU_F_EDID   2

// Sadly, VIRTIO_GPU_F_VIRGL doesn't work on Windows (which sucks because that's the whole reason I started implementing virtio-gpu)
#define REQUIRED_FEATURES   (VIRTIO_F_VERSION_1)

typedef struct virtio_gpu_ctrl_hdr 
{
    uint32_t type;
    uint32_t flags;
    uint32_t fence_id;
    uint32_t ctx_id;
    uint32_t padding;
} virtio_gpu_ctrl_hdr;

typedef struct virtio_gpu_cursor_pos 
{
    uint32_t scanout_id;
    uint32_t x;
    uint32_t y;
    uint32_t padding;
} virtio_gpu_cursor_pos;

typedef struct virtio_gpu_update_cursor 
{
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_cursor_pos pos;
    uint32_t resource_id;
    uint32_t hot_x;
    uint32_t hot_y;
    uint32_t padding;
} virtio_gpu_update_cursor;


void VGPU_Init(uint8_t bus, uint8_t slot, uint8_t function);

inline uint64_t VGPU_ReadFeatures(void);

inline uint8_t VGPU_ReadStatus(void);

inline void VGPU_WriteFeatures(uint64_t features);

inline void VGPU_WriteStatus(uint8_t status);
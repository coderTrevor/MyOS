#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "Virtio.h"

// Queue numbers
#define VIRTIO_GPU_CONTROL_Q_INDEX  0
#define VIRITO_GPU_CURSOR_Q_INDEX   1

// Feature bits
#define VIRTIO_GPU_F_VIRGL  1
#define VIRTIO_GPU_F_EDID   2

// Sadly, VIRTIO_GPU_F_VIRGL doesn't work on Windows (which sucks because that's the whole reason I started implementing a virtio-gpu driver)
// Qemu seems to insist on using these features, whether we negotiate them or not:
#undef REQUIRED_FEATURES
#define REQUIRED_FEATURES   (VIRTIO_F_VERSION_1 | VIRTIO_F_RING_INDIRECT_DESC | VIRTIO_F_RING_EVENT_IDX)

#define VIRTIO_GPU_EVENT_DISPLAY (1 << 0)

enum virtio_gpu_ctrl_type 
{
    /* 2d commands */
    VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
    VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
    VIRTIO_GPU_CMD_RESOURCE_UNREF,
    VIRTIO_GPU_CMD_SET_SCANOUT,
    VIRTIO_GPU_CMD_RESOURCE_FLUSH,
    VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
    VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
    VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
    VIRTIO_GPU_CMD_GET_CAPSET_INFO,
    VIRTIO_GPU_CMD_GET_CAPSET,
    VIRTIO_GPU_CMD_GET_EDID,

    /* cursor commands */
    VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
    VIRTIO_GPU_CMD_MOVE_CURSOR,

    /* success responses */
    VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
    VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
    VIRTIO_GPU_RESP_OK_CAPSET_INFO,
    VIRTIO_GPU_RESP_OK_CAPSET,
    VIRTIO_GPU_RESP_OK_EDID,
    
    /* error responses */
    VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
    VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
    VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

typedef struct virtio_gpu_config 
{
    uint32_t events_read;
    uint32_t events_clear;
    uint32_t num_scanouts;
    uint32_t reserved;
} virtio_gpu_config;

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


extern virtq controlQueue;


void VGPU_Init(uint8_t bus, uint8_t slot, uint8_t function);

void VGPU_Init_Virtqueue(virtq *virtqueue, uint16_t queueIndex);

void VGPU_InterruptHandler();

inline uint64_t VGPU_ReadFeatures(void);

inline uint8_t VGPU_ReadStatus(void);

inline void VGPU_SelectQueue(uint16_t queueIndex);

inline void VGPU_SetQueueAddresses(virtq *virtqueue);

void VGPU_SetupDeviceBuffers();

bool VGPU_SharedInterruptHandler(void);

inline void VGPU_WriteFeatures(uint64_t features);

inline void VGPU_WriteStatus(uint8_t status);
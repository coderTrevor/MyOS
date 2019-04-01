#pragma once
#include <stdint.h>

#define BITMAP_MAGIC 0x4D42

#define BITMAP_COMPRESSION_BI_RGB   0 /* Uncompressed */

#define BITMAP_BYTES_PER_24BPP  3 /* makes the code more readable */

#pragma pack(push, 1)
typedef struct DIB_Header
{
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;
    uint16_t colorPlanes;
    uint16_t bpp;
    uint32_t compressionType;
} DIB_Header;

typedef struct BMP_Header
{
    uint16_t magic;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t pixelsOffset;
    DIB_Header dibHeader;
} BMP_Header;
#pragma pack(pop)

bool Bitmap24Load(char *filename, PIXEL_32BIT *buffer, uint32_t maxBufferSize, uint32_t *width, uint32_t *height);

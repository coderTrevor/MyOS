#include "Display_HAL.h"
#include "../Networking/TFTP.h"
#include "../Terminal.h"
#include "Bitmap.h"
#include "../misc.h"
#include "../Networking/IPv4.h"

//#define MAX_BMP_FILESIZE (256 * 1024)
//uint8_t rawFileBuffer[MAX_BMP_FILESIZE];

void *malloc(size_t);

// returns true on success
// *ppBuffer will have the address of an allocated buffer containing the image on success, NULL otherwise.
// Caller must free *ppBuffer.
bool Bitmap24Load(char *filename, PIXEL_32BIT **ppBuffer, uint32_t *width, uint32_t *height)
{
    *ppBuffer = NULL;

    // Get the size of the image file
    uint32_t fileSize;
    if (!TFTP_GetFileSize(IPv4_PackIP(10, 0, 2, 2), filename, &fileSize))
    {
        terminal_writestring("Failed to determine size of ");
        terminal_writestring(filename);
        terminal_newline();
        return false;
    }

    // Allocate a buffer for the bitmap file
    uint8_t *rawFileBuffer = malloc(fileSize);
    if (!rawFileBuffer)
    {
        terminal_writestring("Not enough memory to open bitmap file\n");
        return false;
    }

    // Get the file via TFTP
    if (!TFTP_GetFile(IPv4_PackIP(10, 0, 2, 2), filename, rawFileBuffer, fileSize, NULL))
    {
        terminal_writestring("Unable to open ");
        terminal_writestring(filename);
        terminal_newline();

        free(rawFileBuffer);

        return false;
    }

    BMP_Header *bmpHeader = (BMP_Header *)rawFileBuffer;

    // Check for the magic number
    if (bmpHeader->magic != BITMAP_MAGIC)
    {
        terminal_writestring(filename);
        terminal_writestring(" is not a 24-bit bitmap file as far as I can tell.\nI don't know how to load it.\n");
        if (debugLevel)
        {
            terminal_writestring("Image magic is ");
            terminal_print_ushort_hex(bmpHeader->magic);
            terminal_newline();
            terminal_dumpHex((uint8_t *)bmpHeader, 64);
        }

        free(rawFileBuffer);

        return false;
    }
    
    if (debugLevel)
        terminal_writestring("BMP file recognized\n");

    // Find where the pixel data begins
    uint32_t pixelDataOffset = bmpHeader->pixelsOffset;

    // Ensure the image is a 24-bit bitmap
    if (bmpHeader->dibHeader.bpp != 24)
    {
        terminal_writestring("I'm sorry but this image isn't a 24-bit bitmap; I don't know how to display it.\n");
        terminal_writestring("Image appears to be ");
        terminal_print_int(bmpHeader->dibHeader.bpp);
        terminal_writestring("-bit.\n");

        free(rawFileBuffer);

        return false;
    }

    // Ensure the file is in RGB format
    if (bmpHeader->dibHeader.compressionType != BITMAP_COMPRESSION_BI_RGB)
    {
        terminal_writestring("I'm sorry but this image is compressed; I don't know how to display it.\n");
        
        free(rawFileBuffer);

        return false;
    }

    // Get the image dimensions
    *width = bmpHeader->dibHeader.width;
    *height = bmpHeader->dibHeader.height;

    if (debugLevel)
    {
        terminal_writestring("Image is ");
        terminal_print_int(*width);
        terminal_writestring(" x ");
        terminal_print_int(*height);
        terminal_writestring(" x 24\n");
    }

    // Allocate image buffer
    PIXEL_32BIT *buffer = malloc((*width) * (*height) * sizeof(PIXEL_32BIT));
    if (!buffer)
    {
        terminal_writestring("Not enough memory to store bitmap image\n");
        
        free(rawFileBuffer);

        return false;
    }
    *ppBuffer = buffer;

    uint8_t *pixelData = (uint8_t*)((uint32_t)bmpHeader + pixelDataOffset);
    PIXEL_32BIT *currentPixel = buffer;

    // For stupid reasons, bitmaps are stored "upside-down," so we'll need to flip them vertically
    // start at the bottom row of the pixelData
    pixelData += (*height - 1) * (*width) * BITMAP_BYTES_PER_24BPP;

    // for every line
    for (unsigned int y = 0; y < *height; ++y)
    {
        // for every pixel of the current line
        for (unsigned int x = 0; x < *width; ++x)
        {
            // copy the color elements from the pixel data to the current pixel
            currentPixel->alpha = 0;
            currentPixel->blue = *(pixelData++);
            currentPixel->green = *(pixelData++);
            currentPixel->red = *(pixelData++);
            ++currentPixel;
        }

        // back pixelData up to the previous row (we backup an "extra" row because we advanced to the end of the current row in the for loop)
        pixelData -= (*width * BITMAP_BYTES_PER_24BPP * 2);
    }

    free(rawFileBuffer);

    return true;
}
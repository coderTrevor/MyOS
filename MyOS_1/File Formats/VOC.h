#pragma once
#include <stdint.h>

// Creative Voice File format
// Information taken from http://www.inversereality.org/tutorials/sound programming/examples/soundblaster16example3.html
// (via archive.org)

#define VOC_MAGIC "Creative Voice File\x1A"

#define BLOCK_TYPE_END          0
#define BLOCK_TYPE_NEW_SAMPLE   1
#define BLOCK_TYPE_NEW_BLOCK    9

#define SUB_BLOCK_HEADER_SIZE   4

// TEMPTEMP
#define MAX_VOC_SIZE (64 * 1024)

// Structs
#pragma pack(push, 1)
typedef struct VOC_Sub_Block
{
    uint8_t blockType;
    // block length is actually represented with 3 bytes
    uint8_t blockLength[3];
    uint8_t data[1];
} VOC_Sub_Block;

typedef struct VOC_Header
{
    char magicString[0x14];
    uint16_t dataOffset;
    uint8_t fileVersionMinor;
    uint8_t fileVersionMajor;
    uint16_t idCode;
} VOC_Header;

typedef struct VOC_Block_Type_New
{
    uint32_t samplesPerSecond;
    uint8_t  bitsPerSample;
    uint8_t  channel;
    uint16_t format;
    uint8_t  reserved[4];
}VOC_Block_Type_New;

typedef struct Sound_Struct
{
    uint32_t Length;
    uint32_t position;
    uint8_t data[MAX_VOC_SIZE];
}Sound_Struct;
#pragma pack(pop)


// Functions
void OpenAndReadVOCs(void);

void PlaySound(int index);

bool ReadVOC(VOC_Header *vocData, int index);

void SetupTimerForPlayback(void);
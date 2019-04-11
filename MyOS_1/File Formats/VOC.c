#include <stdbool.h>
#include "VOC.h"
#include "../Networking/TFTP.h"
#include "../Networking/IPv4.h"
#include "../Terminal.h"
#include "../misc.h"
#include "../Timers/PIT.h"
#include "../Timers/System_Clock.h"
#include "../Drivers/Sound_Blaster_16.h"

// Globals
// TEMPTEMP HACKHACK
uint8_t vocFile1[MAX_VOC_SIZE];
uint8_t vocFile2[MAX_VOC_SIZE];

Sound_Struct sounds[2];
unsigned int soundIndex = 0;
bool vocsLoaded = false;

// how we temporarily tell the interrupt we have a sound to be played
extern bool playSound;


// Functions

void ReadSubBlockTypeNew(VOC_Block_Type_New *subBlock, uint32_t blockLength, int index)
{
    uint32_t dataLength = blockLength - sizeof(VOC_Block_Type_New);
    uint8_t *dataPtr = (uint8_t *)((uint32_t)subBlock + sizeof(VOC_Block_Type_New));

    terminal_writestring("  sub-block is ");
    terminal_print_int(dataLength);
    terminal_writestring(" byte, ");
    terminal_print_int(subBlock->bitsPerSample);
    terminal_writestring("-bit, ");
    terminal_print_int(subBlock->channel);
    terminal_writestring("-channel sample in ");

    switch (subBlock->format)
    {
        case 0:
            terminal_writestring("PCM");
            break;
        case 1:
        case 2:
        case 3:
        case 0x200:
            terminal_writestring("ADPCM");
            break;
        case 4:
            terminal_writestring("Signed");
            break;
        case 5:
            terminal_writestring("ALAW");
            break;
        case 6:
            terminal_writestring("MULAW");
            break;
        default:
            terminal_writestring("Unsupported");
    }
    terminal_writestring(" format, ");
    terminal_print_int(subBlock->samplesPerSecond);
    terminal_writestring(" hz.\n");

    // TEMPTEMP
    memset(sounds[index].data, 0, MAX_VOC_SIZE);
    memcpy(sounds[index].data, dataPtr, dataLength);
    sounds[index].Length = dataLength;
    //soundIndex++;
}

bool ReadVOC(VOC_Header *vocData, int index)
{
    if (memcmp(vocData->magicString, VOC_MAGIC, strlen(VOC_MAGIC) - 1) != 0)
    {
        terminal_writestring("Not a valid VOC file\n");
        return false;
    }

    terminal_writestring("VOC file version ");
    terminal_print_byte(vocData->fileVersionMajor);
    terminal_putchar('.');
    terminal_print_byte(vocData->fileVersionMinor);
    terminal_newline();

    if (vocData->fileVersionMajor != 1 || (vocData->fileVersionMinor != 10 && vocData->fileVersionMinor != 20))
    {
        terminal_writestring("Don't know how to open this VOC version\n");
        return false;
    }

    uint16_t versionBytes = vocData->fileVersionMajor << 8 | vocData->fileVersionMinor;
    uint16_t idCode = (uint16_t)~versionBytes + 0x1234;
    if (idCode != vocData->idCode)
    {
        terminal_writestring("Invalid id code (checksum) for VOC file\nWas expecting ");
        terminal_print_ushort_hex(idCode);
        terminal_writestring(" but file id code was ");
        terminal_print_ushort_hex(vocData->idCode);
        terminal_newline();
        return false;
    }

    terminal_writestring("data offset: ");
    terminal_print_ushort_hex(vocData->dataOffset);
    terminal_newline();

    VOC_Sub_Block *currentBlock = (VOC_Sub_Block *)((uint32_t)vocData + vocData->dataOffset);
    int currentBlockNumber = 0;
    // TODO: Make sure we don't go past the EOF
    while (currentBlock->blockType != BLOCK_TYPE_END)
    {
        uint32_t blockSize = 0;

        // copy block size
        memcpy(&blockSize, currentBlock->blockLength, 3);

        terminal_writestring("Processing block ");
        terminal_print_int(currentBlockNumber);
        terminal_writestring(" type ");
        terminal_print_int(currentBlock->blockType);
        terminal_writestring(" with size ");
        terminal_print_int(blockSize);
        terminal_newline();

        VOC_Block_Type_New *blockNew;
        switch (currentBlock->blockType)
        {
            case BLOCK_TYPE_NEW_BLOCK:
                blockNew = (VOC_Block_Type_New *)((uint32_t)currentBlock + SUB_BLOCK_HEADER_SIZE);
                ReadSubBlockTypeNew(blockNew, blockSize, index++);
                break;
            default:
                terminal_writestring("Don't know how to read sub-block type ");
                terminal_print_int(currentBlock->blockType);
                terminal_newline();
        }

        // Advance to next block, blockSize doesn't count block header (4 bytes)
        currentBlock = (VOC_Sub_Block *)((uint32_t)currentBlock + blockSize + SUB_BLOCK_HEADER_SIZE);
        ++currentBlockNumber;
    }
        
    return true;
}

bool OpenVOC(char *fileName, uint8_t *destinationBuffer, uint32_t maxBufferSize)
{
    // TEMPTEMP: Use tftp to open voc files and use preallocated buffers instead of dynamic memory allocation
    return TFTP_GetFile(IPv4_PackIP(10, 0, 2, 2), fileName, destinationBuffer, maxBufferSize, NULL);
}

bool OpenAndReadVOCs(void)
{
    if (!OpenVOC("ISFXMORE.VOC", vocFile1, MAX_VOC_SIZE))
    {
        terminal_writestring("Unable to open ");
        terminal_writestring("ISFXMORE.VOC\n");
        return false;
    }

    if (!ReadVOC((VOC_Header *)vocFile1, 0))
        return false;

    if (!OpenVOC("ISFXEND.VOC", vocFile2, MAX_VOC_SIZE))
    {
        terminal_writestring("Unable to open ");
        terminal_writestring("ISFXEND.VOC\n");
        return false;
    }

    if (!ReadVOC((VOC_Header *)vocFile2, 1))
        return false;

    vocsLoaded = true;
    return true;
}

void PlaySound(int index)
{
    if (!vocsLoaded)
        OpenAndReadVOCs();
    
    if (!vocsLoaded)
        return;

    SB16_Write(0xD1); // turn speaker on if it's off

    soundIndex = index;

    //SetupTimerForPlayback();

    sounds[index].position = 0;
    soundIndex = index;

    playSound = true;

    terminal_writestring("About to play ");
    terminal_print_int(sounds[index].Length);
    terminal_writestring(" byte sample\n");

    SB16_Play(sounds[index].data, sounds[index].Length, 11025);
}

void SetupTimerForPlayback(void)
{
    // The samples we have (at present) are 11025 hz, so we need to set the PIT accordingly
    if (ticksPerSecond != 11025)
        PIT_Set_Interval(11025);
}
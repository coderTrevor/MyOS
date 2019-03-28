#pragma once

#include <stdint.h>

#define DOS_MAGIC 0x5A4D
#define PE_MAGIC 0x00004550
#define PE32_MAGIC 0x010B

// Taken from https://en.wikibooks.org/wiki/X86_Disassembly/Windows_Executable_Files
typedef struct DOS_Header
{
    // short is 2 bytes, long is 4 bytes
    uint16_t signature;
    uint16_t lastsize;
    uint16_t nblocks;
    uint16_t nreloc;
    uint16_t hdrsize;
    uint16_t minalloc;
    uint16_t maxalloc;
    uint16_t pss; // 2 byte value
    uint16_t psp; // 2 byte value
    uint16_t checksum;
    uint16_t pip; // 2 byte value
    uint16_t pcs; // 2 byte value
    uint16_t relocpos;
    uint16_t noverlay;
    uint16_t reserved1[4];
    uint16_t oem_id;
    uint16_t oem_info;
    uint16_t reserved2[10];
    uint32_t  e_lfanew; // Offset to the 'PE\0\0' signature relative to the beginning of the file
} DOS_Header;

typedef struct PE_Header
{
    uint32_t mMagic; // PE\0\0 or 0x00004550
    uint16_t mMachine;
    uint16_t mNumberOfSections;
    uint32_t mTimeDateStamp;
    uint32_t mPointerToSymbolTable;
    uint32_t mNumberOfSymbols;
    uint16_t mSizeOfOptionalHeader;
    uint16_t mCharacteristics;
} PE_Header;

// From http://www.brokenthorn.com/Resources/OSDevPE.html
typedef struct _IMAGE_DATA_DIRECTORY 
{
    uint32_t VirtualAddress;		// RVA of table
    uint32_t Size;			// size of table
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct Pe32OptionalHeader
{
    uint16_t mMagic; // 0x010b - PE32, 0x020b - PE32+ (64 bit)
    uint8_t  mMajorLinkerVersion;
    uint8_t  mMinorLinkerVersion;
    uint32_t mSizeOfCode;
    uint32_t mSizeOfInitializedData;
    uint32_t mSizeOfUninitializedData;
    uint32_t mAddressOfEntryPoint;
    uint32_t mBaseOfCode;
    uint32_t mBaseOfData;
    uint32_t mImageBase;
    uint32_t mSectionAlignment;
    uint32_t mFileAlignment;
    uint16_t mMajorOperatingSystemVersion;
    uint16_t mMinorOperatingSystemVersion;
    uint16_t mMajorImageVersion;
    uint16_t mMinorImageVersion;
    uint16_t mMajorSubsystemVersion;
    uint16_t mMinorSubsystemVersion;
    uint32_t mWin32VersionValue;
    uint32_t mSizeOfImage;
    uint32_t mSizeOfHeaders;
    uint32_t mCheckSum;
    uint16_t mSubsystem;
    uint16_t mDllCharacteristics;
    uint32_t mSizeOfStackReserve;
    uint32_t mSizeOfStackCommit;
    uint32_t mSizeOfHeapReserve;
    uint32_t mSizeOfHeapCommit;
    uint32_t mLoaderFlags;
    uint32_t mNumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY mDataDirectories[1];
} PE32OptionalHeader;

typedef struct IMAGE_SECTION_HEADER 
{   // size 40 bytes
    char  mName[8];
    uint32_t mVirtualSize;
    uint32_t mVirtualAddress;
    uint32_t mSizeOfRawData;
    uint32_t mPointerToRawData;
    uint32_t mPointerToRealocations;
    uint32_t mPointerToLinenumbers;
    uint16_t mNumberOfRealocations;
    uint16_t mNumberOfLinenumbers;
    uint32_t mCharacteristics;
} IMAGE_SECTION_HEADER;

extern uint8_t peBuffer[10 * 1024];

bool loadAndRunPE(uint8_t *executableDestination, DOS_Header *mzAddress);
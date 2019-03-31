#pragma once
#include <stdint.h>
#include "Terminal.h"

// not sure why alignas() isn't working for me in MSVC 2015
// I can't get __declspect(align(4096)) to work either, the linker complains that 
// the /Align:512 switch is too low, but making it higher breaks the kernel :(
typedef /*__declspec(align(4096))*/ uint32_t PAGE_DIRECTORY_ENTRY;
typedef /*__declspec(align(4096))*/ uint32_t PAGE_TABLE_ENTRY;

#define PAGE_ENTRY_PRESENT          1
#define PAGE_ENTRY_USER_ACCESSIBLE  2
#define PAGE_ENTRY_WRITABLE         4

extern uint32_t paging_space[0x5FFF];

typedef uint32_t ULONG_PTR;

inline void Paging_Enable(multiboot_info *multibootInfo)
{
    // setup the page directory and page tables and enable paging
    PAGE_TABLE_ENTRY *identityTable;        // for identity mapping the first four megabytes
    PAGE_TABLE_ENTRY *programIdentityTable; // for identity mapping four megabytes starting at 8 megabytes
    PAGE_TABLE_ENTRY *initialKernelTable;   // for mapping the kernel to 0xC000 0000
    PAGE_TABLE_ENTRY *videoIdentityTable;   // for identity mapping the linear frame buffer (HACK!)
    PAGE_DIRECTORY_ENTRY *pageDirectory = (PAGE_DIRECTORY_ENTRY*)((uint32_t)paging_space - BASE_ADDRESS + LOADBASE);

    // Ensure pageDirectory is aligned on a 0x1000-byte boundary
    if ((ULONG_PTR)pageDirectory % 0x1000 != 0)
        pageDirectory = (PAGE_TABLE_ENTRY*)((ULONG_PTR)pageDirectory - ((ULONG_PTR)pageDirectory % 0x1000) + 0x1000);

    // Have the page tables follow pageDirectory in memory
    identityTable = (PAGE_TABLE_ENTRY *)((uint32_t)pageDirectory + 0x1000);
    programIdentityTable = (PAGE_TABLE_ENTRY *)((uint32_t)pageDirectory + 0x2000);
    initialKernelTable = (PAGE_TABLE_ENTRY *)((uint32_t)pageDirectory + 0x3000);
    videoIdentityTable = (PAGE_TABLE_ENTRY *)((uint32_t)pageDirectory + 0x4000);

    uint32_t i;

    // Clear out the page directory
    for (i = 0; i < 1024; ++i)
    {
        pageDirectory[i] = PAGE_ENTRY_USER_ACCESSIBLE | PAGE_ENTRY_WRITABLE;
    }

    // Setup identity mapping for the first four megabytes
    for (i = 0; i < 1024; ++i)
    {
        identityTable[i] = (i * 0x1000) | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE;   // attributes: supervisor level, read/write, present
    }

    // put the first page table in the page directory
    pageDirectory[0] = (PAGE_DIRECTORY_ENTRY)((uint32_t)identityTable | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE);

    // Setup identity mapping for the four megabytes starting at megabyte 8
    for (i = 0; i < 1024; ++i)
    {
        programIdentityTable[i] = (0x800000 + i * 0x1000) | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE;   // attributes: supervisor level, read/write, present
    }

    // put the program page table in the page directory (0x80 0000) 
    pageDirectory[2] = (PAGE_DIRECTORY_ENTRY)((uint32_t)identityTable | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE);

    // Map the kernel (4 megs starting at 0x10 0000) to 0xC000 0000
    for (i = 0; i < 1024; ++i)
    {
        initialKernelTable[i] = (0x100000 + i * 0x1000) | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE;   // attributes: supervisor level, read/write, present
    }

    // put the kernel page table in the page directory into entry 768, which will map it to 0xC000 0000
    pageDirectory[768] = (PAGE_DIRECTORY_ENTRY)((uint32_t)initialKernelTable | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE);

    // TEMPTEMP HACKHACK! - identity map the linear frame buffer, which on my Qemu starts at 0xFD00 0000
    uint32_t lfbAddress = 0xFD000000;

    // see if Grub gave us an lfb address and use that one if it did
    if (multibootInfo->flags &  MULTIBOOT_INFO_FRAMEBUFFER_INFO)
    {
        // fingers crossed that lfb is 32-bits // (TODO)
        lfbAddress = (uint32_t)multibootInfo->framebuffer_addr;
    }

    // TODO: allow the lfb to exist anywhere in memory
    // Setup identity mapping for the four megabytes starting at 0xFD00 0000
    for (i = 0; i < 1024; ++i)
    {
        videoIdentityTable[i] = (lfbAddress + i * 0x1000) | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE;   // attributes: supervisor level, read/write, present
    }

    // put the lfb page table in the page directory
    uint32_t page = lfbAddress / 0x400000;
    pageDirectory[page] = (PAGE_DIRECTORY_ENTRY)((uint32_t)videoIdentityTable | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE);

    // load page directory into cr3
    __writecr3((uint32_t)pageDirectory);

    // enable paging!
    __writecr0(__readcr0() | 0x80000000);
}
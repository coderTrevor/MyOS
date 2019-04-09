#include "multiboot.h"
#include "Paging.h"
#include "Console_VGA.h"
#include <stdint.h>

// Allocate space for paging structures. We need enough space for three arrays with 0x1000 32-bit entries each.
// Each array must be aligned on a 0x1000-byte boundary.
// I can't get alignment working via keywords, so I'll just allocate enough extra space to accomodate a 4096-aligned pointer.
// When we use these arrays, we'll need to subtract BASE_ADDRESS from their addresses and add LOADBASE, because paging hasn't been setup yet,
// and the linker will allocate them based on the given base address.
uint32_t paging_space[0x3FFF];

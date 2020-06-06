
#include "misc.h"
    
void* operator new(size_t size)
{
    // Allocate memory
    return dbg_alloc(size);
}


void* operator new[](size_t size)
{
    // Allocate memory
    return dbg_alloc(size);
}


void operator delete(void *p)
{
    if (p == 0) {
        return;
    }

    // Release allocated memory
    dbg_release(p);
}

void operator delete[](void *p)
{
    if (p == 0) {
        return;
    }

    // Release allocated memory
    dbg_release(p);
}

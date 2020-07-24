#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

void* kmalloc(size_t size);
void kfree(void *ptr);

#ifdef __cplusplus
};
#endif /* __cplusplus */

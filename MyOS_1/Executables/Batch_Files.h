#pragma once
#include <stdbool.h>
#include <stdint.h>

bool IsBatchFile(char *filename);

void RunBatch(char *batchFile, size_t fileSize);
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_PATH    64
#define MAX_FILES   64
#define EOF         -1

// (Taken from SDL for compatibility:)
#define SEEK_SET 0       /**< Seek from the beginning of data */
#define SEEK_CUR 1       /**< Seek relative to current read point */
#define SEEK_END 2       /**< Seek relative to the end of data */

#ifndef FILE
typedef int FILE;                   // index into open files array
#endif
typedef unsigned int FILE_POSITION; // current read/write position
typedef char FILENAME[MAX_PATH];

// Struct-of-arrays for open files TODO: Maybe this should actually be AOS?
typedef struct OPEN_FILES
{
    FILENAME filename[MAX_FILES];
    size_t filePos[MAX_FILES];
    size_t fileSize[MAX_FILES];
    bool readOnly[MAX_FILES];
    bool isOpen[MAX_FILES];
    uint8_t *buffer[MAX_FILES];
} OPEN_FILES;


int file_close(int fp);

int file_open(const char * filename, const char * mode);

size_t file_read(void * ptr, size_t size, size_t count, int fp);

int file_seek(FILE * stream, long int offset, int origin);

long int file_tell(FILE * stream);
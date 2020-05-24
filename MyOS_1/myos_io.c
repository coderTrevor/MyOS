#include "myos_io.h"
#include "misc.h"
#include "printf.h"
#include "Networking/TFTP.h"

OPEN_FILES openFiles = { 0 };

int file_close(int fp)
{
    if (fp >= MAX_FILES || fp < 0)
    {
        kprintf("File pointer exceeds limits\n");
        return EOF;
    }

    // Free memory associated with file
    free(openFiles.buffer[fp]);

    openFiles.isOpen[fp] = false;

    return 0;
}

// TODO: Should never return 0 on success
int file_open(const char * filename, const char * mode)
{
    if (strcmp(mode, "r") != 0 && strcmp(mode, "rb") != 0)
        kprintf("Warning: fopen() called with unimplemented mode %s\n", mode);

    // Check for the presence of the file and get its size
    uint32_t fileSize;
    if (!TFTP_GetFileSize(tftpServerIP, (char *)filename, &fileSize))
    {
        kprintf("file %s not found\n", filename);
        return -1;
    }

    // Find the first free slot in openFiles structure
    int index;
    for (index = 0; index < MAX_FILES; ++index)
    {
        if (!openFiles.isOpen[index])
            break;
    }

    if (index == MAX_FILES)
    {
        kprintf("Can't open %s because too many files are currently open\n", filename);
        return -1;
    }

    // cplusplus.com notes: The returned stream is fully buffered by default if it is known to not refer to an interactive device (see setbuf).
    uint8_t *buffer = malloc(fileSize);

    if (!buffer)
    {
        kprintf("Not enough memory to open %s\n", filename);
        return -1;
    }

    if (!TFTP_GetFile(tftpServerIP, (char *)filename, buffer, fileSize, NULL))
    {
        kprintf("Not able to open %s", filename);
        return -1;
    }

    strncpy(openFiles.filename[index], filename, MAX_PATH);
    openFiles.filePos[index] = 0;
    openFiles.fileSize[index] = fileSize;
    openFiles.readOnly[index] = true;
    openFiles.isOpen[index] = true;
    openFiles.buffer[index] = buffer;

    kprintf("opened %s in slot %d\n", filename, index);

    return index;
}

size_t file_read(void * ptr, size_t size, size_t count, int fp)
{
    // TODO: Won't work if size * count is greater than what size_t can represent
    // TODO: Set error or eof as appropriate

    if(debugLevel)
        kprintf("fread(%p, %d, %d, %d)\n", ptr, size, count, fp);

    if (!size || !count)
        return 0;

    size_t readSize = size * count;

    // See if we're supposed to read past the end of the file
    if (readSize + openFiles.filePos[fp] > openFiles.fileSize[fp])
    {
        readSize = openFiles.fileSize[fp] - openFiles.filePos[fp];
    }
    
    // Copy buffer based on the current file pointer
    uint8_t *buffer = (uint8_t *)((uint32_t)openFiles.buffer[fp] + openFiles.filePos[fp]);
    memcpy(ptr, buffer, readSize);

    // Advance file pointer
    openFiles.filePos[fp] += readSize;

    return readSize;
}

// TODO: Support EOF bit
int file_seek(FILE * stream, long int offset, int origin)
{
    int fp = (int)stream;
    
    if (fp < 0 || fp >= MAX_FILES)
    {
        kprintf("file_seek: Invalid file index passed, %d\n", fp);
        return -1;
    }

    if (!openFiles.isOpen[fp])
    {
        kprintf("file_seek called on closed file with index %d\n", fp);
        return -1;
    }

    if(debugLevel)
        kprintf("fseek(%d, %d, ", stream, offset);

    switch (origin)
    {
        case SEEK_SET:
            // Seek based on beginning of file
            if(debugLevel)
                kprintf(" SEEK_SET)\n");

            if(offset >= 0)
                openFiles.filePos[fp] = offset;
            else
            {
                kprintf("file_seek: requested offset would make file pointer negative!\n");
                openFiles.filePos[fp] = 0;
                return -1;
            }
            break;

        case SEEK_CUR:
            // Seek based on current file pointer
            if (debugLevel)
                kprintf(" SEEK_CUR)\n");

            if(offset >= 0 || ((unsigned long)(-offset) >= openFiles.filePos[fp]))
                openFiles.filePos[fp] += offset;
            else
            {
                kprintf("file_seek: requested offset would make file pointer negative!\n");
                openFiles.filePos[fp] = 0;
                return -1;
            }
            break;

        case SEEK_END:
            // Seek based on end of file
            if (debugLevel)
                kprintf(" SEEK_END)\n");

            if(offset >= 0 || ((unsigned long)(-offset) >= openFiles.fileSize[fp]))
                openFiles.filePos[fp] = openFiles.fileSize[fp] + offset;
            else
            {
                kprintf("file_seek: requested offset would make file pointer negative!\n");
                openFiles.filePos[fp] = 0;
                return -1;
            }
            break;

        default:
            kprintf("file_seek: Invalid value passed for origin, %d\n", origin);
            return -1;
    }

    if (openFiles.filePos >= openFiles.fileSize)
    {
        kprintf("seek_file: file position exceeded file size!\n");
        openFiles.filePos[fp] = openFiles.fileSize[fp] - 1;
        return -1;
    }

    return 0;
}

long int file_tell(FILE * stream)
{
    int fp = (int)stream;

    if (fp < 0 || fp >= MAX_FILES)
    {
        kprintf("file_tell: Invalid file index passed, %d\n", fp);
        return -1L;
    }

    if (!openFiles.isOpen[fp])
    {
        kprintf("file_tell called on closed file with index %d\n", fp);
        return -1L;
    }

    return openFiles.filePos[fp];
}
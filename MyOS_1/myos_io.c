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
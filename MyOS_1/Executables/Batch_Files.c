#include "Batch_Files.h"
#include "../Console_Shell.h"
#include "../misc.h"
#include "../Terminal.h"

// Batch files are just a collection of commands the shell understands.
// They are similar to shell scripts but they are identified by their .bat extension.

bool BatchEcho = true;

// see if the filename ends with .bat
bool IsBatchFile(char *filename)
{
    int commandLength = strlen(filename);
    char extension[4];
    strncpy(extension, &filename[commandLength - 4], 4);
    
    if (strcmp(extension, ".bat") == 0)
        return true;

    return false;
}

// TODO: Copy batch file to a new string so we can run other executables and batch files from the current batch file
// TODO: Support Mac line-endings ('\r')
void RunBatch(char *batchFile, size_t fileSize)
{
    size_t currentPosition = 0;
    int lineLength;

    if (debugLevel)
    {
        terminal_writestring("Batch file size: ");
        terminal_print_int(fileSize);
        terminal_newline();
    }

    char currentLine[MAX_COMMAND_LENGTH + 1];   // TODO: this may be redundant since we copy the string to currentCommand anyway
    memset(currentLine, 0, MAX_COMMAND_LENGTH + 1);

    // process each line of the batch file
    while (currentPosition < fileSize)
    {
        // find the end of the next line
        char *endOfLine = strchr(&batchFile[currentPosition], '\n');

        // See if '\n' was found
        if (endOfLine)
            lineLength = endOfLine - &batchFile[currentPosition];
        else
        {
            lineLength = fileSize - currentPosition;
            if (lineLength > MAX_COMMAND_LENGTH)
                lineLength = MAX_COMMAND_LENGTH;
        }

        // copy the current line to currentLine
        strncpy(currentLine, &batchFile[currentPosition], lineLength);
        currentLine[lineLength] = '\0';


        // see if we have Windows line endings, and remove '\r' if we do
        if (currentLine[lineLength - 1] == '\r')
            currentLine[lineLength - 1] = '\0';

        if (BatchEcho)
        {
            // print the current command before executing it
            terminal_writestring(currentLine);
            terminal_newline();
        }

        if (debugLevel)
        {
            terminal_print_int(lineLength);
            terminal_newline();
        }

        // skip blank lines
        if (currentLine[0] != '\0')
        {            
            // have the shell execute currentLine
            strncpy(currentCommand, currentLine, lineLength);
            currentCommand[lineLength] = '\0';
            //terminal_writestring(currentCommand);
            //terminal_newline();
            Shell_Process_command();
        }

        currentPosition += lineLength + 1;
        if (batchFile[currentPosition] == '\r')
            currentPosition++;
    }
}
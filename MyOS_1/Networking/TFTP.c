#include "TFTP.h"
#include "../misc.h"
#include "../Console_VGA.h"
#include "Ethernet.h"

// TODO: Support multiple transactions
uint16_t transactionID;
volatile bool transferInProgress = false;
volatile bool transferError = false;

// TODO: Support dynamic memory allocation, when we have implemented it
uint8_t tftpFile[TFTP_MAX_FILE_SIZE];
uint8_t *nextFilePointer;
uint16_t currentBlockNumber;
uint32_t tftpFileSize;

uint32_t tftpServerIP; // TODO, maybe, support more than one server

bool tftpHideErrors = false;

bool TFTP_TransactionComplete()
{
    return !transferInProgress;
}

// return true on success
// will block until the file is transacted
// TODO: Implement timeout, error-checking
// TODO: Fix hanging if an invalid serverIP is given
// actualFileSize can be NULL if the caller doesn't care about the size
bool TFTP_GetFile(uint32_t serverIP, char *filename, uint8_t *destinationBuffer, uint32_t maxFileSize, uint32_t *actualFileSize)
{
    if(actualFileSize)
        *actualFileSize = 0;

    if (!NIC_Present)
        return false;

    if (debugLevel)
    {
        terminal_writestring("TFTP requesting ");
        terminal_writestring(filename);
        terminal_newline();
    }

    TFTP_RequestFile(serverIP, filename, TFTP_TYPE_BINARY, NIC_MAC);

    //return false;

    // wait until the transfer has completed
    while (transferInProgress)
    { }

    if (transferError)
    {
        if(!tftpHideErrors)
            terminal_writestring("An error occurred with the TFTP transfer\n");
        return false;
    }

    if (tftpFileSize > maxFileSize)
    {
        terminal_writestring("File requested exceeds maximum buffer provided\n");
        return false;
    }

    // copy the data
    memcpy(destinationBuffer, tftpFile, tftpFileSize);

    if (debugLevel)
    {
        terminal_writestring("TFTP File Size: ");
        terminal_print_int(tftpFileSize);
        terminal_newline();
    }

    if(actualFileSize)
        *actualFileSize = tftpFileSize;

    return true;
}

// source port is used to keep track of different transactions
uint16_t TFTP_RequestFile(uint32_t serverIP, char *filename, char *transferMode, uint8_t *sourceMAC)
{    
    TFTP_RequestHeader *tftpData;
    size_t filenameLength = strlen(filename) + 1; // length of filename plus null terminator
    size_t transferModeLength = strlen(transferMode) + 1;

    // packet size is 2 bytes (for the opcode) plus length of filename plus length of transferMode (both strings are zero terminated)
    uint16_t packetSize = (uint16_t)(sizeof(tftpData->opcode) + filenameLength + transferModeLength);
    uint16_t dataSize = packetSize;
    uint16_t sourcePort = 1234; // TODO: should be a random number (I think)
    
    // set some globals that will keep track of the transaction
    transactionID = sourcePort;
    transferInProgress = true;
    nextFilePointer = tftpFile;
    currentBlockNumber = 1; // TFTP Starts with block number 1
    tftpFileSize = 0;
    tftpServerIP = serverIP;
    transferError = false;

    // create the packet to send
    Ethernet_Header *packet = UDP_Create_Packet(serverIP, sourcePort, TFTP_PORT, &packetSize, sourceMAC, &tftpData);

    memset(tftpData, 0, dataSize);
    tftpData->opcode = TFTP_OP_READ_REQUEST;
    
    strncpy(tftpData->filename, filename, filenameLength - 1);
    char *transferModePtr = (char *)((uint32_t)tftpData->filename + filenameLength);
    strncpy(transferModePtr, transferMode, transferModeLength);

    EthernetSendPacket(packet, packetSize);

    return sourcePort;
}

void TFTP_SendAck(uint16_t blockNumber, uint16_t transID, uint16_t destinationPort, uint8_t *sourceMAC)
{
    TFTP_Ack *tftpAck;
    uint16_t packetSize = sizeof(TFTP_Ack);
    uint16_t sourcePort = transID;

    if (debugLevel)
    {
        terminal_writestring("TFTP sending ack for block ");
        terminal_print_int(SwapBytes16(blockNumber));
        terminal_newline();
    }

    // create the packet to send
    Ethernet_Header *packet = UDP_Create_Packet(tftpServerIP, sourcePort, destinationPort, &packetSize, sourceMAC, &tftpAck);
    tftpAck->opcode = TFTP_OP_ACK;
    tftpAck->blockNumber = blockNumber;

    EthernetSendPacket(packet, packetSize);
}

// transactionID will be the destination port of the UDP packet
void TFTP_ProcessDataPacket(TFTP_DataHeader *dataPacket, uint16_t sourcePort, uint16_t transID, uint16_t packetLength, uint8_t *sourceMAC)
{
    // data packet will use four bytes for opcode and block number
    uint16_t dataSize = packetLength - 4;

    if(debugLevel)
        terminal_writestring("TFTP Data received\n");

    // Ensure file isn't too big for the buffer we've set aside
    if (tftpFileSize + dataSize > TFTP_MAX_FILE_SIZE)
    {
        terminal_writestring("Error: requested tftp file exceeds buffer!\n");
        transferInProgress = false;
        transferError = true;

        // send acknowledgement to the server
        TFTP_SendAck(dataPacket->blockNumber, transID, sourcePort, sourceMAC);
        return;
    }
    
    // Ensure block number matches expected number
    uint16_t serverBlockNumber = SwapBytes16(dataPacket->blockNumber);
    if (currentBlockNumber != serverBlockNumber)
    {
        terminal_writestring("Error: tftp packet block number, ");
        terminal_print_int(serverBlockNumber);
        terminal_writestring(" doesn't match expected value of ");
        terminal_print_int(currentBlockNumber);
        terminal_newline();
        transferInProgress = false;
        transferError = true;
        return;
    }   

    // copy received data to the file buffer
    memcpy(nextFilePointer, dataPacket->data, dataSize);
    nextFilePointer += dataSize;
    tftpFileSize += dataSize;
    ++currentBlockNumber;
    
    // send acknowledgement to the server
    TFTP_SendAck(dataPacket->blockNumber, transID, sourcePort, sourceMAC);

    // see if this was the last block
    if (dataSize < TFTP_MAX_BLOCK_SIZE)
        transferInProgress = false;

    return;
}

void TFTP_ProcessPacket(TFTP_Header *packet, uint16_t sourcePort, uint16_t destinationPort, uint16_t packetLength, uint8_t *sourceMAC)
{
    // TODO: Dynamic memory allocation
    if(debugLevel)
        terminal_writestring("TFTP Packet received\n");

    if (packet->opcode == TFTP_OP_DATA)
    {
        TFTP_ProcessDataPacket((TFTP_DataHeader *)packet, sourcePort, destinationPort, packetLength, sourceMAC);
        return;
    }
    
    if (packet->opcode == TFTP_OP_ERROR)
    {
        TFTP_ErrorHeader *errorPacket = (TFTP_ErrorHeader *)packet;

        if (!tftpHideErrors)
        {
            terminal_writestring("TFTP Error packet received: ");
            terminal_writestring(errorPacket->errorMessage);
            terminal_newline();
        }

        transferError = true;
        transferInProgress = false;
        return;
    }
    
    terminal_writestring("Unhandled TFTP Opcode!\n");
}
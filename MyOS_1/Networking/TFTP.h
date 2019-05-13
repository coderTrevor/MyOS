#pragma once

#include <stdint.h>
#include "Ethernet.h"
#include "UDP.h"

// all of these defines are byte-swapped
#define TFTP_OP_READ_REQUEST       0x0100
#define TFTP_OP_WRITE_REQUEST      0x0200
#define TFTP_OP_DATA               0x0300
#define TFTP_OP_ACK                0x0400
#define TFTP_OP_ERROR              0x0500
#define TFTP_OP_OPTION_ACK         0x0600

#define TFTP_TYPE_ASCII             "netascii"
#define TFTP_TYPE_BINARY            "octet"

#define TFTP_PORT                   69

#define TFTP_MAX_BLOCK_SIZE         512

/* Reference taken from RFC 3151(I think?):
Type   Op #     Format without header

                2 bytes    string   1 byte     string   1 byte
                -----------------------------------------------
    RRQ/        | 01/02 |  Filename  |   0  |    Mode    |   0  |
    WRQ         -----------------------------------------------
                2 bytes    2 bytes       n bytes
                ---------------------------------
          DATA  | 03    |   Block #  |    Data    |
                ---------------------------------
                2 bytes    2 bytes
                -------------------
          ACK   | 04    |   Block #  |
                --------------------
                2 bytes  2 bytes        string    1 byte
                ----------------------------------------
        ERROR   | 05    |  ErrorCode |   ErrMsg   |   0  |
                ----------------------------------------
*/

#pragma pack(push, 1)

// I could maybe use a union to combine some of these but I won't bother

typedef struct TFTP_Header
{
    uint16_t opcode;
} TFTP_Header;

typedef struct TFTP_Ack
{
    uint16_t opcode;
    uint16_t blockNumber;
} TFTP_Ack;

typedef struct TFTP_DataHeader
{
    uint16_t opcode;
    uint16_t blockNumber;
    char data[1];
} TFTP_DataHeader;

typedef struct TFTP_ErrorHeader
{
    uint16_t opcode;
    uint16_t errorCode;
    char errorMessage[1];
} TFTP_ErrorHeader;

typedef struct TFTP_RequestHeader
{
    uint16_t opcode;
    char filename[1];
    // Filename is a variable-length, null terminated string.
    // A string for "Mode" will follow the filename
} TFTP_RequestHeader;

#pragma pack(pop, 1)


extern bool tftpHideErrors;
extern uint32_t tftpServerIP;


bool TFTP_GetFile(uint32_t serverIP, char *filename, uint8_t *destinationBuffer, uint32_t maxFileSize, uint32_t *actualFileSize);

bool TFTP_GetFileSize(uint32_t serverIP, char *filename, uint32_t *pActualFileSize);

uint16_t TFTP_RequestFile(uint32_t serverIP, char *filename, char *transferMode, uint8_t *sourceMAC);

void TFTP_ProcessPacket(TFTP_Header *packet, uint16_t sourcePort, uint16_t destinationPort, uint16_t packetLength, uint8_t *sourceMAC);


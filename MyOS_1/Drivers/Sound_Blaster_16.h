#pragma once
#include <stdbool.h>
#include <stdint.h>

// Values taken from https://wiki.osdev.org/Sound_Blaster_16
// and from http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html#SoundIO
// and inversereality via archive.org

// Mixer port offsets
#define MIXER_ADDR      0x04
#define MIXER_DATA      0x05

// Digital Sound Processor port offsets
#define DSP_RESET       0x06
#define DSP_READ        0x0A
#define DSP_WRITE       0x0C
#define DSP_BUFFER      0x0A
#define DSP_STATUS      0x0E
#define DSP_INTERRUPT   0x0F

// Digital Sound Processor commands
#define DSP_CMD_OUTPUT_RATE     0x41    /* Sample rate */
#define DSP_CMD_TRANSFER_MODE   0xB6
#define DSP_CMD_STOP            0xD5
#define DSP_CMD_VERSION         0xE1

// More specific I/O command breakdowns
#define DSP_IO_CMD_8BIT         0xC0
#define DSP_IO_CMD_16BIT        0xB0
#define DSP_IO_CMD_OUTPUT       0x00
#define DSP_IO_CMD_INPUT        0x08
#define DSP_IO_CMD_SINGLE_CYCLE 0x00
#define DSP_IO_CMD_AUTO_INIT    0x04
#define DSP_IO_CMD_FIFO_OFF     0x00
#define DSP_IO_CMD_FIFO_ON      0x02

// Transfer mode definitions
#define DSP_XFER_MODE_MONO      0x00
#define DSP_XFER_MODE_STEREO    0x20
#define DSP_XFER_MODE_UNSIGNED  0x00
#define DSP_XFER_MODE_SIGNED    0x10

//DMA ports
/*#define DMA_ADDRESS         0xC4
#define DMA_COUNT           0xC6
#define DMA_PAGE            0x8B
#define DMA_SINGLE_MASK     0xD4
#define DMA_TRANSFER_MODE   0xD6
#define DMA_CLEAR_POINTER   0xD8*/

#define SB16_BASE0          0x220
#define SB16_BASE1          0x240
#define SB16_BASE2          0x388

// other bases I haven't implemented yet:
#define SB16_BASE3          0x260
#define SB16_BASE4          0x280

#define READ_WRITE_READY_BIT    0x80
#define DSP_READY               0xAA

// Data from http://www.inversereality.org/tutorials/sound programming/examples/soundblaster16example2.html via archive.org
// MIXER OFFSETS
#define MIXER_MASTER_LEFT_VOL    0x30  /* upper 5 bits */
#define MIXER_MIC_VOL            0x3A  /* upper 5 bits */
#define MIXER_PC_SPEAKER_VOL     0x3B  /* upper 2 bits */
#define MIXER_TREBLE_LEFT_VOL    0x44  /* upper 4 bits */
#define MIXER_BASS_RIGHT_VOL     0x47  /* upper 4 bits */

#define MAX_5_BIT_VOLUME         0xF8
#define MAX_2_BIT_VOLUME         0xC0
#define MAX_4_BIT_VOLUME         0xF0


// globals
extern bool sb16Present;
extern uint16_t sb16BaseAddress;
extern uint8_t sb16IRQ;
extern uint8_t sb16_8bitDMAchannel;
extern uint8_t sb16_16bitDMAchannel;

// functions
void SB16_Init(void);

void SB16_Interrupt_Handler(void);

void SB16_Play(uint8_t *soundData, uint32_t sampleSize, uint16_t sampleRate);

uint8_t SB16_Read();

void SB16_SetMixerSettings(void);

void SB16_Write(uint8_t data);

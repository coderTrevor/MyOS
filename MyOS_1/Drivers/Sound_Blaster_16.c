#include <stdbool.h>
#include "Sound_Blaster_16.h"
#include "../Terminal.h"
#include "../System_Specific.h"
#include "../Timers/System_Clock.h"
#include "../Interrupts/Interrupts.h"
#include "../Interrupts/IDT.h"
#include "../Interrupts/PIC.h"
#include "ISA_DMA.h"
#include "../misc.h"

bool sb16Present = false;
uint16_t sb16BaseAddress = 0x220;
uint8_t sb16VersionMajor;
uint8_t sb16VersionMinor;
uint8_t sb16IRQ = 5;    // TODO: provide some way to configure the SB16 IRQ
uint8_t sb16_8bitDMAchannel = 1; // TODO: Make these configurable as well
uint8_t sb16_16bitDMAchannel = 5;

bool SB16_Reset(void)
{
    sb16Present = false;

    // send reset command
    outb(sb16BaseAddress + DSP_RESET, 1);
    
    // wait (much more than) 3.3 microseconds 
    TimeDelayMS(1);

    outb(sb16BaseAddress + DSP_RESET, 0);

    // if present, the DSP should respond within 100 microseconds    
    // TODO: determine how many ticks need to go by for 100 microseconds and don't wait any longer than that
    TimeDelayMS(1);
    if (!(inb(sb16BaseAddress + DSP_STATUS) & READ_WRITE_READY_BIT))
        return false;

    uint8_t readValue = inb(sb16BaseAddress + DSP_READ);
    if (readValue == DSP_READY)
        sb16Present = true;

    return sb16Present;
}

void SB16_Init(void)
{
    terminal_writestring("SB16 driver is looking for a SoundBlaster compatible card...\n");

    sb16BaseAddress = SB16_BASE0;
    
    // Try to send the DSP reset command
    if (!SB16_Reset())
    {
        // Failed to reset the DSP, try the next address
        sb16BaseAddress = SB16_BASE1;
        if (!SB16_Reset())
        {
            // Failed to reset the DSP again, try the final address
            sb16BaseAddress = SB16_BASE2;
            SB16_Reset();
        }
    }

    if (!sb16Present)
    {
        terminal_writestring("No SoundBlaster 16 device was found.\n");
        return;
    }

    // Get the DSP version
    SB16_Write(DSP_CMD_VERSION);
    sb16VersionMajor = SB16_Read();
    sb16VersionMinor = SB16_Read();

    terminal_writestring("SoundBlaster 16 found at ");
    terminal_print_ushort_hex(sb16BaseAddress);
    terminal_writestring(", DSP Version ");
    terminal_print_int(sb16VersionMajor);
    terminal_putchar('.');
    terminal_print_int(sb16VersionMinor);
    terminal_writestring(", initialized.\n");

    // Add an interrupt handler for the SB16
    Set_IDT_Entry((unsigned long)SB16_Interrupt_Handler, HARDWARE_INTERRUPTS_BASE + sb16IRQ);

    // Enable the SB16's IRQ
    IRQ_Enable_Line(sb16IRQ);
}

// TODO: Support various methods of playback - 8/16-bit, mono-stereo, etc...
void SB16_Play(uint8_t *soundData, uint32_t sampleSize, uint16_t sampleRate)
{
    //SB16_SetMixerSettings();


    // determine length of sample to send over DMA
    uint32_t sendLength = sampleSize;
    if (sendLength > (DMA_BUFFER_SIZE))
        sendLength = DMA_BUFFER_SIZE;

    terminal_writestring("Playing ");
    terminal_print_int(sendLength);
    terminal_writestring(" byte sample at ");
    terminal_print_int(sampleRate);
    terminal_writestring(" hz.\n");
    
    // Setup the DMA transfer:
    
    // disable interrupts
    _disable(); 
    
    // Disable DMA channel while programming it
    DMA_DisableChannel(sb16_8bitDMAchannel);

    // Set DMA modes
    DMA_SetMode(sb16_8bitDMAchannel,
                DMA_MODE_SINGLE_MODE | DMA_MODE_ADDRESS_INC | DMA_MODE_SINGLE_CYCLE | DMA_MODE_TRANSFER_READ);

    // Clear Flip-Flop
    DMA_ClearFlipFlop(sb16_8bitDMAchannel);

    // Set the DMA buffer location
    // We only have one DMA buffer right now, but that may change in the future
    DMA_InitBuffer();

    // Copy sound data to our DMA buffer
    memcpy(DMA_Buffer, soundData, sendLength);
    terminal_writestring("buffer: \n");
    terminal_dumpHex(DMA_Buffer, 256);
    terminal_newline();

    // zero out any remaining bytes
    if (sendLength > DMA_BUFFER_SIZE)
    {
        memset((void *)((uint32_t)DMA_Buffer + sendLength), 0, DMA_BUFFER_SIZE - sendLength);
    }

    DMA_SetBuffer(sb16_8bitDMAchannel, DMA_Buffer);

    DMA_SetTransferLength(sb16_8bitDMAchannel, sendLength);
    
    // enable interrupts
    _enable();

    // enable DMA channel
    DMA_EnableChannel(sb16_8bitDMAchannel);

    // Set output sample rate
    SB16_Write(DSP_CMD_OUTPUT_RATE);
    SB16_Write(sampleRate >> 8);    // high byte of sampleRate
    SB16_Write(sampleRate & 0xFF);  // low byte of sampleRate

    // set 8-bit, mono output

    // Write the command byte
    SB16_Write(DSP_IO_CMD_8BIT | DSP_IO_CMD_SINGLE_CYCLE | DSP_IO_CMD_OUTPUT | DSP_IO_CMD_FIFO_ON);   // 8-bit, single cycle, FIFO on

    // Write the mode byte
    SB16_Write(DSP_XFER_MODE_MONO | DSP_XFER_MODE_UNSIGNED);   // Mono

    // Send transfer length to SB16
    SB16_Write((sendLength - 1) & 0xFF);            // Send low byte
    SB16_Write((uint8_t)((sendLength - 1) >> 8));   // followed by high byte
}

uint8_t SB16_Read()
{
    // TODO: timeout after a while
    // wait until the DSP is ready to be read (wait until ready bit is set)
    while (!inb(sb16BaseAddress + DSP_STATUS) & READ_WRITE_READY_BIT)
        ;

    return inb(sb16BaseAddress + DSP_READ);
}

void SB16_SetMixerSettings(void)
{
    uint16_t mixerAddr = sb16BaseAddress + MIXER_ADDR;
    uint16_t mixerData = sb16BaseAddress + MIXER_DATA;

    // Update the volumes from master left to microphone
    // set max volumes for each
    for (uint8_t index = MIXER_MASTER_LEFT_VOL; index <= MIXER_MIC_VOL; ++index)
    {
        outb(mixerAddr, index);
        outb(mixerData, MAX_5_BIT_VOLUME);
    }

    // set max volume for pc speaker
    outb(mixerAddr, MIXER_PC_SPEAKER_VOL);
    outb(mixerData, MAX_2_BIT_VOLUME);

    /*pointer = &NewMixerSettings.outputswitches;
    for (BYTE index = OutPutSwitches; index <= InputR; index++)
    {
        outp(MixerAddr, index);
        outp(MixerData, *pointer);
        pointer++;
    }


    pointer = &NewMixerSettings.inputgainleft;
    for (BYTE index = InputGainLeft; index <= OutputGainRight; index++)
    {
        outp(MixerAddr, index);
        outp(MixerData, (*pointer) << 6);
        pointer++;
    }
    outp(MixerAddr, AGC);
    outp(MixerData, NewMixerSettings.agc);

    pointer = &NewMixerSettings.trebleleft;
    for (BYTE index = TrebleLeft; index <= BassRight; index++)
    {
        outp(MixerAddr, index);
        outp(MixerData, (*pointer) << 4);
        pointer++;
    }*/
    
}

void SB16_Write(uint8_t data)
{
    // TODO: timeout after a while
    // wait until the DSP is ready to be written to (wait until ready bit is cleared)
    while (inb(sb16BaseAddress + DSP_WRITE) & READ_WRITE_READY_BIT)
        ;

    outb(sb16BaseAddress + DSP_WRITE, data);
}

void __declspec(naked) SB16_Interrupt_Handler(void)
{
    _asm pushad;

    terminal_writestring("SB16 Interrupt Handler fired\n");

    ++interrupts_fired;

    // acknowledge the interrupt (just read the DSP status)
    inb(sb16BaseAddress + DSP_STATUS);

    terminal_writestring("SB16 Interrupt Handler done\n");

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + sb16IRQ);

    _asm
    {
        popad
        iretd
    }
}

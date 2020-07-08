#include <stdint.h>
#include "../Drivers/Keyboard.h"
#include "PIC.h"
#include "Interrupts.h"
#include "IDT.h"
#include "../System_Specific.h"
#include "../misc.h"
#include "../Console_VGA.h"
#include "../printf.h"
#include "../Terminal.h"
#include "../paging.h"
#include "../Timers/System_Clock.h"
#include "../Graphics/Display_HAL.h"
#include "../myos_io.h"
#include "../Tasks/Context.h"
#include "../Drivers/PS2_Mouse.h"
#include "../GUI_Kernel.h"
#include "../Debugging/Debug.h"

unsigned long interrupts_fired;

#define MAX_SHARED_HANDLERS 5

// HACKY - Store interrupt handlers in an array for each IRQ which may be shared
// Presently, we know only IRQ's 9 and 11 will wind up shared for the hardware / devices we support
bool (*irq9handlers[MAX_SHARED_HANDLERS])(void) = { 0 };
bool (*irq11handlers[MAX_SHARED_HANDLERS])(void) = { 0 };

int irq9handlerCount = 0;
int irq11handlerCount = 0;

void _declspec(naked) default_interrupt_handler(void)
{
    _asm pushad;

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("Default interrupt handler fired.\n");

    _asm
    {
        popad
        iretd
    }
}

void _declspec(naked) irq11_shared_interrupt_handler(void)
{
    _asm pushad;

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("irq 11 shared interrupt handler fired.\n");

    // Call each of the shared int 9 handlers
    for (int i = 0; i < irq11handlerCount; ++i)
    {
        // Call the shared handler. Break from loop if the device owning that handler fired an interrupt.
        if ((*irq11handlers[i])());
            //break;
    }

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + 11);

    _asm
    {
        popad
        iretd
    }
}

void _declspec(naked) irq9_shared_interrupt_handler(void)
{
    _asm pushad;

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("irq 9 shared interrupt handler fired.\n");

    // Call each of the shared int 9 handlers
    for (int i = 0; i < irq9handlerCount; ++i)
    {
        // Call the shared handler. Break from loop if the device owning that handler fired an interrupt.
        if ((*irq9handlers[i])());
            //break;
    }

    PIC_sendEOI(HARDWARE_INTERRUPTS_BASE + 9);

    _asm
    {
        popad
        iretd
    }
}

void _declspec(naked) close_app_interrupt_handler(int eflags, int cs, uint32_t PID)
{
    (void)eflags, (void)cs;

    __asm
    {
        // prologue
        push ebp
        mov ebp, esp

        // disable interrupts
        cli
    }

    ++interrupts_fired;

    CloseApp(PID);

    __asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) default_exception_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    terminal_writestring("Default exception handler fired.\n");
    terminal_writestring("System halted.\n");

    for (;;)
        __halt();

    /*_asm
    {
    popad
    iretd
    }*/
}

uint32_t espVal;
uint32_t *pReturnVal;
TASK_STACK_LAYOUT *pNewTask;

// In calling this interrupt handler, we can keep track of how much the stack grows starting at oldStackStart and
// ending at espVal (remembering that the stack grows downward, so espVal < oldStackStart). Then we can copy those
// bytes to the new stack, and when we swap in the new stack in our PIT interrupt handler, it will have the same
// data already pushed onto it, ready to be popped off at the end of the interrupt handler.
uint32_t _declspec(naked) dispatch_new_task_interrupt_handler(uint32_t eflags, uint32_t cs, uint32_t newStackBegin, uint32_t oldStackStart)
{
    (void)eflags, (void)cs;
    __asm
    {
        // prologue
        push ebp
        mov ebp, esp

        pushad

        // Keep track of where the stack starts (ends) for the new task
        mov[espVal], esp
    }
    
    pNewTask = (TASK_STACK_LAYOUT *)espVal;

    if (debugLevel)
    {
        terminal_writestring("eflags: ");
        terminal_print_ulong_hex(pNewTask->eflags);
        terminal_writestring("\ncs: ");
        terminal_print_ulong_hex(pNewTask->cs);
        pReturnVal = (uint32_t *)(pNewTask->returnAddress);
        terminal_writestring("\nReturn value: ");
        terminal_print_ulong_hex((uint32_t)pReturnVal);
        terminal_newline();
        terminal_writestring("old Stack Start: ");
        terminal_print_ulong_hex(oldStackStart);
        terminal_writestring("\nnew Stack: ");
        terminal_print_ulong_hex(newStackBegin);
        terminal_newline();
        terminal_writestring("\nnew esp: ");
        terminal_print_ulong_hex(espVal);
        terminal_newline();
        terminal_dumpHexAround((uint8_t *)espVal, 32, 192);
    }

    // copy the task stack to the new stack area
    memmove((void *)(newStackBegin - sizeof(TASK_STACK_LAYOUT)), pNewTask, sizeof(TASK_STACK_LAYOUT));

    __asm
    {
        popad

        // epilogue
        mov esp, ebp
        pop ebp

        iretd
    }
}

void _declspec(naked) exit_interrupt_handler(int eflags, int cs)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    //longjmp(peReturnBuf, 42); // Not only do I not like this way of exitting, it doesn't work right now
    ExitApp();

    // No need for iretd because we'll never return here
}

void _declspec(naked) fclose_interrupt_handler(int eflags, int cs, int fp, int *pRetVal)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("fclose interrupt handler fired.\n");

    *pRetVal = file_close(fp);

    _asm
    {
        // epilogue
        pop ebp

        iretd
    }

}

void _declspec(naked) fopen_interrupt_handler(int eflags, int cs, const char *filename, const char *mode, int *fp)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp

        // re-enable interrupts
        sti
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("fopen interrupt handler fired.\n");

    *fp = file_open(filename, mode);

    _asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) fread_interrupt_handler(int eflags, int cs, void * ptr, size_t size, size_t count, FILE * stream, size_t *pSize)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("fread interrupt handler fired.\n");

    *pSize = file_read(ptr, size, count, (int)stream);

    _asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) fseek_interrupt_handler(int eflags, int cs, FILE *stream, long int offset, int origin, int *pRetVal)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("fseek interrupt handler fired.\n");

    *pRetVal = file_seek(stream, offset, origin);

    _asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) ftell_interrupt_handler(int eflags, int cs, FILE *stream, long int *pRetVal)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("ftell interrupt handler fired.\n");

    *pRetVal = file_tell(stream);

    _asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) get_graphics_interrupt_handler(int eflags, int cs, bool *graphicsInitialized, int *width, int *height)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;
    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi

        pushad
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("syscall get graphics interrupt handler fired.\n");

    *graphicsInitialized = !textMode;
    *width = graphicsWidth;
    *height = graphicsHeight;

    _asm
    {
        popad

        // epilogue
        pop edi
        pop esi
        pop ebx
        pop ebp

        iretd
    }
}

void _declspec(naked) get_mouse_state_interrupt_handler(int eflags, int cs, MOUSE_STATE *pMouseState)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;
    _asm
    {
        push ebp
        mov ebp, esp

        pushad
    }

    *pMouseState = mouseState;

    //kprintf("getMouseState\n");

    _asm
    {
        popad
        pop ebp
        iretd
    }
}

void _declspec(naked) gpf_exception_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    uint32_t errorCode;
    uint32_t address;

    __asm
    {
        pop[errorCode]
        pop[address]
    }

    terminal_fill(' ', VGA_COLOR_WHITE, VGA_COLOR_BLUE);

    terminal_writestring("General Protection Fault handler fired.\nError code ");
    terminal_print_ulong_hex(errorCode);
    terminal_writestring("\nOffending instruction at ");
    terminal_print_ulong_hex(address);
    terminal_writestring(".\nMemory looks like:\n");
    terminal_dumpHexAround((uint8_t *)address, 128, 128);
    terminal_writestring("System halted.\n");

    for (;;)
        __halt();

    /*_asm
    {
    popad
    iretd
    }*/
}

void _declspec(naked) graphics_blit_interrupt_handler(int eflags, int cs, const SDL_Rect *sourceRect, PIXEL_32BIT *image)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("graphics blit allocator interrupt handler fired.\n");

    GraphicsBlit(sourceRect->x, sourceRect->y, image, sourceRect->w, sourceRect->h);

    _asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) gui_register_callback_interrupt_handler(int eflags, int cs, GUI_CALLBACK callbackFunc)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs, (void)callbackFunc;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
        pushad
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("GUI register callback interrupt handler fired.\n");

    // TODO IMPORTANT The GUI application should be mapped to kernel space

    guiCallback = callbackFunc;

    GUI_CallbackAdded();

    _asm
    {
        popad
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) hide_shell_display_interrupt_handler(int eflags, int cs)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;
    _asm
    {
        push ebp
        mov ebp, esp
    }

    showOverlay = false;
    cursorEnabled = false;
    
    _asm
    {
        pop ebp
        iretd
    }
}

void _declspec(naked) launch_app_interrupt_handler(int eflags, int cs, const char *appFilename, bool exclusive, bool *pRetVal)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    // prologue
    _asm
    {
        push ebp
        mov ebp, esp

        //pushad
    }

    *pRetVal = LaunchApp(appFilename, exclusive, 0x800000);

    // epilogue
    _asm
    {
        //popad

        pop ebp
        iretd
    }
}

void  _declspec(naked) page_allocator_interrupt_handler(int eflags, int cs, unsigned int pages, unsigned int *pPagesAllocated, uint32_t *pRetVal)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;

    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("syscall page allocator interrupt handler fired.\n");
        
    KPageAllocator(pages, pPagesAllocated, pRetVal);

    _asm
    {
        // epilogue
        pop ebp

       iretd
    }
}

void _declspec(naked) page_fault_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    uint32_t errorCode;
    uint32_t address;

    __asm
    {
        pop[errorCode]
        pop[address]
    }

    terminal_fill(' ', VGA_COLOR_WHITE, VGA_COLOR_BLUE);

    terminal_writestring("Page Fault handler fired by ");
    terminal_writestring(tasks[currentTask].imageName);
    terminal_writestring(".\nError code ");
    terminal_print_ulong_hex(errorCode);
    terminal_writestring("\nOffending instruction at ");
    terminal_print_ulong_hex(address);
    terminal_writestring(".\nMemory looks like:\n");
    //terminal_dumpHex((uint8_t *)address, 256);
    terminal_dumpHexAround((uint8_t *)address, 128, 128);

    uint32_t regvar;
    __asm
    {
        mov regvar, esp
    }
    kprintf("Value of esp: 0x%lX\n", regvar);

    __asm
    {
        mov regvar, ebp
    }
    kprintf("Value of ebp: 0x%lX\n", regvar);

    kprintf("Memory address accessed: 0x%lX\n", __readcr2());

    DebugStackTrace(10);

    terminal_writestring("System halted.\n");

    for (;;)
        __halt();
    
    /*_asm
    {
    popad
    iretd
    }*/
}

void _declspec(naked) invalid_opcode_handler(void)
{
    //_asm pushad;
    //__asm cli;

    ++interrupts_fired;

    uint32_t address;

    __asm
    {
        pop [address]
    }

    terminal_fill(' ', VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    terminal_writestring("Invalid opcode handler fired.\nEncountered invalid opcode at ");
    terminal_print_ulong_hex(address);
    
    DebugStackTrace(10);

    terminal_writestring(".\nMemory looks like:\n");
    terminal_dumpHexAround((uint8_t *)address, 128, 128);
    terminal_writestring("System halted.\n");

    for(;;)
        __halt();

    /*_asm
    {
    popad
    iretd
    }*/
}

void Interrupts_Add_Shared_Handler(bool (*sharedHandlerAddress)(void), uint8_t irq)
{
    if (irq != 9 && irq != 11)
    {
        kprintf("Only IRQ's 9 and 11 support sharing right now!\n");
        return;
    }

    if (irq == 9)
    {
        irq9handlers[irq9handlerCount++] = sharedHandlerAddress;

        Set_IDT_Entry((unsigned long)irq9_shared_interrupt_handler, HARDWARE_INTERRUPTS_BASE + 9);

        IRQ_Enable_Line(9);
    }
    else
    {
        irq11handlers[irq11handlerCount++] = sharedHandlerAddress;

        Set_IDT_Entry((unsigned long)irq11_shared_interrupt_handler, HARDWARE_INTERRUPTS_BASE + 11);

        IRQ_Enable_Line(11);
    }

}

void Interrupts_Init()
{
    // Initialize the PIC's and remap hardware interrupts 0 - 15 to 32 - 47
    PIC_remap((unsigned char)HARDWARE_INTERRUPTS_BASE, (unsigned char)(HARDWARE_INTERRUPTS_BASE + 8));

    // Start with every interrupt disabled
    uint8_t mask = (uint8_t)~0;
    outb(PIC1_DATA, mask);

    // Enable IRQ 0 for the timer
    IRQ_Enable_Line(0);

    // Enable IRQ 1, the keyboard handler
    IRQ_Enable_Line(1);

    // Initialize the interrupt descriptor table
    IDT_Init();
}

// TODO: using a pointer like this from user space is considered insecure
// NOTE, TODO: See printf_string_interrupt_handler for explanation of eflags and cs parameters
void _declspec(naked) print_string_interrupt_handler(int eflags, int cs, char *str)
{
    // supress warning about unused parameters
    (void)eflags, (void)cs;
    _asm
    {
        // prologue
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi

        pushad
    }

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("syscall interrupt handler fired.\n");

    // Write str to the terminal
    terminal_writestring(str);

    _asm
    {
        popad

        // epilogue
        pop edi
        pop esi
        pop ebx
        pop ebp

        iretd
    }
}

// TODO: Develop some mechanism to allow printf_ to return an int
// NOTE: Because this function is called as an interrupt handler, the parameters will not be on the stack where the compiler would expect them,
// because of the EFLAGS and CS registers that the int instruction pushes on the stack (I might have reversed the order of them). 
// Having the eflags and cs as explicit (but unused) parameters in the function definition is, IMHO, an elegant solution to this problem.
// TODO: This will need to be modified when we have a user-mode
void _declspec(naked) printf_interrupt_handler(int eflags, int cs, const char *fmt, va_list va)
{
    // suppress warnings that eflags and cs aren't used
    (void)eflags, (void)cs;

    _asm 
    {
        // prologue
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi

        pushad
    }

    // print the addresses of the fmt and va variables (to see if we know where to find them)
    /*if (debugLevel)
    {
        _asm
        {
            push fmt
            call terminal_print_ulong_hex
            call terminal_newline
            push va
            call terminal_print_ulong_hex
            call terminal_newline
        }
    }*/

    ++interrupts_fired;

    if (debugLevel)
        terminal_writestring("printf interrupt handler fired.\n");
    
    // Here's where printf actually happens
    if (guiCallback)
        GUI_printf(fmt, va);
    else
        vprintf_(fmt, va);
    
    _asm
    {
        popad
    
        // epilogue
        pop edi
        pop esi
        pop ebx
        pop ebp

        iretd
    }
}

void _declspec(naked) read_from_keyboard_handler(int eflags, int cs, uint16_t *pScanCode, bool *pRetVal)
{
    (void)eflags, (void)cs;

    __asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    *pRetVal = keyboard_read_from_queue(pScanCode) & 0xff;

    __asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) time_delay_ms_interrupt_handler(int eflags, int cs, uint32_t milliSeconds)
{
    (void)eflags, (void)cs;

    __asm
    {
        // prologue
        push ebp
        mov ebp, esp

        sti // enable interrupts so timer can fire
    }

    TimeDelayMS(milliSeconds);

    __asm
    {
        // epilogue
        pop ebp

        iretd
    }
}

void _declspec(naked) time_get_uptime_ms_handler(int eflags, int cs, uint32_t *pRetVal)
{
    (void)eflags, (void)cs;

    __asm
    {
        // prologue
        push ebp
        mov ebp, esp
    }

    *pRetVal = TimeGetUptimeMS();

    __asm
    {
        // epilogue
        pop ebp

        iretd
    }
}
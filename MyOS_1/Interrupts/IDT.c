#include "../Drivers/Keyboard.h"
#include "../Drivers/RTL_8139.h"
#include "IDT.h"
#include "Interrupts.h"
#include "System_Calls.h"
#include "../Timers/PIT.h"

IDT_ENTRY IDT[256];
IDT_ptr_struct IDT_ptr;

void Set_IDT_Entry(unsigned long IRQ_Handler, int IDT_Number)
{
    IDT[IDT_Number].offset_lowerbits = IRQ_Handler & 0xffff;
    IDT[IDT_Number].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    IDT[IDT_Number].zero = 0;
    IDT[IDT_Number].type_attr = 0x8e; /* INTERRUPT_GATE */
    IDT[IDT_Number].offset_higherbits = (IRQ_Handler & 0xffff0000) >> 16;
}

void IDT_Init(void)
{
    interrupts_fired = 0;

    // set exception handlers
    for (int i = 0; i < 32; ++i)
    {
        Set_IDT_Entry((unsigned long)default_exception_handler, i);
    }

    Set_IDT_Entry((unsigned long)invalid_opcode_handler, 6);
    Set_IDT_Entry((unsigned long)gpf_exception_handler, 13);
    Set_IDT_Entry((unsigned long)page_fault_handler, 14);

    // set default handlers
    for (int i = 32; i < 256; ++i)
    {
        Set_IDT_Entry((unsigned long)default_interrupt_handler, i);
    }

    // set timer handler
    Set_IDT_Entry((unsigned long)timer_interrupt_handler, TIMER_INTERRUPT);

    // set keyboard handler (interrupt 1 is remapped to 33, 0x21)
    Set_IDT_Entry((unsigned long)keyboard_interrupt_handler, KEYBOARD_INTERRUPT);

    // Set print string handler
    Set_IDT_Entry((unsigned long)print_string_interrupt_handler, SYSCALL_PRINT);

    // Set printf handler
    Set_IDT_Entry((unsigned long)printf_interrupt_handler, SYSCALL_PRINTF);

    // Set handler to get graphics info
    Set_IDT_Entry((unsigned long)get_graphics_interrupt_handler, SYSCALL_GET_GRAPHICS_INFO);

    // Set handler for allocating a new page of memory
    Set_IDT_Entry((unsigned long)page_allocator_interrupt_handler, SYSCALL_PAGE_ALLOCATOR);

    // Set handler for time delay
    Set_IDT_Entry((unsigned long)time_delay_ms_interrupt_handler, SYSCALL_TIME_DELAY_MS);

    // Set handler for graphics blit function
    Set_IDT_Entry((unsigned long)graphics_blit_interrupt_handler, SYSCALL_GRAPHICS_BLIT);

    // Set handler for fopen
    Set_IDT_Entry((unsigned long)fopen_interrupt_handler, SYSCALL_FOPEN);

    // Set handler for fclose
    Set_IDT_Entry((unsigned long)fclose_interrupt_handler, SYSCALL_FCLOSE);

    // Set handler for fread
    Set_IDT_Entry((unsigned long)fread_interrupt_handler, SYSCALL_FREAD);

    // Set handler for fseek
    Set_IDT_Entry((unsigned long)fseek_interrupt_handler, SYSCALL_FSEEK);

    // Set handler for ftell
    Set_IDT_Entry((unsigned long)ftell_interrupt_handler, SYSCALL_FTELL);

    // Set handler for exit
    Set_IDT_Entry((unsigned long)exit_interrupt_handler, SYSCALL_EXIT_APP);

    // Set handler for getting uptime in ms
    Set_IDT_Entry((unsigned long)time_get_uptime_ms_handler, SYSCALL_TIME_UPTIME_MS);

    // Set handler for reading from keyboard
    Set_IDT_Entry((unsigned long)read_from_keyboard_handler, SYSCALL_READ_FROM_KEYBOARD);

    // Set handler for dispatching a new task
    Set_IDT_Entry((unsigned long)dispatch_new_task_interrupt_handler, SYSCALL_DISPATCH_NEW_TASK);

    // Set handler for getting mouse state
    Set_IDT_Entry((unsigned long)get_mouse_state_interrupt_handler, SYSCALL_GET_MOUSE_STATE);

    // Set handler for hiding shell display elements
    Set_IDT_Entry((unsigned long)hide_shell_display_interrupt_handler, SYSCALL_HIDE_SHELL_DISPLAY);

    // Set handler for the GUI telling the kernel how to send the GUI messages
    Set_IDT_Entry((unsigned long)gui_register_callback_interrupt_handler, SYSCALL_REGISTER_GUI_CALLBACK);

    // Set handler for launching an app
    Set_IDT_Entry((unsigned long)launch_app_interrupt_handler, SYSCALL_LAUNCH_APP);

    /* fill the IDT descriptor */
    IDT_ptr.base = (uint32_t)IDT;
    IDT_ptr.size = (sizeof(IDT_ENTRY) * 256);
    /*idt_address = (unsigned long)IDT;
    idt_ptr[0] = (sizeof(IDT_ENTRY) * 256) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;*/

    // load the idt pointer and enable interrupts
    _asm
    {
        lidt IDT_ptr
        sti
    }
}
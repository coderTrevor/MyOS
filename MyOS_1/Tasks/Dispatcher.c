#include "Context.h"
#include "../printf.h"
#include "../Interrupts/System_Calls.h"
#include "../GUI_Kernel.h"

PROCESS_CONTROL_BLOCK tasks[64] = { 0 };
READY_QUEUE_ENTRY *readyQueueHead = NULL;

int currentTask = 0;   // 0 is for the main kernel thread
int ticksLeftInTask = TICKS_PER_TASK;
uint32_t nextPID = 1000;

// This must be global so we can access it while playing with the stack in ways the compiler can't predict
uint32_t stackPtr;

void DispatchNewTask(uint32_t programStart, uint32_t stackSize, const char *imageName, bool exclusive)
{
    // disable interrupts
    __asm cli

    if (debugLevel)
    {
        kprintf("Dispatching %s\n", imageName);
        kprintf("program starts at 0x%lX\n", programStart);
    }

    // Find a free task slot
    uint16_t taskSlot;
    for (taskSlot = 0; taskSlot < MAX_TASKS; ++taskSlot)
    {
        if (!tasks[taskSlot].inUse)
            break;
    }

    if (taskSlot >= MAX_TASKS)
    {
        kprintf("ERROR: Too many tasks are running to start %s\n", imageName);
        goto endOfDispatch;
    }

    // Allocate stack space (must be 16-byte aligned)    
    stackPtr = 0x400000;  //TODO: FIX://// (uint32_t)malloc(stackSize + 0x100);

    if (!stackPtr)
    {
        kprintf("ERROR: Couldn't allocate stack space for %s\n", imageName);
        goto endOfDispatch;
    }

    // ensure stackPtr is properly-aligned
    if (stackPtr % 0x100 != 0)
        stackPtr += (0x100 - stackPtr % 0x100);

    // Advance stackPtr to the end of the stack (the stack grows downward from the top of the memory region)
    stackPtr += stackSize + (stackSize * taskSlot);   // TEMPTEMP - Make sure multiple programs have different stack addresses

    if (debugLevel)
        kprintf("0x%lX\n", stackPtr);

    // Add task to PCB
    tasks[taskSlot].stackSize = stackSize;
    tasks[taskSlot].ESP = stackPtr;
    strncpy(tasks[taskSlot].imageName, imageName, MAX_IMAGE_NAME_LENGTH - 1);
    tasks[taskSlot].inUse = true;
    tasks[taskSlot].PID = nextPID++;

    // Create ready queue entry
    READY_QUEUE_ENTRY *queueEntry;
    queueEntry = malloc(sizeof(READY_QUEUE_ENTRY));
    // TODO: Exitting programs will need to be cleaned up somehow
    if (!queueEntry)
    {
        kprintf("ERROR: Couldn't allocate memory for ready queue entry!\n");
        goto endOfDispatch;
    }

    queueEntry->taskIndex = taskSlot;
    queueEntry->nextEntry = NULL;
    
    // Add queue entry to end of the ready queue
    if (!readyQueueHead)
    {
        readyQueueHead = queueEntry;
    }
    else
    {
        // Walk through list until we've found the final entry
        READY_QUEUE_ENTRY *finalEntry = readyQueueHead;
        while (finalEntry->nextEntry)
            finalEntry = finalEntry->nextEntry;

        finalEntry->nextEntry = queueEntry;
    }   

    // setup stack for the task, and copy the stack to stackPtr
    __asm
    {
        push esp
        push [stackPtr]
        int SYSCALL_DISPATCH_NEW_TASK   // call dispatch_new_task_interrupt_handler(eflags, cs, stackPtr, esp)
        add esp, 8
    }

    // Update the task's "stack image" to represent values appropriate for the new task
    TASK_STACK_LAYOUT *pNewTask = (TASK_STACK_LAYOUT *)((unsigned long)stackPtr - sizeof(TASK_STACK_LAYOUT));    
    pNewTask->returnAddress = programStart;
    pNewTask->registers[REG_INDEX_EBP] = (unsigned long)pNewTask + (sizeof(uint32_t) * 8);
    pNewTask->registers[REG_INDEX_ESP] = (unsigned long)pNewTask + (sizeof(uint32_t) * 8);
    pNewTask->prologueEbp = (unsigned long)stackPtr;

    tasks[taskSlot].ESP = (uint32_t)pNewTask;
    tasks[taskSlot].exclusive = exclusive;
    tasks[taskSlot].cr3 = __readcr3();  // TEMPTEMP: Give the new task the same page table as the kernel

    // If a GUI shell is running, tell it about the new process
    if (guiCallback)
    {
        GUI_CreateConsoleWindowForApp(taskSlot);
    }

    if (debugLevel)
    {
        terminal_writestring("eflags: ");
        terminal_print_ulong_hex(pNewTask->eflags);
        terminal_writestring("\ncs: ");
        terminal_print_ulong_hex(pNewTask->cs);

        terminal_writestring("\nReturn value: ");
        terminal_print_ulong_hex(pNewTask->returnAddress);
        terminal_newline();

        terminal_writestring("\nREG_INDEX_ESP: ");
        terminal_print_ulong_hex(pNewTask->registers[REG_INDEX_ESP]);
        terminal_newline();

        terminal_writestring("\nREG_INDEX_EBP: ");
        terminal_print_ulong_hex(pNewTask->registers[REG_INDEX_EBP]);
        terminal_newline();

        terminal_writestring("\nPrologue ebp: ");
        terminal_print_ulong_hex(pNewTask->prologueEbp);
        terminal_newline();

        terminal_dumpHexAround((uint8_t *)pNewTask, 16, sizeof(TASK_STACK_LAYOUT) + 16);
    }

endOfDispatch:
    // re-enable interrupts
    __asm sti
}
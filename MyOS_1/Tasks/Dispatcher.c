#include "Context.h"
#include "../printf.h"
#include "../Interrupts/System_Calls.h"
#include "../paging.h"
#include "../Networking/TFTP.h"
#include "../Executables/PE32.h"

PROCESS_CONTROL_BLOCK tasks[64] = { 0 };
READY_QUEUE_ENTRY *readyQueueHead = NULL;

int currentTask = 0;   // 0 is for the main kernel thread
int ticksLeftInTask = TICKS_PER_TASK;
uint32_t nextPID = 1000;

// This must be global so we can access it while playing with the stack in ways the compiler can't predict
uint32_t stackPtr;

void DispatchNewTask(uint32_t programStart, PAGE_DIRECTORY_ENTRY *newPageDirectory, uint32_t stackSize, const char *imageName, bool exclusive)
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
    tasks[taskSlot].cr3 = (uint32_t)newPageDirectory;

    // Create ready queue entry
    READY_QUEUE_ENTRY *queueEntry;
    queueEntry = dbg_alloc(sizeof(READY_QUEUE_ENTRY));
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

// TODO: Handle multiple error codes
// returns true on success
bool LaunchApp(const char *appName, int exclusive, uint32_t exeLocation)
{
    // Get the size of the executable file
    uint32_t fileSize;
    bool succeeded = true;

    // we'll need to re-enable interrupts if they were disabled
    _enable();

    if (!TFTP_GetFileSize(tftpServerIP, appName, &fileSize))
    {
        printf("Failed to determine size of %s\n", appName);
        return false;
    }
    
    // Allocate a buffer for the executable file
    uint8_t *peBuffer = dbg_alloc(fileSize);
    if (!peBuffer)
    {
        printf("Not enough memory to open %s\n", appName);
        return false;
    }

    // Download the executable
    if (!TFTP_GetFile(tftpServerIP, appName, peBuffer, fileSize, NULL))
    {
        printf("Error reading %s from server!\n", appName);
        return false;
    }

    // Run the executable
    if (!loadAndRunPE((uint8_t*)exeLocation, (DOS_Header*)peBuffer, appName, exclusive))
    {
        printf("Error running %s\n", appName);
        succeeded = false;
    }

    dbg_release(peBuffer);

    return succeeded;    
}

// This is where a running program will "return" to. We can also call it with the exit system call
void ExitApp()
{
    // Make sure interrupts are disabled
    _disable();

    // TODO: handle multiEnable = false
    // If the ready-queue is empty or contains only this task, 
    // or if we're not running in multitasking mode, there's nothing we can do here
    if (!readyQueueHead || (!readyQueueHead->nextEntry && readyQueueHead->taskIndex == currentTask))
    {
        kprintf("ExitApp called on only running app. System halted");
        for (;;)
            __halt();
    }
        
    // Allow the task index to be reused
    tasks[currentTask].inUse = false;
    
    // TODO: Free all memory associated with this task
    // TODO: Free the stack or mark it as reusable (not sure how to do this right now)
    
    // Make sure this task can be swapped out ASAP
    tasks[currentTask].exclusive = false;
    ticksLeftInTask = 0;

    printf("\n%s has exited.\n", tasks[currentTask].imageName);

    // TODO: Tell the GUI the program has quit

    // Enable interrupts
    _enable();

    // TODO: Is there a better way to switch contexts from here?
    // Now we'll just wait for the timer interrupt to fire and this task will never be revisitted
    for (;;)
        __halt();
}
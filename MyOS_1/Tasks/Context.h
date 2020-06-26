#pragma once
#include "../misc.h"
#include "../printf.h"
#include <stdint.h>

// TODO: Make less ghetto
#define MAX_TASKS               64
#define TICKS_PER_TASK          1
#define MAX_IMAGE_NAME_LENGTH   32

typedef struct CONTEXT_INFO
{
    /*uint32_t EBP;
    uint32_t EBX;
    uint32_t EDI;
    uint32_t ESI;
    uint32_t EIP;*/
    uint32_t ESP;   // All the other registers are saved and restored with pusha / popa

    uint32_t stackSize;
    char imageName[MAX_IMAGE_NAME_LENGTH];
    bool inUse;
    bool exclusive; // True to disable switching to any other tasks
    uint32_t PID;
    // ? What else?
} CONTEXT_INFO, PROCESS_CONTROL_BLOCK;

struct READY_QUEUE_ENTRY;
typedef struct READY_QUEUE_ENTRY
{
    uint32_t taskIndex;
    void *nextEntry;
} READY_QUEUE_ENTRY;

// this is the layout of a task's stack after an interrupt has fired and the prologue of the interrupt has happened
typedef struct TASK_STACK_LAYOUT
{
    uint32_t registers[8];  // My interrupt will execute pushad to push all regs onto the stack
    uint32_t prologueEbp;   // prologue of interrupt handler will push the old value of ebp onto the stack
    uint32_t returnAddress; // Where execution will "resume" after the iretd is executed at the end of the interrupt
    // cs and eflags names may be swapped at various points in my code. I ignore their actual values so I've yet to research which one is which.
    uint32_t cs;            // cs that calling the interrupt pushes onto the stack
    uint32_t eflags;        // calling the interrupt pushes this onto the stack
} TASK_STACK_LAYOUT;

// Indices into TASK_STRUCT_LAYOUT.registers
#define REG_INDEX_ESP 3
#define REG_INDEX_EBP 2

extern READY_QUEUE_ENTRY *readyQueueHead;
extern PROCESS_CONTROL_BLOCK tasks[MAX_TASKS];
//extern READY_QUEUE_ENTRY readyQueuePool[MAX_TASKS];
extern int ticksLeftInTask;
extern int currentTask;

extern uint32_t nextPID;

extern bool multiEnable; // TEMPTEMP

void DispatchNewTask(uint32_t programStart, uint32_t stackSize, const char *imageName, bool exclusive);

void SwitchTask(uint32_t taskIndex);
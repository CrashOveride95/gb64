
#ifndef _CPU_H
#define _CPU_H

#include "memory_map.h"

enum STOP_REASON {
    STOP_REASON_NONE,
    STOP_REASON_STOP,
    STOP_REASON_HALT,
    STOP_REASON_INTERRUPT_RET,
    STOP_REASON_ERROR,
};

enum GB_FLAGS {
    GB_FLAGS_Z = 0x80,
    GB_FLAGS_N = 0x40,
    GB_FLAGS_H = 0x20,
    GB_FLAGS_C = 0x10,
};

enum GB_INTERRUPTS {
    GB_INTERRUPTS_V_BLANK   = 0x01,
    GB_INTERRUPTS_LCDC      = 0x02,
    GB_INTERRUPTS_TIMER     = 0x04,
    GB_INTERRUPTS_SERIAL    = 0x08,
    GB_INTERRUPTS_INPUT     = 0x10,
    GB_INTERRUPTS_ENABLED   = 0x80,
};

struct CPUState {
    unsigned char a;
    unsigned char f;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char h;
    unsigned char l;
    unsigned short sp;
    unsigned short pc;
    unsigned char stopReason;
    unsigned char interrupts;
    unsigned char nextInterrupt;
    unsigned char unused1;
    unsigned long cyclesRun;
    unsigned long nextTimerTrigger;
};

extern int runCPUCPU(struct CPUState* state, struct Memory* memory, int cyclesToRun);

extern void initializeCPU(struct CPUState* state);

#endif
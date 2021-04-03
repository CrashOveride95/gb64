#include "cpu_test.h"
#include "../gameboy.h"
#include "../../memory.h"

int testInt(
    char *testName,
    char *testOutput,
    int actual,
    int expected
) {
    if (actual != expected)
    {
        sprintf(testOutput, "Failed %s:\n E %d A %d", testName, expected, actual);
        return 0;
    }

    return 1;
}

int testCPUState(
    char *testName,
    char *testOutput,
    struct CPUState* actual,
    struct CPUState* expected
) {
    if (actual->a != expected->a || actual->f != expected->f || actual->b != expected->b || actual->c != expected->c ||
        actual->d != expected->d || actual->e != expected->e || actual->h != expected->h || actual->l != expected->l ||
        actual->pc != expected->pc || actual->sp != expected->sp || actual->stopReason != expected->stopReason ||
        actual->interrupts != expected->interrupts)
    {
        sprintf(testOutput, 
            "Failed cpu: %s\n"
            "   A  F  B  C  D  E  H  L  PC   SP\n"
            " E %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X\n"
            " A %02X %02X %02X %02X %02X %02X %02X %02X %04X %04X\n"
            " ASR %d ESR %d AI %X EI %X",
            testName, 
            expected->a, expected->f, expected->b, expected->c, 
            expected->d, expected->e, expected->h, expected->l, 
            expected->pc, expected->sp,
            actual->a, actual->f, actual->b, actual->c, 
            actual->d, actual->e, actual->h, actual->l, 
            actual->pc, actual->sp,
            actual->stopReason, expected->stopReason,
            actual->interrupts, expected->interrupts
        );

        return 0;
    }

    return 1;
}

char* registerNames[] = {
    "B",
    "C",
    "D",
    "E",
    "H",
    "L",
    "HL",
    "A",
    "d8",
};

int registerOffset[] = {
    offsetof(struct CPUState, b),
    offsetof(struct CPUState, c),
    offsetof(struct CPUState, d),
    offsetof(struct CPUState, e),
    offsetof(struct CPUState, h),
    offsetof(struct CPUState, l),
    0,
    offsetof(struct CPUState, a),
    0,
};

unsigned char* getRegisterPointer(struct CPUState* cpu, unsigned char* hlTarget, unsigned char* d8Target, int registerIndex)
{
    if (registerIndex == HL_REGISTER_INDEX)
    {
        return hlTarget;
    }
    else if (registerIndex == d8_REGISTER_INDEX)
    {
        return d8Target;
    }
    else
    {
        return (unsigned char*)cpu + registerOffset[registerIndex];
    }
}

void BankSwitchingWriter(struct Memory* memory, int addr, int value)
{
    
}

int runCPUTests(char* testOutput) {
    struct CPUState cpu;
    int i;
    char subTestOutput[200];

    zeroMemory(&gGameboy.memory, sizeof(struct Memory));

    for (i = 0; i < MEMORY_MAP_SIZE; ++i)
    {
        setMemoryBank(&gGameboy.memory, i, gGameboy.memory.internalRam, 0, 0);
    }

    gGameboy.memory.bankSwitch = &BankSwitchingWriter;

    initializeCPU(&cpu);

    if (
        !run0x0Tests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0x1Tests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0x2Tests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0x3Tests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0x4_7Tests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0x8_9Tests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0xA_BTests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0xCTests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0xDTests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0xETests(&cpu, &gGameboy.memory, subTestOutput) ||
        !run0xFTests(&cpu, &gGameboy.memory, subTestOutput) ||
        !runRegisterTests(&cpu, &gGameboy.memory, subTestOutput) ||
        !runInterruptTests(&cpu, &gGameboy.memory, subTestOutput) ||
        0)
    {
        sprintf(testOutput, "runCPU 0x%X\n%s", &runCPU, subTestOutput);
        return 0;
    }

    return 1;
}
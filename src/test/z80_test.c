#include "z80_test.h"
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

int testZ80State(
    char *testName,
    char *testOutput,
    struct Z80State* actual,
    struct Z80State* expected
) {
    if (actual->a != expected->a || actual->f != expected->f || actual->b != expected->b || actual->c != expected->c ||
        actual->d != expected->d || actual->e != expected->e || actual->h != expected->h || actual->l != expected->l ||
        actual->pc != expected->pc || actual->sp != expected->sp || actual->stopReason != expected->stopReason ||
        actual->interrupts != expected->interrupts)
    {
        sprintf(testOutput, 
            "Failed z80: %s\n"
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
    offsetof(struct Z80State, b),
    offsetof(struct Z80State, c),
    offsetof(struct Z80State, d),
    offsetof(struct Z80State, e),
    offsetof(struct Z80State, h),
    offsetof(struct Z80State, l),
    0,
    offsetof(struct Z80State, a),
    0,
};

unsigned char* getRegisterPointer(struct Z80State* z80, unsigned char* hlTarget, unsigned char* d8Target, int registerIndex)
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
        return (unsigned char*)z80 + registerOffset[registerIndex];
    }
}

void DefaultRegisterWriter(struct Memory* memory, int addr, unsigned char value)
{
    if (addr >= MISC_START && addr < 0x10000)
    {
        memory->miscBytes[addr - MISC_START] = value;
    }
}

void BankSwitchingWriter(struct Memory* memory, int addr, unsigned char value)
{
    
}

int runTests(char* testOutput) {
    struct Z80State z80;
    int i;
    char subTestOutput[200];

    zeroMemory(&gGameboy.memory, sizeof(struct Memory));

    for (i = 0; i < MEMORY_MAP_SIZE; ++i)
    {
        gGameboy.memory.memoryMap[i] = gGameboy.memory.internalRam;
    }

    for (i = 0; i < REGISTER_WRITER_COUNT; ++i)
    {
        gGameboy.memory.registerWriters[i] = &DefaultRegisterWriter;
    }

    gGameboy.memory.bankSwitch = &BankSwitchingWriter;

    initializeZ80(&z80);

    if (
        !run0x0Tests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0x1Tests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0x2Tests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0x3Tests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0x4_7Tests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0x8_9Tests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0xA_BTests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0xCTests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0xDTests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0xETests(&z80, &gGameboy.memory, subTestOutput) ||
        !run0xFTests(&z80, &gGameboy.memory, subTestOutput) ||
        !runRegisterTests(&z80, &gGameboy.memory, subTestOutput) ||
        !runInterruptTests(&z80, &gGameboy.memory, subTestOutput) ||
        0)
    {
        sprintf(testOutput, "runZ80CPU 0x%X\n%s", &runZ80CPU, subTestOutput);
        return 0;
    }

	sprintf(testOutput, "Tests Passed %X", &runZ80CPU);

    return 1;
}

#include "rspppu.h"
#include "rspppu_includes.h"
#include "graphics.h"

#include <ultra64.h>

extern long long int ppuTextStart[];
extern long long int ppuTextEnd[];

extern long long int ppuDataStart[];
extern long long int ppuDataEnd[];

static char __attribute__((aligned(8))) outputBuffer[64];

extern OSMesgQueue     dmaMessageQ;
extern OSMesg          dmaMessageBuf;
extern OSPiHandle	   *handler;
extern OSIoMesg        dmaIOMessageBuf;

struct PPUPerformance __attribute__((aligned(8))) gPPUPerformance;

struct PPUTask
{
    u8* output;
    struct Memory* memorySource;
    struct GraphicsMemory* graphics;
    short flags;
    char lcdc;
    char ly;
    char scy;
    char scx;
    char wy;
    char wx;
    struct PPUPerformance* performanceMetrics;
};

static struct PPUTask __attribute__((aligned(8))) gPPUTask;
extern u8 __attribute__((aligned(8))) gScreenBuffer[];

void setupPPU()
{

}

void startPPUFrame(struct Memory* memory, int gbc)
{
    OSTask task;

    gPPUTask.output = (u8*)K0_TO_PHYS(gScreenBuffer);
    gPPUTask.memorySource = (struct Memory*)K0_TO_PHYS(memory);
    gPPUTask.graphics = (struct GraphicsMemory*)K0_TO_PHYS(&memory->vram);
    gPPUTask.flags = 0;
    gPPUTask.lcdc = READ_REGISTER_DIRECT(memory, REG_LCDC);
    gPPUTask.ly = READ_REGISTER_DIRECT(memory, REG_LY);
    gPPUTask.scy = READ_REGISTER_DIRECT(memory, REG_SCY);
    gPPUTask.scx = READ_REGISTER_DIRECT(memory, REG_SCX);
    gPPUTask.wy = READ_REGISTER_DIRECT(memory, REG_WY);
    gPPUTask.wx = READ_REGISTER_DIRECT(memory, REG_WX);
    gPPUTask.performanceMetrics = (struct PPUPerformance*)K0_TO_PHYS(&gPPUPerformance);

    if (gbc) {
        gPPUTask.flags |= PPU_TASK_FLAGS_COLOR;
    }

    task.t.type = M_GFXTASK;
    task.t.flags = OS_TASK_SP_ONLY;
    task.t.ucode_boot = (u64*)ppuTextStart;
    task.t.ucode_boot_size = (char*)ppuTextEnd - (char*)ppuTextStart;
    task.t.ucode = NULL;
    task.t.ucode_size = 0;
    task.t.ucode_data = (u64*)ppuDataStart;
    task.t.ucode_data_size = (char*)ppuDataEnd - (char*)ppuDataStart;
    task.t.dram_stack = 0;
    task.t.dram_stack_size = 0;
    task.t.output_buff = (u64*)outputBuffer;
    task.t.output_buff_size = 0;
    task.t.data_ptr = (u64*)&gPPUTask;
    task.t.data_size = sizeof(struct PPUTask);
    task.t.yield_data_ptr = 0;
    task.t.yield_data_size = 0;
    
    osWritebackDCache(&gPPUTask, sizeof(struct PPUTask));
    osWritebackDCache(&memory->misc.sprites, sizeof(memory->misc.sprites));

    osSpTaskStart(&task);
    // clear mode 3 bit
    IO_WRITE(SP_STATUS_REG, SP_CLR_SIG0 | SP_SET_SIG1 | SP_CLR_SIG2);
}

void renderPPURow(struct Memory* memory, struct GraphicsState* state)
{
    gPPUTask.lcdc = READ_REGISTER_DIRECT(memory, REG_LCDC);
    gPPUTask.ly = READ_REGISTER_DIRECT(memory, REG_LY);
    gPPUTask.scy = READ_REGISTER_DIRECT(memory, REG_SCY);
    gPPUTask.scx = READ_REGISTER_DIRECT(memory, REG_SCX);
    gPPUTask.wy = READ_REGISTER_DIRECT(memory, REG_WY);
    gPPUTask.wx = READ_REGISTER_DIRECT(memory, REG_WX);
    osWritebackDCache(&gPPUTask, sizeof(struct PPUTask));
    osWritebackDCache(&memory->vram, sizeof(&memory->vram));

    prepareGraphicsPallete(state);

    if (state->row - state->lastRenderedRow >= GB_RENDER_STRIP_HEIGHT)
    {
        renderScreenBlock(state);
    }

    // set mode 3 bit
    IO_WRITE(SP_STATUS_REG, SP_SET_SIG0);
}

void enterMode2(struct Memory* memory) {
    osWritebackDCache(&memory->misc.sprites, sizeof(memory->misc.sprites));
}
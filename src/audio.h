
#ifndef _AUDIO_H
#define _AUDIO_H
#include <ultra64.h>

#define AUDIO_BUFFER_COUNT      4
#define APU_TICKS_PER_SEC       256
#define APU_TICKS_PER_SEC_L2    8
#define CYCLES_PER_TICK         4096
#define CYCLES_PER_TICK_L2      12

#define RESTART_BIT             0x80

struct Memory;

#define GET_WAVE_DUTY(length_pattern) (((length_pattern) & 0xC0) >> 6)
#define GET_SOUND_LENGTH(length_pattern) (64 - ((length_pattern) & 0x3F))

#define GET_SWEEP_TIME(sweep) (((sweep) >> 4) & 0x7)
#define GET_SWEEP_DIR(sweep) (((sweep) >> 3) & 0x1)
#define GET_SWEEP_SHIFT(sweep) ((sweep) & 0x7)

#define GET_ENVELOPE_VOLUME(volume) (((volume) >> 4) & 0xF)
#define GET_ENVELOPE_DIR(volume) (((volume) >> 3) & 0x1)
#define GET_ENVELOPE_STEP_DURATION(volume) ((volume) & 0x7)
#define GET_SOUND_FREQ(frequencyHi, frequencyLo) ((((int)(frequencyHi) & 0x7) << 8) | (int)(frequencyLo))
#define GET_SOUND_LENGTH_LIMITED(frequencyHi) (((frequencyHi) & 0x40) >> 6)
#define GET_PCM_VOLUME(volume) (((volume) >> 5) & 0x3)

#define SOUND_LENGTH_INDEFINITE     ~0

#define TICK_LENGTH(rvalue) if ((rvalue) != SOUND_LENGTH_INDEFINITE) --rvalue

#define NOISE_MAX_CLOCK_SHIFT 13

struct AudioSample
{
    short l;
    short r;
};

struct AudioSweep {
    u8 stepDir;
    u8 stepShift;
    u8 stepDuration;
    u8 stepTimer;
};

struct AudioEnvelope {
    u8 volume;
    u8 step;
    u8 stepDuration;
    u8 stepTimer;
};

struct SquareWaveSound {
    u16 cycle;
    u16 waveDuty;
    u16 frequency;
    struct AudioSweep sweep;
    struct AudioEnvelope envelope;
    u16 length;
};

struct PCMSound {
    u16 cycle;
    u16 volume;
    u16 frequency;
    u16 length;
};

enum LFSRWidth {
    LFSRWidth15,
    LFSRWidth7
};

struct NoiseSound {
    struct AudioEnvelope envelope;
    u16 length;
    u16 lfsr;
    // fixed point numbers 8:24
    u32 accumulator;
    u32 sampleStep;
    enum LFSRWidth lfsrWidth;
};

enum SoundIndex {
    SoundIndexSquare1,
    SoundIndexSquare2,
    SoundIndexPCM,
    SoundIndexNoise,
};

struct AudioRenderState
{
    struct SquareWaveSound sound1;
    struct SquareWaveSound sound2;
    struct PCMSound pcmSound;
    struct NoiseSound noiseSound;
    u32 cyclesEmulated;
    u32 nextTickCycle;
};

struct AudioState
{
    struct AudioSample* buffers[AUDIO_BUFFER_COUNT];
    u16 sampleRate;
    u16 samplesPerBuffer;
    u16 currentWriteBuffer;
    u16 currentSampleIndex;
    u16 nextPlayBuffer;
    u16 freqLimit;
    int tickAdjustment;
};

extern struct AudioState gAudioState;

void initAudio(struct AudioState* audioState, int sampleRate, int frameRate);
void tickAudio(struct Memory* memoryMap, int untilCyles);
void finishAudioFrame(struct Memory* memoryMap);
void updateOnOffRegister(struct Memory* memoryMap);
u32 getAudioWriteHeadLead(struct AudioState* audioState);

void adjustCyclesEmulator(struct AudioRenderState* audioState, u32 by);

#endif
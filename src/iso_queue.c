#include "iso_queue.h"

#include "common.h"

int AllocdBuffersCount = 0;
int NextBuffer = 0;
int AllocdStrBuffersCount = 0;
int NextStrBuffer = 0;

#define LOOP_END 1
#define LOOP_REPEAT 2
#define LOOP_START 4

// clang-format off
// Empty ADPCM block with loop flags
char VAG_SilentLoop[48] = {
    0x0, LOOP_START | LOOP_REPEAT, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, LOOP_REPEAT,              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, LOOP_END | LOOP_REPEAT,   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};
// clang-format on

int vag_cmd_used = 0;
int vag_cmd_cnt = 0;
int max_vag_cmd_cnt = 0;

struct PriStackEntry gPriStack[4];
int VagCmdsPriCounter[11];

INCLUDE_ASM("asm/nonmatchings/iso_queue", InitBuffers);

INCLUDE_ASM("asm/nonmatchings/iso_queue", AllocDataBuffer);

INCLUDE_ASM("asm/nonmatchings/iso_queue", AllocateBuffer);

INCLUDE_ASM("asm/nonmatchings/iso_queue", FreeBuffer);

INCLUDE_ASM("asm/nonmatchings/iso_queue", ReleaseMessage);

INCLUDE_ASM("asm/nonmatchings/iso_queue", AllocIsoPages);

INCLUDE_ASM("asm/nonmatchings/iso_queue", FreeIsoPages);

INCLUDE_ASM("asm/nonmatchings/iso_queue", QueueMessage);

INCLUDE_ASM("asm/nonmatchings/iso_queue", UnqueueMessage);

INCLUDE_ASM("asm/nonmatchings/iso_queue", GetMessage);

INCLUDE_ASM("asm/nonmatchings/iso_queue", ProcessMessageData);

INCLUDE_ASM("asm/nonmatchings/iso_queue", ReturnMessage);

INCLUDE_ASM("asm/nonmatchings/iso_queue", GetVAGCommand);

INCLUDE_ASM("asm/nonmatchings/iso_queue", FreeVAGCommand);

INCLUDE_ASM("asm/nonmatchings/iso_queue", CheckForIsoPageBoundaryCrossing);

INCLUDE_ASM("asm/nonmatchings/iso_queue", FreeDataBuffer);

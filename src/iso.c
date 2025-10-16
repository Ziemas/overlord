#include "common.h"

INCLUDE_RODATA("asm/nonmatchings/iso", D_000127C0);

INCLUDE_ASM("asm/nonmatchings/iso", InitISOFS);

INCLUDE_ASM("asm/nonmatchings/iso", IsoQueueVagStream);

INCLUDE_ASM("asm/nonmatchings/iso", IsoPlayVagStream);

INCLUDE_ASM("asm/nonmatchings/iso", ISOThread);

INCLUDE_ASM("asm/nonmatchings/iso", RunDGOStateMachine);

INCLUDE_ASM("asm/nonmatchings/iso", LoadDGO);

INCLUDE_ASM("asm/nonmatchings/iso", CopyData);

INCLUDE_ASM("asm/nonmatchings/iso", FindISOFile);

INCLUDE_ASM("asm/nonmatchings/iso", FindVAGFile);

INCLUDE_ASM("asm/nonmatchings/iso", GetISOFileLength);

INCLUDE_ASM("asm/nonmatchings/iso", NullCallback);

INCLUDE_ASM("asm/nonmatchings/iso", IsoStopVagStream);

INCLUDE_ASM("asm/nonmatchings/iso", CopyDataToIOP);

INCLUDE_ASM("asm/nonmatchings/iso", CopyDataToEE);

INCLUDE_ASM("asm/nonmatchings/iso", DGOThread);

INCLUDE_ASM("asm/nonmatchings/iso", RPC_DGO);

INCLUDE_ASM("asm/nonmatchings/iso", LoadNextDGO);

INCLUDE_ASM("asm/nonmatchings/iso", CancelDGO);

INCLUDE_ASM("asm/nonmatchings/iso", InitDriver);

INCLUDE_ASM("asm/nonmatchings/iso", SetVagClock);

INCLUDE_RODATA("asm/nonmatchings/iso", D_000129E0);

INCLUDE_RODATA("asm/nonmatchings/iso", D_00012A30);

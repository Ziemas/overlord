#include "common.h"

INCLUDE_ASM("asm/nonmatchings/streamlfo", SineLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitSineLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitRandLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitStreamLfoHandler);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RandomLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RemoveLfoStreamFromList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", CheckLfoList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", UpdateLfoVars);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RandomLfoSetPitchVars);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RandomLfoWaitForPitch);

INCLUDE_ASM("asm/nonmatchings/streamlfo", SineLfoSetPitchVars);

INCLUDE_ASM("asm/nonmatchings/streamlfo", SineLfoWaitForPitch);

INCLUDE_ASM("asm/nonmatchings/streamlfo", StreamLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitStreamLfoList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", AddToCircularLfoStreamList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", FindLfoStreamInList);

INCLUDE_RODATA("asm/nonmatchings/streamlfo", D_00012F70);

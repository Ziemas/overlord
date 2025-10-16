#include "common.h"

INCLUDE_RODATA("asm/nonmatchings/ssound", D_00012CC0);

INCLUDE_ASM("asm/nonmatchings/ssound", InitSound);

INCLUDE_ASM("asm/nonmatchings/ssound", AllocateSound);

INCLUDE_ASM("asm/nonmatchings/ssound", CalculateFalloffVolume);

INCLUDE_ASM("asm/nonmatchings/ssound", CalculateAngle);

INCLUDE_ASM("asm/nonmatchings/ssound", SetEarTrans);

INCLUDE_ASM("asm/nonmatchings/ssound", SndMemAlloc);

INCLUDE_ASM("asm/nonmatchings/ssound", LookupSound);

INCLUDE_ASM("asm/nonmatchings/ssound", CleanSounds);

INCLUDE_ASM("asm/nonmatchings/ssound", UpdateVolume);

INCLUDE_ASM("asm/nonmatchings/ssound", GetVolume);

INCLUDE_ASM("asm/nonmatchings/ssound", GetPan);

INCLUDE_ASM("asm/nonmatchings/ssound", KillSoundsInGroup);

INCLUDE_ASM("asm/nonmatchings/ssound", SetCurve);

INCLUDE_ASM("asm/nonmatchings/ssound", SetMusicVol);

INCLUDE_ASM("asm/nonmatchings/ssound", SetBufferMem);

INCLUDE_ASM("asm/nonmatchings/ssound", ReleaseBufferMem);

INCLUDE_ASM("asm/nonmatchings/ssound", SndMemFree);

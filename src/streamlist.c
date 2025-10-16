#include "common.h"

INCLUDE_RODATA("asm/nonmatchings/streamlist", D_00012F80);

INCLUDE_ASM("asm/nonmatchings/streamlist", InsertVagStreamInList);

INCLUDE_ASM("asm/nonmatchings/streamlist", QueueNewStreamsFromList);

INCLUDE_ASM("asm/nonmatchings/streamlist", CheckPlayList);

INCLUDE_ASM("asm/nonmatchings/streamlist", StreamListThread);

INCLUDE_ASM("asm/nonmatchings/streamlist", InitVagStreamList);

INCLUDE_ASM("asm/nonmatchings/streamlist", FindVagStreamInList);

INCLUDE_ASM("asm/nonmatchings/streamlist", GetVagStreamInList);

INCLUDE_ASM("asm/nonmatchings/streamlist", RemoveVagStreamFromList);

INCLUDE_ASM("asm/nonmatchings/streamlist", EmptyVagStreamList);

INCLUDE_ASM("asm/nonmatchings/streamlist", MergeVagStreamLists);

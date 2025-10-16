#include "common.h"

int gNumFiles = 0;
int gMemUsed = 0;

INCLUDE_ASM("asm/nonmatchings/ramdisk", InitRamdisk);

INCLUDE_ASM("asm/nonmatchings/ramdisk", Thread_Server);

INCLUDE_ASM("asm/nonmatchings/ramdisk", RPC_Ramdisk);

#include "common.h"

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

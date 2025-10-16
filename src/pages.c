#include "common.h"

INCLUDE_RODATA("asm/nonmatchings/pages", D_00013110);

INCLUDE_ASM("asm/nonmatchings/pages", InitPagedMemory);

INCLUDE_ASM("asm/nonmatchings/pages", AllocPagesBytes);

INCLUDE_ASM("asm/nonmatchings/pages", AllocPages);

INCLUDE_ASM("asm/nonmatchings/pages", FreePagesList);

INCLUDE_ASM("asm/nonmatchings/pages", StepTopPage);

INCLUDE_ASM("asm/nonmatchings/pages", FromPagesCopy);

#include "common.h"

INCLUDE_RODATA("asm/nonmatchings/sbank", D_00012C80);

INCLUDE_ASM("asm/nonmatchings/sbank", InitBanks);

INCLUDE_ASM("asm/nonmatchings/sbank", AllocateBankName);

INCLUDE_ASM("asm/nonmatchings/sbank", LookupBank);

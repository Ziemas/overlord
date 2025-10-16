#include "common.h"

char gCommonBank[2048];
char gGunBank[2048];
char gBoardBank[2048];
char gLevelBanks[3][2048];

struct BankRecord *gBanks[6] = {(struct BankRecord *)gCommonBank,    (struct BankRecord *)gGunBank,
                                (struct BankRecord *)gBoardBank,     (struct BankRecord *)gLevelBanks[0],
                                (struct BankRecord *)gLevelBanks[1], (struct BankRecord *)gLevelBanks[2]};

INCLUDE_RODATA("asm/nonmatchings/sbank", D_00012C80);

INCLUDE_ASM("asm/nonmatchings/sbank", InitBanks);

INCLUDE_ASM("asm/nonmatchings/sbank", AllocateBankName);

INCLUDE_ASM("asm/nonmatchings/sbank", LookupBank);

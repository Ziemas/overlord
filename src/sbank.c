#include "sbank.h"

#include "common.h"

#include <string.h>

char gCommonBank[2048];
char gGunBank[2048];
char gBoardBank[2048];
char gLevelBanks[3][2048];

struct BankRecord *gBanks[6] = {(struct BankRecord *)gCommonBank,    (struct BankRecord *)gGunBank,
                                (struct BankRecord *)gBoardBank,     (struct BankRecord *)gLevelBanks[0],
                                (struct BankRecord *)gLevelBanks[1], (struct BankRecord *)gLevelBanks[2]};

void InitBanks() {
    int i;

    for (i = 0; i < 6; i++) {
        gBanks[i]->bank_handle = 0;
        gBanks[i]->used = 0;
        gBanks[i]->unk30 = 0;
    }

    strncpy(gBanks[0]->slot_name, "common", 16);
    gBanks[0]->spu_loc = 0x20000;
    gBanks[0]->spu_size = 0xAFCC0;
    strncpy(gBanks[1]->slot_name, "gun", 16);
    gBanks[1]->spu_loc = 0x131740;
    gBanks[1]->spu_size = 0;
    strncpy(gBanks[2]->slot_name, "board", 16);
    gBanks[2]->spu_loc = 0x131740;
    gBanks[2]->spu_size = 0;
    strncpy(gBanks[3]->slot_name, "level0", 16);
    gBanks[3]->spu_loc = 0x131740;
    gBanks[3]->spu_size = 0x42800;
    strncpy(gBanks[4]->slot_name, "level1", 16);
    gBanks[4]->spu_loc = 0x173F40;
    gBanks[4]->spu_size = 0x42800;
    strncpy(gBanks[5]->slot_name, "level2", 16);
    gBanks[5]->spu_loc = 0x1B6740;
    gBanks[5]->spu_size = 0x42800;
}

struct BankRecord *AllocateBankName(const char *name) {
    struct BankRecord **bank, *found = NULL;
    int i;

    if ((!strncmp(name, "common", 16) || !strncmp(name, "commonj", 16)) && !gBanks[0]->used) {
        found = gBanks[0];
    }

    if (!found) {
        i = 3;
        bank = &gBanks[3];
        while (i < 6) {
            if (!(*bank)->used) {
                found = (*bank);
                (*bank)->bank_handle = 0;
                (*bank)->unk30 = 0;
                break;
            }
            bank++;
            i++;
        }
    }

    return found;
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/sbank", LookupBank);
#else
#endif

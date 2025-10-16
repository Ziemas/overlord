#ifndef SBANK_H_
#define SBANK_H_

#include "989snd.h"
#include "common.h"

struct BankRecord {
    char name[16];
    u32 unk10[4];

    u32 spu_loc;
    u32 spu_size;

    SoundBankPtr bank_handle;

    bool used;
    u32 unk30;
    u32 unk34[7];
};

struct BankRecord *AllocateBankName(const char *name);
struct BankRecord *LookupBank(const char *name);

#endif // SBANK_H_

#ifndef ISO_API_H_
#define ISO_API_H_

#include "common.h"
#include "sbank.h"


void UnLoadMusic(s32 *handle);
void LoadMusic(char *name, s32 *handle);
void LoadSoundBank(const char *name, struct BankRecord *rec);

#endif // ISO_API_H_

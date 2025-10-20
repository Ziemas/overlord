#ifndef STREAMLFO_H_
#define STREAMLFO_H_

#include "list.h"

struct LfoListData {
    struct ListNode l;
    int unk0xc;
    int unk0x10;
    int unk0x14;
    int unk0x18;
    int unk0x1c;
    int unk0x20;
    int unk0x24;
    int unk0x28;
};

extern struct List LfoList;

void RemoveLfoStreamFromList(struct LfoListData *data, struct List *list);

#endif // STREAMLFO_H_

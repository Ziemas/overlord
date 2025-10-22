#ifndef LIST_H_
#define LIST_H_

#include "common.h"

struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    int in_use;
};

struct List {
    char name[8];
    char unk[4];
    int sema;
    int maybe_any_in_use;
    u32 elt_count;
    int unk2_init0;
    int unk0x1c;

    struct ListNode *list;
    u8 *buffer;
};

void InitList(struct List *head, u32 elt_count, int elt_size, const char* name);

#endif // LIST_H_

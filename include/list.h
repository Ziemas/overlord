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
    int sema;             // 12
    int maybe_any_in_use; // 16
    int elt_count;        // 20
    int unk2_init0;       // 24

    struct ListNode *list;
    u8 *buffer;
};

bool InitList(struct List *head, u32 elt_count, int elt_size);

#endif // LIST_H_

#include "streamlist.h"

#include "common.h"
#include "list.h"
#include "sysmem.h"

#include <string.h>

// INCLUDE_ASM("asm/nonmatchings/streamlist", InitVagStreamList);
void InitVagStreamList(struct List *list, u32 count, const char *name) {
    struct VagStreamData *d;
    int i;

    strncpy(list->name, name, 8);
    InitList(list, count, sizeof(*d), name);

    d = (struct VagStreamData *)list->list;
    for (i = 0; i < count; d++, i++) {
        d->l.in_use = 0;
        strncpy(d->name, "free", 48);
        d->id = 0;
        d->snd_handler = 0;
        d->unk0x54 = 0;
        d->unk0x4c = 0;
        d->unk0x50 = 0;
        d->unk0x58 = 0;
        d->unk0x5c = 0;
        d->unk0x60 = 0;
        d->unk0x64 = 0;
    }
}

struct VagStreamData *FindVagStreamInList(struct VagStreamData *data, struct List *list) {
    struct VagStreamData *ret = NULL;
    struct VagStreamData *d = NULL;
    u32 count;
    int i;

    count = list->elt_count;
    d = (struct VagStreamData *)list->list;

    for (i = 0; i < count; i++) {
        if (d->id == data->id && !strncmp(d->name, data->name, 48)) {
            ret = d;
            i = count;
        }

        d = d->l.next;
    }

    return ret;
}

// INCLUDE_ASM("asm/nonmatchings/streamlist", GetVagStreamInList);
struct VagStreamData *GetVagStreamInList(int index, struct List *list) {
    struct VagStreamData *d = NULL;
    u32 count;

    count = list->elt_count;
    if (index < count) {
        d = (struct VagStreamData *)list->list;

        while (index) {
            d = d->l.next;
            index--;
        }
    }

    return d;
}

void EmptyVagStreamList(struct List *list) {
    struct VagStreamData *d;
    u32 count;
    int i;

    count = list->elt_count;
    d = (struct VagStreamData *)list->buffer;

    for (i = 0; i < count; d++, i++) {
        strncpy(d->name, "free", 48);
        d->id = 0;
        d->snd_handler = 0;
        d->unk0x54 = 0;
        d->unk0x4c = 0;
        d->unk0x50 = 0;
        d->unk0x58 = 0;
        d->unk0x5c = 0;
        d->unk0x60 = 0;
        d->unk0x64 = 0;
        d->l.in_use = 0;
    }

    list->maybe_any_in_use = 1;
}

void RemoveVagStreamFromList(struct VagStreamData *data, struct List *list) {
    struct VagStreamData *d;

    d = FindVagStreamInList(data, list);
    if (d) {
        d->l.in_use = 0;
        strncpy(d->name, "free", 48);
        d->id = 0;
        d->snd_handler = 0;
        d->unk0x54 = 0;
        d->unk0x4c = 0;
        d->unk0x50 = 0;
        d->unk0x58 = 0;
        d->unk0x5c = 0;
        d->unk0x60 = 0;
        d->unk0x64 = 0;
        list->maybe_any_in_use = 1;
    }
}

static inline struct VagStreamData *findFreeEntry(struct List *list) {
    struct VagStreamData *walk, *free;
    u32 count;
    int i;

    count = list->elt_count;
    walk = (struct VagStreamData *)list->list;
    free = NULL;

    for (i = 0; i < count; i++) {
        if (!walk->id) {
            free = walk;
            i = count;
        }

        walk = walk->l.next;
    }

    return free;
}

// INCLUDE_RODATA("asm/nonmatchings/streamlist", D_00012F80);
// INCLUDE_ASM("asm/nonmatchings/streamlist", InsertVagStreamInList);
struct VagStreamData *InsertVagStreamInList(struct VagStreamData *stream, struct List *list) {
    struct VagStreamData *walk, *free;
    u32 count;
    int i;

    /*
      count = list->elt_count;
      walk = (struct VagStreamData *)list->list;
      free = NULL;

    for (i = 0; i < count; i++) {
        if (!walk->id) {
            free = walk;
            i = count;
        }

        walk = walk->l.next;
    }

      */

    free = findFreeEntry(list);

    if (free) {
        walk = (struct VagStreamData *)list->list;

    } else {
        Kprintf("IOP: ======================================================================\n");
        Kprintf("IOP: streamlist InsertVagStreamInList: no free spot in list %s\n", list->name);
        Kprintf("IOP: ======================================================================\n");
    }

    return free;
}

INCLUDE_ASM("asm/nonmatchings/streamlist", MergeVagStreamLists);

INCLUDE_ASM("asm/nonmatchings/streamlist", QueueNewStreamsFromList);

INCLUDE_ASM("asm/nonmatchings/streamlist", CheckPlayList);

INCLUDE_ASM("asm/nonmatchings/streamlist", StreamListThread);

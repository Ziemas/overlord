#ifndef STREAMLIST_H_
#define STREAMLIST_H_

#include "list.h"

struct VagStreamData {
    struct ListNode l;
    char name[48];
    int unk0x3c;
    int id;
    int unk0x44;
    int snd_handler;
    int unk0x4c;
    int unk0x50;
    int unk0x54;
    int unk0x58;
    int unk0x5c;
    int unk0x60;
    int unk0x64;
};

extern struct List PluginStreamsList;
extern struct List EEPlayList;

void RemoveVagStreamFromList(struct VagStreamData *node, struct List *list);

#endif // STREAMLIST_H_

#ifndef ISO_QUEUE_H_
#define ISO_QUEUE_H_

#include "common.h"
#include "iso_api.h"

struct PriStackEntry {
    struct ISO_Hdr *entries[8];
    int count;
};

extern struct PriStackEntry gPriStack[4];
extern int VagCmdsPriCounter[11]; // can't be in vag.c, where is it?

u8 *CheckForIsoPageBoundaryCrossing(struct ISOBuffer *buf);
void ReleaseMessage(struct ISO_Hdr *param_1, int param_2);
int QueueMessage(struct ISO_Hdr *param_1, int param_2, const char *param_3, int param_4);
void FreeBuffer(struct ISOBuffer *param_1, int param_2);

#endif // ISO_QUEUE_H_

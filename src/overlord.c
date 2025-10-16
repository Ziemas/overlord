#include "common.h"

#include <kerror.h>
#include <loadcore.h>
#include <scrtchpd.h>
#include <sif.h>
#include <sifrpc.h>
#include <stdio.h>
#include <sysmem.h>
#include <thread.h>
#include <vblank.h>

char *gLanguage = "ENG";
int IopTicks = 0;
void *ScratchPadMemory = NULL;
int IsoThreadCounter = 0;
int StopPluginStreams = 0;
int ActiveVagStreams = 0;

int SndPlayThread;

void InitBanks();
void InitSound();
void InitRamdisk();
int VBlank_Handler(void *);
void Thread_Server();
void Thread_Player();
void Thread_Loader();
void InitISOFS(char *, char *);

int start(int argc, char *argv[]) {
    struct ThreadParam th;
    int tidServer, tidPlayer, tidLoader;

    FlushDcache();

    if (!sceSifCheckInit()) {
        sceSifInit();
    }

    sceSifInitRpc(0);
    printf("IOP: =========Startup===(%x)====\n", (uint)start);
    printf("IOP: Size     %lu\n", QueryMemSize());
    printf("IOP: Free     %lu\n", QueryTotalFreeMemSize());
    printf("IOP: Max      %lu\n", QueryMaxFreeMemSize());
    printf("IOP: Used     %lu\n", QueryMemSize() - QueryTotalFreeMemSize());
    printf("IOP: start at 0x%8.8X\n", (uint)start);

    ScratchPadMemory = AllocScratchPad(0);
    if (ScratchPadMemory == KE_SPAD_INUSE) {
        Kprintf("======================================================================\n");
        Kprintf("IOP: overlord start: scratchpad busy\n");
        Kprintf("======================================================================\n");
        while (1)
            ;
    }

    InitBanks();
    InitSound();
    InitRamdisk();
    RegisterVblankHandler(0, 0x20, VBlank_Handler, NULL);

    th.entry = Thread_Server;
    th.attr = TH_C;
    th.initPriority = 0x7a;
    th.stackSize = 0x800;
    th.option = 0x0;
    tidServer = CreateThread(&th);
    if (tidServer <= 0) {
        return 1;
    }

    th.entry = Thread_Player;
    th.attr = TH_C;
    th.initPriority = 100;
    th.stackSize = 0x800;
    th.option = 0x0;
    tidPlayer = CreateThread(&th);
    if (tidPlayer <= 0) {
        return 1;
    }

    SndPlayThread = tidPlayer;

    th.entry = Thread_Loader;
    th.attr = TH_C;
    th.initPriority = 0x73;
    th.stackSize = 0x1000;
    th.option = 0x0;
    tidLoader = CreateThread(&th);
    if (tidLoader <= 0) {
        return 1;
    }

    InitISOFS(argv[1], argv[2]);
    StartThread(tidServer, 0);
    StartThread(tidPlayer, 0);
    StartThread(tidLoader, 0);

    printf("IOP: =========After inits=============\n");
    printf("IOP: Size     %lu\n", QueryMemSize());
    printf("IOP: Free     %lu\n", QueryTotalFreeMemSize());
    printf("IOP: Max      %lu\n", QueryMaxFreeMemSize());
    printf("IOP: Used     %lu\n", QueryMemSize() - QueryTotalFreeMemSize());

    return 0;
}

void ExitIOP() {
    while (1)
        ;
}

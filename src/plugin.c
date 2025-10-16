#include "common.h"

#include <989snd.h>
#include <stdio.h>
#include <sysmem.h>

int PluginId = 1;

extern Extern989HandlerPtr VagStreamHandlerPtr;
SInt32 QueueVagStream989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);
SInt32 NullPlugin989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);
SInt32 PlayQueuedVagStream989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);
SInt32 StopVagStream989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);
SInt32 SetVagStreamVolume989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);
SInt32 StopEmAll989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);
SInt32 SetStreamLfo989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);

void Init989Plugins() {
    VagStreamHandlerPtr = AllocSysMemory(0, 0x40, NULL);
    if (!VagStreamHandlerPtr) {
        Kprintf("IOP: ======================================================================\n");
        Kprintf("IOP: plugin Init989Plugins: no memory for VagStreamHandler\n");
        Kprintf("IOP: ======================================================================\n");
        while (1)
            ;
    }

    VagStreamHandlerPtr->proc_id = 0x53545256;
    VagStreamHandlerPtr->num_funcs = 11;
    VagStreamHandlerPtr->reserved1 = 0;
    VagStreamHandlerPtr->reserved2 = 0;
    VagStreamHandlerPtr->procs[0] = QueueVagStream989;
    VagStreamHandlerPtr->procs[1] = NullPlugin989;
    VagStreamHandlerPtr->procs[2] = PlayQueuedVagStream989;
    VagStreamHandlerPtr->procs[3] = StopVagStream989;
    VagStreamHandlerPtr->procs[4] = NullPlugin989;
    VagStreamHandlerPtr->procs[5] = SetVagStreamVolume989;
    VagStreamHandlerPtr->procs[6] = NullPlugin989;
    VagStreamHandlerPtr->procs[7] = StopEmAll989;
    VagStreamHandlerPtr->procs[8] = NullPlugin989;
    VagStreamHandlerPtr->procs[9] = NullPlugin989;
    VagStreamHandlerPtr->procs[10] = SetStreamLfo989;

    snd_RegisterExternProcHandler(VagStreamHandlerPtr);
}

INCLUDE_ASM("asm/nonmatchings/plugin", QueueVagStream989);

SInt32 NullPlugin989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5) { return 0; }

SInt32 PlayQueuedVagStream989(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5) { return 0; }

INCLUDE_ASM("asm/nonmatchings/plugin", StopVagStream989);

INCLUDE_ASM("asm/nonmatchings/plugin", SetVagStreamVolume989);

INCLUDE_ASM("asm/nonmatchings/plugin", StopEmAll989);

INCLUDE_ASM("asm/nonmatchings/plugin", SetStreamLfo989);

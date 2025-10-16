#include "common.h"

INCLUDE_ASM("asm/nonmatchings/vag", InitVagCmds);

INCLUDE_ASM("asm/nonmatchings/vag", SmartAllocVagCmd);

INCLUDE_ASM("asm/nonmatchings/vag", TerminateVAG);

INCLUDE_ASM("asm/nonmatchings/vag", PauseVAG);

INCLUDE_ASM("asm/nonmatchings/vag", UnPauseVAG);

INCLUDE_ASM("asm/nonmatchings/vag", RestartVag);

INCLUDE_ASM("asm/nonmatchings/vag", SetVAGVol);

INCLUDE_ASM("asm/nonmatchings/vag", SetVagStreamsNoStart);

INCLUDE_ASM("asm/nonmatchings/vag", InitVAGCmd);

INCLUDE_ASM("asm/nonmatchings/vag", SetVagStreamsNotScanned);

INCLUDE_ASM("asm/nonmatchings/vag", RemoveVagCmd);

INCLUDE_ASM("asm/nonmatchings/vag", FindFreeVagCmd);

INCLUDE_ASM("asm/nonmatchings/vag", FindNotQueuedVagCmd);

INCLUDE_ASM("asm/nonmatchings/vag", FindWhosPlaying);

INCLUDE_ASM("asm/nonmatchings/vag", FindVagStreamId);

INCLUDE_ASM("asm/nonmatchings/vag", FindVagStreamPluginId);

INCLUDE_ASM("asm/nonmatchings/vag", FindVagStreamName);

INCLUDE_ASM("asm/nonmatchings/vag", FindThisVagStream);

INCLUDE_ASM("asm/nonmatchings/vag", AnyVagRunning);

INCLUDE_ASM("asm/nonmatchings/vag", FreeVagCmd);

INCLUDE_RODATA("asm/nonmatchings/vag", D_00012BB0);

INCLUDE_RODATA("asm/nonmatchings/vag", D_00012C00);

INCLUDE_ASM("asm/nonmatchings/vag", SetNewVagCmdPri);

INCLUDE_ASM("asm/nonmatchings/vag", HowManyBelowThisPriority);

INCLUDE_ASM("asm/nonmatchings/vag", StopVAG);

INCLUDE_ASM("asm/nonmatchings/vag", VAG_MarkLoopEnd);

INCLUDE_ASM("asm/nonmatchings/vag", VAG_MarkLoopStart);

INCLUDE_ASM("asm/nonmatchings/vag", CalculateVAGPitch);

INCLUDE_ASM("asm/nonmatchings/vag", PauseVagStreams);

INCLUDE_ASM("asm/nonmatchings/vag", UnPauseVagStreams);

INCLUDE_ASM("asm/nonmatchings/vag", SetAllVagsVol);

INCLUDE_ASM("asm/nonmatchings/vag", CalculateVAGVolumes);

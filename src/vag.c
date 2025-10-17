#include "vag.h"

#include "common.h"

struct VagCmd VagCmds[4];
struct VagCmd *VagCmdsPriList[11][4];
int VagCmdsPriCounter[11];
int ActiveVagStreams;
int StreamSRAM[4];
int TrapSRAM[4];
int StreamVoice[4];

int NullCallback(struct iso_message *, struct iso_buffer_header *);

// INCLUDE_ASM("asm/nonmatchings/vag", InitVagCmds);
void InitVagCmds() {
    struct VagCmd *cmd;
    int i, j;

    cmd = VagCmds;
    for (i = 0; i < 4; i++, cmd++) {
        for (j = 0; j < 25; j++) {
            cmd->status[j] = 0;
        }

        cmd->unkec = 0;
        cmd->unk8c = 0;
        cmd->status[2] = 1;
        cmd->unk0xc4 = 0;
        cmd->unk0xc8 = 0;
        cmd->unk0xcc = 0;
        cmd->unk0xb4 = 0;
        cmd->unk0xb8 = 0;
        cmd->unk0xbc = 0;
        cmd->unk0xc0 = 0;
        cmd->status[5] = 0;
        cmd->status[6] = 0;
        cmd->buffers_count = 0;
        cmd->safe_to_change_dma_fields = 1;
        cmd->data_left = 0;
        cmd->sample_rate = 0;
        cmd->invalid = 0;
        cmd->unk_264 = 0x4000;
        cmd->unk10c = 0;
        cmd->msg.cb_buf = NULL;
        cmd->msg.ready_for_data = 0;
        cmd->msg.callback = NullCallback;
        cmd->msg.lse = 0;
        cmd->sibling = NULL;
        cmd->iop_dma_mem = NULL;
        cmd->transfer_channel = -1;
        cmd->new_pitch = 0;
        cmd->positioned = 0;
        cmd->trans[0] = 0;
        cmd->trans[1] = 0;
        cmd->trans[2] = 0;
        cmd->fo_min = 0;
        cmd->fo_max = 0;
        cmd->fo_curve = 0;
        cmd->end_addr = TrapSRAM[i];
        cmd->spu_base_addr = StreamSRAM[i];
        cmd->voice = StreamVoice[i];
        cmd->cmd_id = i;
        cmd->unk28 = 0;
        cmd->vag_dir_entry = NULL;
        cmd->status[1] = 0;
        cmd->volume = 0;
        cmd->new_pitch = 0;
        cmd->id = 0;
        cmd->unk118 = 0;
        cmd->sound_handler = 0;
        cmd->unk0xb0 = 0;
        cmd->unk11c = 0;
        cmd->unk120 = 0;
        cmd->unk124 = 0;
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 11; j++) {
            VagCmdsPriList[j][i] = NULL;
        }
    }

    for (i = 0; i < 11; i++) {
        VagCmdsPriCounter[i] = 0;
    }

    VagCmdsPriCounter[0] = 4;
}

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

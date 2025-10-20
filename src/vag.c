#include "vag.h"

#include "common.h"
#include "iso_queue.h"
#include "libsd.h"
#include "spustreams.h"
#include "srpc.h"
#include "ssound.h"
#include "streamlfo.h"
#include "streamlist.h"

#include "intrman.h"
#include "string.h"
#include "sysmem.h"

struct VagCmd VagCmds[4];
struct VagCmd *VagCmdsPriList[11][4];
int ActiveVagStreams;
int StreamSRAM[4];
int TrapSRAM[4];
int StreamVoice[4];

int NullCallback(struct ISO_Hdr *, struct ISOBuffer *);

void InitVAGCmd(struct VagCmd *vag, int status) {
    int i;

    for (i = 0; i < 25; i++) {
        vag->status[i] = 0;
    }

    vag->unkec = 0;
    vag->unk8c = 0;

    if (status) {
        vag->status[2] = 1;
    } else {
        vag->status[2] = 0;
    }

    vag->playPos = 0;
    vag->vagClock_unk2 = 0;
    vag->unk0xcc = 0;
    vag->vagClock_unk1 = 0;
    vag->unk0xb8 = 0;
    vag->vagClockS = 0;
    vag->unk0xc0 = 0;
    vag->status[5] = 0;
    vag->status[6] = 0;
    vag->buffers_count = 0;
    vag->safe_to_change_dma_fields = 1;
    vag->data_left = 0;
    vag->sample_rate = 0;
    vag->invalid = 0;
    vag->unk_264 = 0x4000;
    vag->unk10c = 0;
    vag->msg.cb_buf = NULL;
    vag->msg.ready_for_data = 1;
    vag->msg.callback = NullCallback;
    vag->msg.lse = NULL;
    vag->sibling = NULL;
    vag->iop_dma_mem = NULL;
    vag->transfer_channel = -1;
    vag->new_pitch = 0;
    vag->positioned = 0;
    vag->trans[0] = 0;
    vag->trans[1] = 0;
    vag->trans[2] = 0;
    vag->fo_min = 5;
    vag->fo_max = 30;
    vag->fo_curve = 1;
}

void InitVagCmds() {
    struct VagCmd *vag;
    int i, j;

    vag = VagCmds;
    for (i = 0; i < 4; i++, vag++) {
        InitVAGCmd(vag, 1);

        vag->spu_base_addr = StreamSRAM[i];
        vag->trap_addr = TrapSRAM[i];
        vag->voice = StreamVoice[i];

        vag->cmd_id = i;
        vag->unk28 = 0;
        vag->vag_dir_entry = NULL;
        vag->status[1] = 0;
        vag->volume = 0;
        vag->new_pitch = 0;
        vag->id = 0;
        vag->plugin_id = 0;
        vag->sound_handler = 0;
        vag->unk0xb0 = 0;
        vag->priority = 0;
        vag->unk120 = 0;
        vag->unk124 = 0;
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 11; j++) {
            VagCmdsPriList[j][i] = NULL;
        }
    }

    for (j = 0; j < 11; j++) {
        VagCmdsPriCounter[j] = 0;
    }

    VagCmdsPriCounter[0] = 4;
}

void RemoveVagCmd(struct VagCmd *vag, int disable_intr) {
    int oldstat;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    VagCmdsPriList[vag->priority][vag->cmd_id] = NULL;
    if (VagCmdsPriCounter[vag->priority] > 0) {
        VagCmdsPriCounter[vag->priority]--;
    } else {
        Kprintf("IOP: ======================================================================\n");
        Kprintf("IOP: vag RemoveVagCmd: VagCmdsPriCounter[%d] is zero\n", vag->priority);
        Kprintf("IOP: ======================================================================\n");
    }

    VagCmdsPriCounter[0]++;

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

struct VagCmd *FindFreeVagCmd() {
    int i;

    for (i = 0; i < 4; i++) {
        if (!VagCmds[i].id) {
            return &VagCmds[i];
        }
    }

    return NULL;
}

// INCLUDE_ASM("asm/nonmatchings/vag", SetNewVagCmdPri);
void SetNewVagCmdPri(struct VagCmd *vag, int new_pri, int disable_intr) {
    int oldstat;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    if (vag) {
        VagCmdsPriList[vag->priority][vag->cmd_id] = NULL;
        if (VagCmdsPriCounter[vag->priority] > 0) {
            VagCmdsPriCounter[vag->priority]--;
        } else {
            Kprintf("IOP: ======================================================================\n");
            Kprintf("IOP: vag RemoveVagCmd: VagCmdsPriCounter[%d] is zero\n", vag->priority);
            Kprintf("IOP: ======================================================================\n");
        }

        VagCmdsPriList[new_pri][vag->cmd_id] = vag;
        VagCmdsPriCounter[new_pri]++;
        vag->priority = new_pri;
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

int HowManyBelowThisPriority(u32 pri, int disable_intr) {
    int oldstat;
    int i, ret;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    ret = 0;
    for (i = 0; i < pri; i++) {
        ret += VagCmdsPriCounter[i];
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }

    return ret;
}

struct VagCmd *SmartAllocVagCmd(struct VagCmd *vag) {
    struct VagCmd *found;
    u32 pri;
    int i;

    found = FindFreeVagCmd();
    if (!found) {
        found = FindNotQueuedVagCmd();
    }

    if (!found) {
        pri = found->priority;
        if (vag->priority) {
            for (pri = 0; pri < vag->priority; pri++) {
                for (i = 0; i < 4; i++) {
                    if (VagCmdsPriList[pri][i]) {
                        found = VagCmdsPriList[pri][i];
                        if (!found->status[11]) {
                            i = 4;
                            pri = vag->priority;
                        }
                    }
                }
            }
        }
    }

    if (found) {
        ActiveVagStreams++;
        if (ActiveVagStreams < 2) {
            WakeSpuStreamsUp();
        }
    }

    return found;
}

void FreeVagCmd(struct VagCmd *vag, int disable_intr) {
    int oldstat;
    int i;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    for (i = 0; i < 25; i++) {
        vag->status[i] = 0;
    }

    vag->status[1] = 0;
    vag->status[2] = 1;
    vag->status[7] = 0;
    vag->vagClock_unk1 = 0;
    vag->unk0xb8 = 0;
    vag->vagClockS = 0;
    vag->unk0xc0 = 0;
    SetVagStreamName(vag, 0, 0);
    vag->name[0] = 0;
    vag->unk8c = 0;
    vag->base_pitch = 0;
    vag->unk28 = 0;
    vag->vag_dir_entry = 0;
    vag->playPos = 0;
    vag->vagClock_unk2 = 0;
    vag->unk0xcc = 0;
    vag->buffers_count = 0;
    vag->safe_to_change_dma_fields = 1;
    vag->data_left = 0;
    vag->sample_rate = 0;
    vag->invalid = 0;
    vag->unk10c = 0;
    vag->volume = 0;
    vag->unk_264 = 0x4000;
    vag->new_pitch = 0;
    vag->id = 0;
    vag->plugin_id = 0;
    vag->sound_handler = 0;
    vag->priority = 0;
    vag->unk120 = 0;
    vag->unk124 = 0;
    vag->positioned = 0;
    vag->msg.cb_buf = 0;
    vag->msg.ready_for_data = 0;
    vag->msg.callback = NullCallback;
    vag->msg.lse = 0;
    vag->iop_dma_mem = 0;
    vag->transfer_channel = -1;
    vag->unkec = 0;

    if (ActiveVagStreams > 0) {
        ActiveVagStreams--;
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

void SetVagStreamsNotScanned() {
    struct VagCmd *vag;
    int i;

    vag = VagCmds;
    for (i = 0; i < 4; i++, vag++) {
        vag->status[7] = 0;
    }
}

void SetVagStreamsNoStart(int value, int disable_intr) {
    struct VagCmd *vag;
    int oldstat;
    int i;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    vag = VagCmds;
    if (value == 1) {
        for (i = 0; i < 4; i++, vag++) {
            vag->status[23] = 1;
        }
    } else if (value == 0) {
        for (i = 0; i < 4; i++, vag++) {
            vag->status[23] = 0;
        }
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

struct VagCmd *FindNotQueuedVagCmd() {
    struct VagCmd *vag;
    int i;

    vag = VagCmds;
    for (i = 0; i < 4; i++) {
        if (!vag->status[7] && !vag->status[0xb] && !vag->status[4]) {
            return vag;
        }

        vag++;
    }

    return NULL;
}

struct VagCmd *FindWhosPlaying() {
    struct VagCmd *vag;
    int i;

    vag = VagCmds;
    for (i = 0; i < 4; i++) {
        if (!vag->status[2] && vag->status[1]) {
            return vag;
        }

        vag++;
    }

    return NULL;
}

int AnyVagRunning() {
    struct VagCmd *vag;
    int i, count;

    vag = VagCmds;
    count = 0;
    for (i = 0; i < 4; i++) {
        if (vag->status[4]) {
            count++;
        }

        vag++;
    }

    return count;
}

struct VagCmd *FindVagStreamPluginId(int id) {
    struct VagCmd *vag;
    int i;

    if (id) {
        vag = VagCmds;
        for (i = 0; i < 4; i++) {
            if (vag->plugin_id == id) {
                return vag;
            }

            vag++;
        }
    }

    return NULL;
}

struct VagCmd *FindVagStreamId(int id) {
    struct VagCmd *vag;
    int i;

    if (id) {
        vag = VagCmds;
        for (i = 0; i < 4; i++) {
            if (vag->id == id) {
                return vag;
            }

            vag++;
        }
    }

    return NULL;
}

struct VagCmd *FindVagStreamName(char *name) {
    struct VagCmd *vag;
    int i;

    vag = VagCmds;
    for (i = 0; i < 4; i++) {
        if (!strcmp(vag->name, name)) {
            return vag;
        }

        vag++;
    }

    return NULL;
}

struct VagCmd *FindThisVagStream(char *name, int id) {
    struct VagCmd *vag;
    int i;

    vag = VagCmds;
    for (i = 0; i < 4; i++) {
        if (!strcmp(vag->name, name) && vag->id == id) {
            return vag;
        }

        vag++;
    }

    return NULL;
}

// INCLUDE_RODATA("asm/nonmatchings/vag", D_00012BB0);
// INCLUDE_RODATA("asm/nonmatchings/vag", D_00012C00);
// INCLUDE_ASM("asm/nonmatchings/vag", StopVAG);
void StopVAG(struct VagCmd *vag, int disable_intr) {
    struct VagCmd *sibling;
    int oldstat;
    u32 koff;
    int i;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    sibling = vag->sibling;
    PauseVAG(vag, 0);

    if (vag->status[5]) {
        koff = 1 << (vag->voice >> 1);
        if (sibling) {
            koff |= 1 << (sibling->voice >> 1);
        }
        sceSdSetSwitch(SD_S_KOFF | vag->voice & 1, koff);
    }

    for (i = 0; i < 25; i++) {
        vag->status[i] = 0;
    }

    vag->volume = 0;
    vag->new_pitch = 0;
    vag->id = 0;
    vag->plugin_id = 0;
    vag->msg.ready_for_data = 0;
    vag->sound_handler = 0;
    vag->unk8c = 0;
    vag->base_pitch = 0;
    vag->msg.callback = NullCallback;
    vag->vagClock_unk1 = 0;
    vag->unk0xb8 = 0;
    vag->vagClockS = 0;
    vag->unk0xc0 = 0;

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

// INCLUDE_ASM("asm/nonmatchings/vag", TerminateVAG);
void TerminateVAG(struct VagCmd *vag, int disable_intr) {
    struct VagCmd *sibling;
    struct VagStreamData data;
    struct LfoListData lfo_data;
    int oldstat;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    sibling = vag->sibling;
    strncpy(data.name, vag->name, 48);
    data.id = vag->id;
    vag->status[7] = 0;
    if (vag->status[5]) {
        StopVAG(vag, 0);
    }

    ReleaseMessage(&vag->msg, 0);
    RemoveVagCmd(vag, 0);
    FreeVagCmd(vag, 0);

    if (sibling) {
        sibling->status[7] = 0;
        RemoveVagCmd(sibling, 0);
        FreeVagCmd(sibling, 0);
    }

    if (vag->sound_handler) {
        RemoveVagStreamFromList(&data, &PluginStreamsList);
        lfo_data.unk0x24 = vag->id;
        lfo_data.unk0x28 = vag->plugin_id;
        RemoveLfoStreamFromList(&lfo_data, &LfoList);
    }

    RemoveVagStreamFromList(&data, &EEPlayList);

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

// INCLUDE_ASM("asm/nonmatchings/vag", PauseVAG);
void PauseVAG(struct VagCmd *vag, int disable_intr) {
    struct VagCmd *sibling;
    int oldstat;
    u32 koff, addr;

    if (vag->status[2]) {
        return;
    }

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    if (!vag->status[11]) {
        sibling = vag->sibling;
        if (sibling) {
            if (vag->status[1]) {
                sceSdSetParam(SD_VP_VOLL | vag->voice, 0);
                sceSdSetParam(SD_VP_VOLR | vag->voice, 0);
                sceSdSetParam(SD_VP_VOLL | sibling->voice, 0);
                sceSdSetParam(SD_VP_VOLR | sibling->voice, 0);
            }
            sceSdSetParam(SD_VP_PITCH | sibling->voice, 0);
            sceSdSetParam(SD_VP_PITCH | vag->voice, 0);
            koff = VOICE_BIT(vag->voice);
            koff |= VOICE_BIT(sibling->voice);
            sceSdSetSwitch(SD_S_KOFF | CORE_BIT(vag->voice), koff);

            if (vag->status[5]) {
                vag->resume_addr = addr = GetSpuRamAddress(vag) & ~0x7;
                sibling->resume_addr = (vag->resume_addr - vag->spu_base_addr) + sibling->spu_base_addr;
            } else {
                vag->resume_addr = 0;
                sibling->resume_addr = 0;
            }

            vag->status[2] = 1;
            sibling->status[2] = 1;
        } else {
            if (vag->status[1]) {
                sceSdSetParam(SD_VP_VOLL | vag->voice, 0);
                sceSdSetParam(SD_VP_VOLR | vag->voice, 0);
            }
            sceSdSetParam(SD_VP_PITCH | vag->voice, 0);
            koff = VOICE_BIT(vag->voice);
            sceSdSetSwitch(SD_S_KOFF | CORE_BIT(vag->voice), koff);

            if (vag->status[5]) {
                vag->resume_addr = GetSpuRamAddress(vag);
            } else {
                vag->resume_addr = 0;
            }

            vag->status[2] = 1;
        }
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

void UnPauseVAG(struct VagCmd *vag, int disable_intr) {
    struct VagCmd *sibling;
    int oldstat;
    s32 voll, volr;
    int pitch;
    u32 kon;

    if (!vag->status[2]) {
        return;
    }

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    if (!vag->status[11]) {
        sibling = vag->sibling;
        pitch = CalculateVAGPitch(vag->base_pitch, vag->new_pitch);
        CalculateVAGVolumes(vag, &voll, &volr);
        if (sibling) {
            if (vag->status[1]) {
                kon = VOICE_BIT(vag->voice);
                kon |= VOICE_BIT(sibling->voice);
                sceSdSetParam(SD_VP_PITCH | vag->voice, pitch);
                sceSdSetParam(SD_VP_PITCH | sibling->voice, pitch);

                if (vag->resume_addr) {
                    sceSdSetAddr(SD_VA_SSA | vag->voice, vag->resume_addr);
                    sceSdSetAddr(SD_VA_SSA | sibling->voice, sibling->resume_addr);
                }

                sceSdSetSwitch(SD_S_KON | CORE_BIT(vag->voice), kon);

                sceSdSetParam(SD_VP_VOLL | vag->voice, voll);
                sceSdSetParam(SD_VP_VOLL | sibling->voice, 0);
                sceSdSetParam(SD_VP_VOLR | vag->voice, 0);
                sceSdSetParam(SD_VP_VOLR | sibling->voice, volr);
            }
            vag->status[2] = 0;
            sibling->status[2] = 0;
        } else {
            if (vag->status[1]) {
                kon = VOICE_BIT(vag->voice);
                sceSdSetParam(SD_VP_PITCH | vag->voice, pitch);
                if (vag->resume_addr) {
                    sceSdSetAddr(SD_VA_SSA | vag->voice, vag->resume_addr);
                }
                sceSdSetSwitch(SD_S_KON | CORE_BIT(vag->voice), kon);
                sceSdSetParam(SD_VP_VOLL | vag->voice, voll);
                sceSdSetParam(SD_VP_VOLR | vag->voice, volr);
            }
            vag->status[2] = 0;
        }
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

// INCLUDE_ASM("asm/nonmatchings/vag", RestartVag);
void RestartVag(struct VagCmd *vag, int buffer, int disable_intr) {
    struct VagCmd *sibling;
    int oldstat;
    s32 voll, volr;
    u32 vbits;
    u32 offset;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    CalculateVAGVolumes(vag, &voll, &volr);
    if (!vag->status[11]) {
        sibling = vag->sibling;
        offset = (!!buffer) << 13;

        vbits = VOICE_BIT(vag->voice);
        if (sibling) {
            vbits |= VOICE_BIT(sibling->voice);
        }

        sceSdSetSwitch(SD_S_KOFF | CORE_BIT(vag->voice), vbits);
        sceSdSetParam(SD_VP_VOLL | vag->voice, 0);
        sceSdSetParam(SD_VP_VOLR | vag->voice, 0);
        if (sibling) {
            sceSdSetParam(SD_VP_VOLL | sibling->voice, 0);
            sceSdSetParam(SD_VP_VOLR | sibling->voice, 0);
            sceSdSetAddr(SD_VA_SSA | vag->voice, vag->spu_base_addr + offset);
            sceSdSetAddr(SD_VA_SSA | sibling->voice, sibling->spu_base_addr + offset);
        } else {
            sceSdSetAddr(SD_VA_SSA | vag->voice, vag->spu_base_addr + offset);
        }

        sceSdSetSwitch(SD_S_KON | CORE_BIT(vag->voice), vbits);
        if (sibling) {
            sceSdSetParam(SD_VP_VOLL | vag->voice, voll);
            sceSdSetParam(SD_VP_VOLL | sibling->voice, 0);
            sceSdSetParam(SD_VP_VOLR | vag->voice, 0);
            sceSdSetParam(SD_VP_VOLR | sibling->voice, volr);
        } else {
            sceSdSetParam(SD_VP_VOLL | vag->voice, voll);
            sceSdSetParam(SD_VP_VOLR | vag->voice, volr);
        }
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

void PauseVagStreams() {
    struct VagCmd *vag;
    int i;

    vag = VagCmds;
    for (i = 0; i < 4; i++) {
        if (vag->status[4] && !vag->status[2]) {
            PauseVAG(vag, 1);
        }

        vag++;
    }
}

void UnPauseVagStreams() {
    struct VagCmd *vag;
    int i;

    vag = VagCmds;
    for (i = 0; i < 4; i++) {
        if (vag->status[4] && vag->status[2]) {
            UnPauseVAG(vag, 1);
        }

        vag++;
    }
}

void CalculateVAGVolumes(struct VagCmd *vag, int *out_left, int *out_right) {
    if (!vag->positioned) {
        s32 vol;

        vol = (vag->volume * MasterVolume[2]) >> 6;
        if (vol >= 0x4000) {
            vol = 0x3fff;
        }
        *out_left = vol;
        *out_right = vol;
    } else {
        struct VolumePair *pan;
        u32 angle;
        u32 vol;

        vol = CalculateFalloffVolume(vag->trans, (vag->volume * MasterVolume[2]) >> 10, vag->fo_curve, vag->fo_min,
                                     vag->fo_max);

        angle = (u32)(630 - CalculateAngle(vag->trans)) % 360;
        pan = &gPanTable[angle];

        *out_left = (pan->left * vol) >> 10;
        *out_right = (pan->right * vol) >> 10;

        if (*out_left >= 0x4000) {
            *out_left = 0x3fff;
        }

        if (*out_right >= 0x4000) {
            *out_right = 0x3fff;
        }
    }
}

int CalculateVAGPitch(u32 base, s32 mod) {
    if (mod != 0) {
        if (mod > 0) {
            base = (base * (mod + 1524)) / 1524;
        } else {
            base = 1524 * base / (1524 - mod);
        }
    }

    return base;
}

// INCLUDE_ASM("asm/nonmatchings/vag", SetVAGVol);
void SetVAGVol(struct VagCmd *vag, int disable_intr) {
    struct VagCmd *sibling;
    GSoundHandlerPtr sndhnd;
    sceSdBatch batch[6];
    int pitch, batch_count;
    int voll, volr;
    int oldstat;

    if (vag && vag->status[4] && !vag->status[2] && !vag->status[11]) {
        CalculateVAGVolumes(vag, &voll, &volr);
        sndhnd = vag->sound_handler;
        sibling = sibling;
        if (sndhnd) {
            // TODO
        }

        if (sibling) {
            batch[0].func = 1;
            batch[0].entry = SD_VP_VOLL | vag->voice;
            batch[0].value = voll;

            batch[1].func = 1;
            batch[1].entry = SD_VP_VOLL | sibling->voice;
            batch[1].value = 0;

            batch[2].func = 1;
            batch[2].entry = SD_VP_VOLR | vag->voice;
            batch[2].value = 0;

            batch[3].func = 1;
            batch[3].entry = SD_VP_VOLR | sibling->voice;
            batch[3].value = volr;

            pitch = CalculateVAGPitch(vag->base_pitch, vag->new_pitch);
            batch[4].func = 1;
            batch[4].entry = SD_VP_PITCH | vag->voice;
            batch[4].value = pitch;

            batch[5].func = 1;
            batch[5].entry = SD_VP_PITCH | sibling->voice;
            batch[5].value = pitch;
            batch_count = 6;
        } else {
            batch[0].func = 1;
            batch[0].entry = SD_VP_VOLL | vag->voice;
            batch[0].value = voll;

            batch[1].func = 1;
            batch[1].entry = SD_VP_VOLR | vag->voice;
            batch[1].value = volr;

            pitch = CalculateVAGPitch(vag->base_pitch, vag->new_pitch);
            batch[3].func = 1;
            batch[3].entry = SD_VP_PITCH | vag->voice;
            batch[3].value = pitch;

            batch_count = 3;
        }

        if (disable_intr == 1) {
            CpuSuspendIntr(&oldstat);
            sceSdProcBatch(batch, NULL, batch_count);
            CpuResumeIntr(oldstat);
        } else {
            sceSdProcBatch(batch, NULL, batch_count);
        }
    }
}

void SetAllVagsVol(int group) {
    GSoundHandlerPtr sndhnd;
    struct VagCmd *vag;
    int i;

    vag = VagCmds;

    if (group < 0) {
        for (i = 0; i < 4; i++) {
            SetVAGVol(vag, 1);
            vag++;
        }
    } else {
        for (i = 0; i < 4; i++) {
            sndhnd = vag->sound_handler;
            if (sndhnd) {
                if (sndhnd->VolGroup == group) {
                    SetVAGVol(vag, 1);
                }

                vag++;
            }
        }
    }
}

void VAG_MarkLoopStart(char *data) {
    data[1] = 6;
    data[17] = 2;
}

void VAG_MarkLoopEnd(char *data, int offset) { data[offset - 15] = 3; }

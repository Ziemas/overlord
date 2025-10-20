#ifndef VAG_H_
#define VAG_H_

#include "common.h"
#include "iso_api.h"

#define VOICE_BIT(voice) (1 << ((voice) >> 1))
#define CORE_BIT(voice) ((voice)&1)

struct VagDirEntry {
    char name[8];
    u32 offset;
    u32 channels;
};

struct VagCmd {
    struct ISO_Hdr msg;
    int unk28;
    struct VagDirEntry *vag_dir_entry;
    struct VagCmd *sibling;
    u8 *iop_dma_mem;
    u32 transfer_channel;
    int safe_to_change_dma_fields;
    u32 resume_addr;
    char name[48];
    u32 unk74;
    u32 spu_base_addr;
    u32 trap_addr;
    int voice;
    u32 cmd_id;
    GSoundHandlerPtr sound_handler;
    int unk8c;
    char unk21[30];
    u16 unkae;
    int unk0xb0;
    int vagClock_unk1;
    int unk0xb8;
    int vagClockS;
    int unk0xc0;
    int playPos;
    int vagClock_unk2;
    int unk0xcc;

    // byte 18 - dma field 0 status
    // byte 19 - dma field 1 status
    char status[0x19];
    int unkec;
    u32 buffers_count;
    s32 data_left;
    u32 sample_rate;
    u32 base_pitch;
    u32 new_pitch;
    u32 invalid;
    u32 unk_264;
    int unk10c;
    u32 volume;
    int id;
    int plugin_id;
    int priority;
    int unk120;
    int unk124;
    u32 positioned;
    s32 trans[3];
    s32 fo_min;
    s32 fo_max;
    s32 fo_curve;
};

extern struct VagCmd VagCmds[4];

int CalculateVAGPitch(u32 base, s32 mod);
void PauseVAG(struct VagCmd *vag, int disable_intr);
void UnPauseVAG(struct VagCmd *vag, int disable_intr);
void RestartVag(struct VagCmd *vag, int param_2, int disable_intr);
struct VagCmd* FindNotQueuedVagCmd();
void CalculateVAGVolumes(struct VagCmd* cmd, s32* l_out, s32* r_out);

#endif // VAG_H_

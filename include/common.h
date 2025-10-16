#ifndef COMMON_H_
#define COMMON_H_

#include "include_asm.h"
#include "types.h"

typedef enum { false, true } bool;

typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;
typedef signed char s8;
typedef unsigned char u8;


extern s32 gSema;
extern s32 gMusic;

struct fs_api {
    void *init;
    void *find;
    void *find_in;
    void *get_length;
    void *open;
    void *open_wad;
    void *close;
    void *page_begin_read;
    void *sync_read;
    void *load_sound_bank;
    void *store_sound_bank_in_iop;
    void *load_sound_bank_from_iop;
    void *load_sound_bank_from_ee;
    void *load_music;
    void *poll_drive;
};

struct SoundParams {
    u16 mask;
    s16 pitch_mod;
    s16 bend;
    s16 fo_min;
    s16 fo_max;
    s8 fo_curve;
    s8 priority;
    s32 volume;
    s32 trans[3];
    u8 group;
    u8 reg[3];
};

struct SoundRpcGetIrxVersion {
    u32 major;
    u32 minor;
    u32 ee_addr;
};

struct SoundRpcBankCommand {
    u8 pad[12];
    char bank_name[16];
};

struct SoundRpcSetLanguageCommand {
    u32 language_id;
};

struct SoundRpcPlayCommand {
    u32 sound_id;
    u32 pad[2];
    char name[16];
    struct SoundParams parms;
};

struct SoundRpcSetParamCommand {
    u32 sound_id;
    struct SoundParams parms;
    s32 auto_time;
    s32 auto_from;
};

struct SoundRpcSoundIdCommand {
    u32 sound_id;
};

struct SoundRpcSetFlavaCommand {
    u8 flava;
};

struct SoundRpcSetReverb {
    u8 core;
    s32 reverb;
    u32 left;
    u32 right;
};

struct SoundRpcSetEarTrans {
    s32 ear_trans[3];
    s32 cam_trans[3];
    s32 cam_angle;
};

struct SoundRpc2SetEarTrans {
    s32 ear_trans1[3];
    s32 ear_trans0[3];
    s32 cam_trans[3];
    s32 cam_angle;
};

struct SoundRpcSetFPSCommand {
    u8 fps;
};

struct SoundRpcSetFallof {
    u8 pad[12];
    char name[16];
    s32 curve;
    s32 min;
    s32 max;
};

struct SoundRpcSetFallofCurve {
    s32 curve;
    s32 falloff;
    s32 ease;
};

struct SoundRpcGroupCommand {
    u8 group;
};

struct SoundRpcMasterVolCommand {
    struct SoundRpcGroupCommand group;
    s32 volume;
};

struct SoundRpcStereoMode {
    s32 stereo_mode;
};

struct SoundRpcSetMidiReg {
    s32 reg;
    s32 value;
};

enum SOUND_COMMAND {
    SCMD_IOP_STORE = 0,
    SCMD_IOP_FREE = 1,
    SCMD_LOAD_BANK = 2,
    SCMD_LOAD_BANK_FROM_IOP = 3,
    SCMD_LOAD_BANK_FROM_EE = 4,
    SCMD_LOAD_MUSIC = 5,
    SCMD_UNLOAD_BANK = 6,
    SCMD_PLAY = 7,
    SCMD_PAUSE_SOUND = 8,
    SCMD_STOP_SOUND = 9,
    SCMD_CONTINUE_SOUND = 10,
    SCMD_SET_PARAM = 11,
    SCMD_SET_MASTER_VOLUME = 12,
    SCMD_PAUSE_GROUP = 13,
    SCMD_STOP_GROUP = 14,
    SCMD_CONTINUE_GROUP = 15,
    SCMD_GET_IRX_VERSION = 16,
    SCMD_SET_FALLOFF_CURVE = 17,
    SCMD_SET_SOUND_FALLOFF = 18,
    SCMD_RELOAD_INFO = 19,
    SCMD_SET_LANGUAGE = 20,
    SCMD_SET_FLAVA = 21,
    SCMD_SET_MIDI_REG = 22,
    SCMD_SET_REVERB = 23,
    SCMD_SET_EAR_TRANS = 24,
    SCMD_SHUTDOWN = 25,
    SCMD_LIST_SOUNDS = 26,
    SCMD_UNLOAD_MUSIC = 27,
    SCMD_SET_FPS = 28,
    SCMD_BOOT_LOAD = 29,
    SCMD_GAME_LOAD = 30,
    SCMD_NUM_TESTS = 31,
    SCMD_NUM_TESTRUNS = 32,
    SCMD_NUM_SECTORS = 33,
    SCMD_NUM_STREAMSECTORS = 34,
    SCMD_NUM_STREAMBANKS = 35,
    SCMD_TRACK_PITCH = 36,
    SCMD_LINVEL_NOM = 37,
    SCMD_LINVEL_STM = 38,
    SCMD_SEEK_NOM = 39,
    SCMD_SEEK_STM = 40,
    SCMD_READ_SEQ_NOM = 41,
    SCMD_READ_SEQ_STM = 42,
    SCMD_READ_SPR_NOM = 43,
    SCMD_READ_SPR_STM = 44,
    SCMD_READ_SPR_STRN_NOM = 45,
    SCMD_RAND_STM_ABORT = 46,
    SCMD_RAND_NOM_ABORT = 47,
    SCMD_IOP_MEM = 48,
    SCMD_CANCEL_DGO = 49,
    SCMD_SET_STEREO_MODE = 50,
};

struct SoundRpcCommand {
    short rsvd1;
    u16 command;
    union {
        struct SoundRpcGetIrxVersion irx_version;
        struct SoundRpcBankCommand load_bank;
        struct SoundRpcSetLanguageCommand set_language;
        struct SoundRpcPlayCommand play;
        struct SoundRpcSoundIdCommand sound_id;
        struct SoundRpcSetFPSCommand fps;
        struct SoundRpcSetEarTrans ear_trans;
        struct SoundRpc2SetEarTrans ear_trans_j2;
        struct SoundRpcSetReverb reverb;
        struct SoundRpcSetFallof fallof;
        struct SoundRpcSetFallofCurve fallof_curve;
        struct SoundRpcGroupCommand group;
        struct SoundRpcSetFlavaCommand flava;
        struct SoundRpcMasterVolCommand master_volume;
        struct SoundRpcSetParamCommand param;
        struct SoundRpcStereoMode stereo_mode;
        struct SoundRpcSetMidiReg midi_reg;
        u8 max_size[0x4c]; // Temporary
    } u;
};

#endif // COMMON_H_

#ifndef _989SND_H_
#define _989SND_H_

typedef char SInt8;
typedef unsigned char UInt8;
typedef short int SInt16;
typedef short unsigned int UInt16;
typedef int SInt32;
typedef unsigned int UInt32;
typedef long int SInt64;
typedef long unsigned int UInt64;
typedef int BOOL;

typedef SInt32 SoundBankPtr;

typedef SInt32 (*Extern989Proc)(SInt32 arg1, SInt32 arg2, SInt32 arg3, SInt32 arg4, SInt32 arg5);

typedef struct { // 0x8
    /* 0x0 */ UInt32 core[2];
} VoiceFlags;

typedef struct { // 0x1c
    /* 0x00 */ SInt32 Type;
    /* 0x04 */ struct SoundBank_tag *Bank;
    /* 0x08 */ void *OrigBank;
    /* 0x0c */ SInt8 OrigType;
    /* 0x0d */ SInt8 Prog;
    /* 0x0e */ SInt8 Note;
    /* 0x0f */ SInt8 Fine;
    /* 0x10 */ SInt16 Vol;
    /* 0x12 */ SInt8 pad1;
    /* 0x13 */ SInt8 VolGroup;
    /* 0x14 */ SInt16 Pan;
    /* 0x16 */ SInt8 XREFSound;
    /* 0x17 */ SInt8 pad2;
    /* 0x18 */ UInt16 Flags;
    /* 0x1a */ UInt16 pad3;
} Sound;

typedef Sound *SoundPtr;

typedef SInt32 (*EffectProcPtr)(/* parameters unknown */);
typedef struct { // 0x10
    /* 0x0 */ UInt32 Flags;
    /* 0x4 */ SInt16 delta_counter;
    /* 0x6 */ SInt16 delta_type;
    /* 0x8 */ struct EffectChain *next;
    /* 0xc */ EffectProcPtr proc;
} EffectChain;

typedef EffectChain *EffectChainPtr;

struct GenericSoundHandler { // 0x34
    /* 0x00 */ UInt32 OwnerID;
    /* 0x04 */ SoundPtr Sound;
    /* 0x08 */ EffectChainPtr Effects;
    /* 0x0c */ SInt16 Original_Vol;
    /* 0x0e */ SInt16 Original_Pan;
    /* 0x10 */ SInt16 Current_Vol;
    /* 0x12 */ SInt16 Current_Pan;
    /* 0x14 */ SInt16 Current_PM;
    /* 0x16 */ UInt8 flags;
    /* 0x17 */ SInt8 VolGroup;
    /* 0x18 */ VoiceFlags Voices;
    /* 0x20 */ struct GenericSoundHandler *prev;
    /* 0x24 */ struct GenericSoundHandler *next;
    /* 0x28 */ struct GenericSoundHandler *parent;
    /* 0x2c */ struct GenericSoundHandler *first_child;
    /* 0x30 */ struct GenericSoundHandler *siblings;
};

typedef struct GenericSoundHandler GSoundHandler;
typedef struct GenericSoundHandler *GSoundHandlerPtr;

typedef struct { // 0x14
    /* 0x00 */ UInt32 proc_id;
    /* 0x04 */ SInt32 num_funcs;
    /* 0x08 */ UInt32 reserved1;
    /* 0x0c */ UInt32 reserved2;
    /* 0x10 */ Extern989Proc procs[1];
} Extern989Handler;

typedef Extern989Handler *Extern989HandlerPtr;

BOOL snd_RegisterExternProcHandler(Extern989HandlerPtr hand);
void snd_UnloadBank(SoundBankPtr bank);
void snd_ResolveBankXREFS();
void snd_SetPlaybackMode(SInt32 mode);

SInt32 snd_GetFreeSPUDMA();
void snd_FreeSPUDMA(SInt32 ch);

#endif // _989SND_H_

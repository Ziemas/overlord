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

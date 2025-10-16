#include "common.h"

#include <libcdvd.h>

static int sNumFiles = 0;
static int sArea1 = 0;
static int sAreaDiff = 0;

void FS_Init();
void FS_Find();
void FS_FindIN();
void FS_GetLength();
void FS_Open();
void FS_OpenWad();
void FS_Close();
void FS_PageBeginRead();
void FS_SyncRead();
void FS_LoadSoundBank();
void FS_StoreSoundBankInIOP();
void FS_LoadSoundBankFromIOP();
void FS_LoadSoundBankFromEE();
void FS_LoadMusic();
void FS_PollDrive();

struct fs_api iso_cd = {
    FS_Init,
    FS_Find,
    FS_FindIN,
    FS_GetLength,
    FS_Open,
    FS_OpenWad,
    FS_Close,
    FS_PageBeginRead,
    FS_SyncRead,
    FS_LoadSoundBank,
    FS_StoreSoundBankInIOP,
    FS_LoadSoundBankFromIOP,
    FS_LoadSoundBankFromEE,
    FS_LoadMusic,
    FS_PollDrive,
};

static sceCdRMode sStreamMode = {15, SCECdSpinNom, 0, 0};
static sceCdRMode *sMode = &sStreamMode;
static int sReadInfo = 0;

int SubBufferToRead = -1;
int ReadPagesPagePool = 0;
int ReadPagesCurrentPage = 0;
int ReadPagesSectorsPerPage = 0;
int ReadPagesCurrentSector = 0;
int ReadPagesCurrentBuffer = 0;
int ReadPagesNumToRead = 0;
int ReadPagesDoneFlag = 0;
int ReadPagesCancelRead = 0;

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/iso_cd", IsoCdPagesCallBack);
#else
void IsoCdPagesCallback(int arg0) {}
#endif

INCLUDE_RODATA("asm/nonmatchings/iso_cd", D_000122C0);

INCLUDE_ASM("asm/nonmatchings/iso_cd", ReadDirectory);

INCLUDE_ASM("asm/nonmatchings/iso_cd", DecodeDUP);

INCLUDE_ASM("asm/nonmatchings/iso_cd", LoadMusicTweaks);

INCLUDE_ASM("asm/nonmatchings/iso_cd", LoadDiscID);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_Init);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_Open);

INCLUDE_RODATA("asm/nonmatchings/iso_cd", D_000124E8);

INCLUDE_RODATA("asm/nonmatchings/iso_cd", D_00012530);

INCLUDE_RODATA("asm/nonmatchings/iso_cd", D_0001256C);

INCLUDE_ASM("asm/nonmatchings/iso_cd", DecompressBlock);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_PageBeginRead);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_LoadSoundBank);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_LoadMusic);

INCLUDE_ASM("asm/nonmatchings/iso_cd", CD_WaitReturn);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_Find);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_FindIN);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_GetLength);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_OpenWad);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_Close);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_SyncRead);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_StoreSoundBankInIOP);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_LoadSoundBankFromIOP);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_LoadSoundBankFromEE);

INCLUDE_ASM("asm/nonmatchings/iso_cd", FS_PollDrive);

INCLUDE_ASM("asm/nonmatchings/iso_cd", CdReturn);

INCLUDE_ASM("asm/nonmatchings/iso_cd", DoCdReadPages);

INCLUDE_ASM("asm/nonmatchings/iso_cd", CheckPagesReady);

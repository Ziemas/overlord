#include "common.h"

INCLUDE_ASM("asm/nonmatchings/iso_cd", IsoCdPagesCallBack);

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

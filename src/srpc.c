#include "common.h"

#include "iso_api.h"
#include "overlord.h"
#include "sbank.h"

#include <string.h>
#include <thread.h>

int gFrameNum = -1;
unsigned int gInfoEE = 0;
int gFreeMem = 0;
int gNoCD = 0;
int gDirtyCD = 0;
int gDiskSpeed[2] = {0, 0};
int gDupSeg = -1;
int gFPS = 60;
int MasterVolume[17] = {
    0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400,
    0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400,
};
int gMusicFade = 0;
int gMusicFadeDir = 0;
int gMusicTweak = 0;
int gMusicPause = 0;
int gSoundEnable = 0;
int gSoundInUse = 0;

INCLUDE_ASM("asm/nonmatchings/srpc", RPC_Player);

void *RPC_Loader(unsigned int fno, void *data, int size) {
    struct SoundRpcCommand *cmd = data;
    int count;

    static char *languages[] = {
        "ENG", "FRE", "GER", "SPA", "ITA", "JAP", "KOR", "UKE",
    };

    count = size / 0x50;
    if (!gSoundEnable) {
        return NULL;
    }

    for (; count > 0; count--, cmd++) {
        void *p = cmd;
        switch (cmd->command) {
        case SCMD_LOAD_BANK: {
            struct SoundRpcBankCommand *load_bank = p;
            struct BankRecord *bank;
            bank = LookupBank(load_bank->bank_name);
            if (bank) {
                break;
            }
            bank = AllocateBankName(load_bank->bank_name);
            if (!bank) {
                break;
            }
            strncpy(bank->name, load_bank->bank_name, 16);
            bank->used = 1;
            bank->unk30 = 0;
            LoadSoundBank(load_bank->bank_name, bank);
        } break;

        case SCMD_UNLOAD_BANK: {
            struct SoundRpcBankCommand *load_bank = p;
            struct BankRecord *bank;
            SoundBankPtr handle;
            bank = LookupBank(load_bank->bank_name);
            if (!bank) {
                break;
            }
            handle = bank->bank_handle;
            bank->bank_handle = 0;
            if (!bank->unk30) {
                bank->used = 0;
            }
            snd_UnloadBank(handle);
            snd_ResolveBankXREFS();

        } break;

        case SCMD_LOAD_MUSIC: {
            struct SoundRpcBankCommand *load_bank = p;
            while (WaitSema(gSema))
                ;
            if (gMusic) {
                UnLoadMusic(&gMusic);
            }
            LoadMusic(load_bank->bank_name, &gMusic);
            SignalSema(gSema);
        } break;

        case SCMD_UNLOAD_MUSIC: {
            struct SoundRpcBankCommand *load_bank = p;
            while (WaitSema(gSema))
                ;

            if (gMusic) {
                UnLoadMusic(&gMusic);
            }

            SignalSema(gSema);
        } break;
        case SCMD_GET_IRX_VERSION: {
            struct SoundRpcGetIrxVersion *irx_version = p;
            irx_version->major = 4;
            irx_version->minor = 0;
            gInfoEE = irx_version->ee_addr;
            return p;
        } break;
        case SCMD_SET_LANGUAGE: {
            struct SoundRpcSetLanguageCommand *set_language = p;
            gLanguage = languages[set_language->language_id];
        } break;
        case SCMD_SET_STEREO_MODE: {
            struct SoundRpcStereoMode *stereo_mode = p;
            int pmode;
            switch (stereo_mode->stereo_mode) {
            case 0:
                pmode = 1;
                break;
            case 1:
                pmode = 2;
                break;
            case 2:
                pmode = 0;
                break;
            default:
                continue;
                break;
            }

            snd_SetPlaybackMode(pmode);
        } break;

        default:
            break;
        }
    }

    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/srpc", VBlank_Handler);

INCLUDE_ASM("asm/nonmatchings/srpc", Thread_Player);

INCLUDE_ASM("asm/nonmatchings/srpc", Thread_Loader);

INCLUDE_ASM("asm/nonmatchings/srpc", SetVagStreamName);

INCLUDE_ASM("asm/nonmatchings/srpc", SetVagName);

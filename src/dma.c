#include "989snd.h"
#include "common.h"
#include "intrman.h"
#include "kerror.h"
#include "sif.h"
#include "vag.h"

#include <libsd.h>

int EeDmaSema = 0;
volatile int SpuDmaStatus = 0;
struct VagCmd *DmaVagCmd;
struct VagCmd *DmaStereoVagCmd;
static sceSifDmaData dma;

int SpuDmaIntr(int, void *);

void EeDmaIntr() { iSignalSema(EeDmaSema); }

void DMA_SendToEE(u8 *iop_mem, u32 size, u32 ee_addr) {
    struct SemaParam sema;
    int oldintr;
    int ret;

    if (!size) {
        return;
    }

    if (!EeDmaSema) {
        sema.attr = 1;
        sema.initCount = 0;
        sema.maxCount = 1;
        sema.option = 0;
        EeDmaSema = CreateSema(&sema);
    } else {
        if (PollSema(EeDmaSema) == KE_SEMA_ZERO)
            WaitSema(EeDmaSema);
    }

    dma.data = (u32)iop_mem;
    dma.addr = ee_addr;
    dma.size = size;
    dma.mode = 0;

    CpuSuspendIntr(&oldintr);
    ret = sceSifSetDmaIntr(&dma, 1, EeDmaIntr, NULL);
    CpuResumeIntr(oldintr);

    if (!ret) {
        while (1)
            ;
    }
}

void DmaCancelThisVagCmd(struct VagCmd *cmd) {
    int channel;

    if (DmaVagCmd != cmd) {
        return;
    }

    sceSdSetTransIntrHandler(DmaVagCmd->transfer_channel, NULL, NULL);
    channel = DmaVagCmd->transfer_channel;
    if (channel >= 0) {
        snd_FreeSPUDMA(channel);
    }
    DmaVagCmd = NULL;
    DmaStereoVagCmd = NULL;
    SpuDmaStatus = 0;
}

int SpuDmaIntr(int channel, void *userdata) {
    int kon, koff;

    if (SpuDmaStatus != 1) {
        return 0;
    }

    if (DmaVagCmd) {
        if (DmaStereoVagCmd) {
            if (DmaStereoVagCmd->data_left) {
                if (DmaStereoVagCmd->buffers_count & 1) {
                    sceSdVoiceTrans(DmaVagCmd->transfer_channel, 0, DmaStereoVagCmd->iop_dma_mem,
                                    DmaStereoVagCmd->spu_base_addr + 0x2000, DmaStereoVagCmd->data_left);
                } else {
                    sceSdVoiceTrans(DmaVagCmd->transfer_channel, 0, DmaStereoVagCmd->iop_dma_mem,
                                    DmaStereoVagCmd->spu_base_addr, DmaStereoVagCmd->data_left);
                }

                DmaStereoVagCmd->data_left = 0;
                DmaStereoVagCmd->iop_dma_mem = NULL;
                return 0;
            }

            if (DmaVagCmd->buffers_count & 1) {
                DmaVagCmd->status[18] = 1;
                DmaStereoVagCmd->status[18] = 1;
            } else {
                DmaVagCmd->status[19] = 1;
                DmaStereoVagCmd->status[19] = 1;
            }
        } else {
            if (DmaVagCmd->buffers_count & 1) {
                DmaVagCmd->status[18] = 1;
            } else {
                DmaVagCmd->status[19] = 1;
            }
        }

        if (DmaVagCmd->buffers_count == 1) {
            int pitch = CalculateVAGPitch(DmaVagCmd->base_pitch, DmaVagCmd->new_pitch);
            if (DmaStereoVagCmd) {
                DmaVagCmd->resume_addr = 0;
                DmaStereoVagCmd->resume_addr = 0;
                sceSdSetAddr(SD_VA_SSA | DmaVagCmd->voice, DmaVagCmd->spu_base_addr + 48);
                sceSdSetAddr(SD_VA_SSA | DmaStereoVagCmd->voice, DmaStereoVagCmd->spu_base_addr + 48);
                sceSdSetParam(SD_VP_ADSR1 | DmaVagCmd->voice, 0xf);
                sceSdSetParam(SD_VP_ADSR1 | DmaStereoVagCmd->voice, 0xf);
                sceSdSetParam(SD_VP_ADSR2 | DmaVagCmd->voice, 0x1fc0);
                sceSdSetParam(SD_VP_ADSR2 | DmaStereoVagCmd->voice, 0x1fc0);
                sceSdSetParam(SD_VP_PITCH | DmaVagCmd->voice, pitch);
                sceSdSetParam(SD_VP_PITCH | DmaStereoVagCmd->voice, pitch);
                kon = (1 << (DmaVagCmd->voice >> 1));
                kon |= (1 << (DmaStereoVagCmd->voice >> 1));
            } else {
                DmaVagCmd->resume_addr = 0;
                sceSdSetAddr(SD_VA_SSA | DmaVagCmd->voice, DmaVagCmd->spu_base_addr + 48);
                sceSdSetParam(SD_VP_ADSR1 | DmaVagCmd->voice, 0xf);
                sceSdSetParam(SD_VP_ADSR2 | DmaVagCmd->voice, 0x1fc0);
                sceSdSetParam(SD_VP_PITCH | DmaVagCmd->voice, pitch);
                kon = (1 << (DmaVagCmd->voice >> 1));
            }

            sceSdSetSwitch(SD_S_KON | DmaVagCmd->voice & 1, kon);
        } else if (DmaVagCmd->buffers_count == 2) {
            DmaVagCmd->status[1] = 1;
            if (DmaStereoVagCmd) {
                DmaStereoVagCmd->status[1] = 1;
            }

            if (DmaVagCmd->status[2] == 0) {
                DmaVagCmd->status[2] = 1;
                UnPauseVAG(DmaVagCmd, 0);
            } else {
                if (DmaStereoVagCmd) {
                    sceSdSetParam(SD_VP_PITCH | DmaStereoVagCmd->voice, 0);
                    sceSdSetParam(SD_VP_PITCH | DmaVagCmd->voice, 0);
                    koff = (1 << (DmaVagCmd->voice >> 1));
                    koff |= (1 << (DmaStereoVagCmd->voice >> 1));
                } else {
                    sceSdSetParam(SD_VP_PITCH | DmaVagCmd->voice, 0);
                    koff = (1 << (DmaVagCmd->voice >> 1));
                }

                sceSdSetSwitch(SD_S_KOFF | DmaVagCmd->voice & 1, koff);
            }
        }

        DmaVagCmd->safe_to_change_dma_fields = 1;
        if (DmaStereoVagCmd) {
            DmaStereoVagCmd->safe_to_change_dma_fields = 1;
        }

        DmaVagCmd = NULL;
        DmaStereoVagCmd = NULL;
    }

    sceSdSetTransIntrHandler(channel, NULL, NULL);

    if (channel >= 0) {
        snd_FreeSPUDMA(channel);
    }

    SpuDmaStatus = 0;

    return 0;
}

bool DMA_SendToSPUAndSync(u8 *iop_mem, u32 size_one_side, u32 spu_addr, struct VagCmd *cmd, int disable_intr) {
    struct VagCmd *sibling;
    int transferred;
    int channel;
    int oldintr;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldintr);
    }

    if (SpuDmaStatus) {
        if (disable_intr == 1) {
            CpuResumeIntr(oldintr);
        }
        return false;
    }

    channel = snd_GetFreeSPUDMA();
    if (channel == -1) {
        if (disable_intr == 1) {
            CpuResumeIntr(oldintr);
        }
        return false;
    }

    DmaVagCmd = cmd;
    if (cmd) {
        sibling = cmd->sibling;
        DmaStereoVagCmd = sibling;
        cmd->safe_to_change_dma_fields = 0;
        if (sibling) {
            sibling->iop_dma_mem = iop_mem + size_one_side;
            sibling->data_left = size_one_side;
            sibling->buffers_count = cmd->buffers_count;
            cmd->transfer_channel = channel;
        }
    }

    SpuDmaStatus = 1;
    sceSdSetTransIntrHandler(channel, SpuDmaIntr, 0);
    transferred = sceSdVoiceTrans(channel, 0, iop_mem, spu_addr, size_one_side);

    if (disable_intr == 1) {
        CpuResumeIntr(oldintr);
    }

    size_one_side = ((size_one_side + 63) & ~63);
    if (transferred < size_one_side) {
        return false;
    }

    return true;
}

#include "common.h"

#include "iso_queue.h"
#include "vag.h"

#include <intrman.h>

#define BSWAP(in) ((u32)(in) >> 24) | ((in) >> 8) & 0xFF00 | ((in) << 8) & 0xFF0000 | ((in) << 24)

#ifdef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/spustreams", ProcessVAGData);
#else
int ProcessVAGData(struct VagCmd *vag, struct Buffer *buf) {
    struct VagCmd *sibling;
    int chunk_size;
    int oldintr;

    if (vag->status[6]) {
        return -1;
    }

    if (vag->status[11]) {
        return -1;
    }

    if (vag->invalid) {
        buf->decompressed_size = 0;
        return -1;
    }

    if (!vag->safe_to_change_dma_fields != 1) {
        return -1;
    }

    CpuSuspendIntr(&oldintr);
    CheckForIsoPageBoundaryCrossing(buf);
    chunk_size = 0x2000;
    if (sibling) {
        chunk_size = 0x4000;
    }

    if (vag->buffers_count == 0) {
        u8 *p = buf->decomp_buffer;
        if (*(u32 *)p != 'pGAV' && *(u32 *)p != 'VAGp') {
            vag->invalid = 1;
            buf->decompressed_size = 0;
            CpuResumeIntr(oldintr);
            return -1;
        }

        vag->sample_rate = ((u32 *)p)[3];
        vag->data_left = ((u32 *)p)[4];
        vag->unk0xcc = 0;
        if (*(u32 *)p == 'pGAV') {
            vag->sample_rate = BSWAP(vag->sample_rate);
            vag->data_left = BSWAP(vag->data_left);
        }

        if (sibling) {
            vag->sample_rate = ((u32 *)p)[3];
            vag->data_left = ((u32 *)p)[4];
            vag->unk0xcc = 0;
        }
    }

    CpuResumeIntr(oldintr);
    return -1;
}
#endif

INCLUDE_RODATA("asm/nonmatchings/spustreams", D_00013020);

INCLUDE_RODATA("asm/nonmatchings/spustreams", D_00013070);

INCLUDE_ASM("asm/nonmatchings/spustreams", GetVAGStreamPos);

INCLUDE_ASM("asm/nonmatchings/spustreams", CheckVAGStreamProgress);

INCLUDE_ASM("asm/nonmatchings/spustreams", CheckVagStreamsProgress);

INCLUDE_ASM("asm/nonmatchings/spustreams", StopVagStream);

INCLUDE_ASM("asm/nonmatchings/spustreams", UpdateIsoBuffer);

INCLUDE_ASM("asm/nonmatchings/spustreams", InitSpuStreamsThread);

INCLUDE_ASM("asm/nonmatchings/spustreams", WakeSpuStreamsUp);

INCLUDE_ASM("asm/nonmatchings/spustreams", GetSpuRamAddress);

u32 bswap(s32 in) { return ((u32)in >> 24) | (in >> 8) & 0xFF00 | (in << 8) & 0xFF0000 | (in << 24); }

INCLUDE_ASM("asm/nonmatchings/spustreams", ProcessStreamData);

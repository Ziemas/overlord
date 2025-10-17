#include <intrman.h>

#include "common.h"
#include "dma.h"
#include "iso_queue.h"
#include "vag.h"

#define BSWAP(in) ((u32)(in) >> 24) | (((s32)in) >> 8) & 0xFF00 | ((in) << 8) & 0xFF0000 | ((in) << 24)
void VAG_MarkLoopEnd(u8 *data, int offset);
void VAG_MarkLoopStart(u8 *data);

#ifdef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/spustreams", ProcessVAGData);
#else
int ProcessVAGData(struct VagCmd *vag, struct Buffer *buf) {
    struct VagCmd *sibling;
    struct Page *page;
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

    if (vag->safe_to_change_dma_fields != 1) {
        return -1;
    }

    CpuSuspendIntr(&oldintr);
    CheckForIsoPageBoundaryCrossing(buf);
    page = buf->page;
    if (page->state == PAGE_SIX && page->state != PAGE_ALLOCATED_FILLED) {
        CpuResumeIntr(oldintr);
        return -1;
    }

    page->state = PAGE_SIX;
    sibling = vag->sibling;

    chunk_size = 0x2000;
    if (vag->sibling) {
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

        vag->sample_rate = ((u32 *)p)[4];
        vag->data_left = ((u32 *)p)[3];
        vag->unk0xcc = 0;
        if (*(u32 *)p == 'pGAV') {
            vag->sample_rate = BSWAP(vag->sample_rate);
            vag->data_left = BSWAP(vag->data_left);
        }

        if (sibling) {
            sibling->sample_rate = ((u32 *)p)[4];
            sibling->data_left = ((u32 *)p)[3];
            sibling->unk0xcc = 0;
        }

        vag->unk_264 = 0x4000;
        vag->base_pitch = (vag->sample_rate << 12) / 48000;
        vag->data_left += 0x30; // account for the file header in the first block

        if (vag->data_left <= 0x2000 && vag->unk_264 >= 0x4000) {
            vag->unk_264 = (vag->data_left >= 0x20) ? vag->data_left + 0x20 : 0x10;
            if (sibling) {
                sibling->sample_rate = vag->sample_rate;
                sibling->data_left = vag->data_left;
                sibling->unk0xcc = 0;
                sibling->unk_264 = vag->unk_264;
                sibling->base_pitch = vag->base_pitch;
                sibling->data_left = vag->data_left;
                sibling->unk_264 = vag->unk_264;
            }
        }

        if (!DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr, vag, 0)) {
            CpuResumeIntr(oldintr);
            return -1;
        }

        vag->msg.ready_for_data = 0;
        vag->unk0xc4 = 0;
        vag->unk0xc8 = 0;
        sibling->unk0xc8 = 0; // unguarded dereference
        buf->decomp_buffer += chunk_size;
        if (vag->data_left < chunk_size) {
            vag->data_left = 0;
            buf->decompressed_size = 0;
            vag->buffers_count++;

            CpuResumeIntr(oldintr);
            return -1;
        }

        vag->msg.ready_for_data = 0;
        buf->decomp_buffer += chunk_size;

        vag->data_left -= chunk_size;
        buf->decompressed_size -= chunk_size;
        if (vag->data_left < chunk_size) {
            vag->data_left = 0;
            buf->decompressed_size = 0;
        }
        vag->buffers_count++;

        CpuResumeIntr(oldintr);
        return -1;
    }

    if (vag->buffers_count == 1) {
        if (vag->data_left < chunk_size && vag->unk_264 >= 0x4000) {
            int unk = 0x2010;
            if (vag->data_left >= 0x20) {
                if (sibling) {
                    unk = vag->data_left / 2 + 0x1ff0;
                } else {
                    unk = vag->data_left + 0x1ff0;
                }

                vag->unk_264 = unk;
                if (sibling) {
                    sibling->unk_264 = unk;
                }
            }
        } else {
            VAG_MarkLoopEnd(buf->decomp_buffer, 0x2000);
            VAG_MarkLoopStart(buf->decomp_buffer);
            if (sibling) {
                VAG_MarkLoopEnd(buf->decomp_buffer, 0x4000);
                VAG_MarkLoopStart(buf->decomp_buffer + 0x2000);
            }
        }

        if (!DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr + 0x2000, vag, 0)) {
            CpuResumeIntr(oldintr);
            return -1;
        }

        vag->msg.ready_for_data = 0;
        buf->decomp_buffer += chunk_size;

        vag->data_left -= chunk_size;
        buf->decompressed_size -= chunk_size;
        if (vag->data_left < chunk_size) {
            vag->data_left = 0;
            buf->decompressed_size = 0;
        }
        vag->buffers_count++;

        CpuResumeIntr(oldintr);
        return -1;
    }

    if ((vag->buffers_count & 1) == 0) {
        if (vag->data_left < chunk_size && vag->unk_264 >= 0x4000) {
            int unk = 0x2010;
            if (vag->data_left >= 0x20) {
                if (sibling) {
                    unk = vag->data_left / 2 + 0x1ff0;
                } else {
                    unk = vag->data_left + 0x1ff0;
                }

                vag->unk_264 = unk;
                if (sibling) {
                    sibling->unk_264 = unk;
                }
            }

        } else {
            VAG_MarkLoopEnd(buf->decomp_buffer, 0x2000);
            VAG_MarkLoopStart(buf->decomp_buffer);
            if (sibling) {
                VAG_MarkLoopEnd(buf->decomp_buffer, 0x4000);
                VAG_MarkLoopStart(buf->decomp_buffer + 0x2000);
            }
        }

        if (!DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr, vag, 0)) {
            CpuResumeIntr(oldintr);
            return -1;
        }

        vag->msg.ready_for_data = 0;
        buf->decomp_buffer += chunk_size;

        vag->data_left -= chunk_size;
        buf->decompressed_size -= chunk_size;
        if (vag->data_left < chunk_size) {
            vag->data_left = 0;
            buf->decompressed_size = 0;
        }
        vag->buffers_count++;

        CpuResumeIntr(oldintr);
        return -1;
    }

    if ((vag->buffers_count & 1) == 1) {
        if (vag->data_left < chunk_size && vag->unk_264 >= 0x4000) {
            int unk = 0x10;
            if (vag->data_left >= 0x20) {
                if (sibling) {
                    unk = vag->data_left / 2 - 0x10;
                } else {
                    unk = vag->data_left - 0x10;
                }

                vag->unk_264 = unk;
                if (sibling) {
                    sibling->unk_264 = unk;
                }
            }
        } else {
            VAG_MarkLoopEnd(buf->decomp_buffer, 0x2000);
            VAG_MarkLoopStart(buf->decomp_buffer);
            if (sibling) {
                VAG_MarkLoopEnd(buf->decomp_buffer, 0x4000);
                VAG_MarkLoopStart(buf->decomp_buffer + 0x2000);
            }
        }

        if (!DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr + 0x2000, vag, 0)) {
            vag->msg.ready_for_data = 0;
            CpuResumeIntr(oldintr);
            return -1;
        }

        vag->msg.ready_for_data = 0;
        buf->decomp_buffer += chunk_size;

        vag->data_left -= chunk_size;
        buf->decompressed_size -= chunk_size;
        if (vag->data_left < chunk_size) {
            vag->data_left = 0;
            buf->decompressed_size = 0;
        }
        vag->buffers_count++;

        CpuResumeIntr(oldintr);
        return -1;
    }

exit:
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

u32 bswap(s32 in) { return BSWAP(in); }

INCLUDE_ASM("asm/nonmatchings/spustreams", ProcessStreamData);

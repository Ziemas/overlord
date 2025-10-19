#include <intrman.h>

#include "common.h"
#include "dma.h"
#include "iso_queue.h"
#include "libsd.h"
#include "sysmem.h"
#include "vag.h"

int StreamsThread;

void VAG_MarkLoopEnd(u8 *data, int offset);
void VAG_MarkLoopStart(u8 *data);

int ProcessVAGData(struct VagCmd *vag, struct Buffer *buf);

void CheckVagStreamsProgress();
void UpdateIsoBuffer(struct Buffer *buf, struct VagCmd *vag, int stereo);

u32 bswap(s32 in) { return ((u32)(in) >> 24) | (((s32)in) >> 8) & 0xFF00 | ((in) << 8) & 0xFF0000 | ((in) << 24); }

void UpdateIsoBuffer(struct Buffer *buf, struct VagCmd *vag, int stereo) {
    int chunk_size;

    chunk_size = 0x2000;
    if (stereo) {
        chunk_size = 0x4000;
    }

    buf->decomp_buffer += chunk_size;
    if (chunk_size < vag->data_left) {
        vag->data_left -= chunk_size;
        buf->decompressed_size -= chunk_size;
    } else {
        vag->data_left = 0;
        buf->decompressed_size = 0;
    }

    vag->buffers_count++;
}

int ProcessVAGData(struct VagCmd *vag, struct Buffer *buf) {
    struct VagCmd *sibling;
    struct Page *page;
    int chunk_size, chunk_size2;
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
    if (page->state != PAGE_SIX) {
        if (page->state != PAGE_ALLOCATED_FILLED) {
            CpuResumeIntr(oldintr);
            return -1;
        }
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
            vag->sample_rate = bswap(vag->sample_rate);
            vag->data_left = bswap(vag->data_left);
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
            if (vag->data_left < 0x20) {
                vag->unk_264 = 0x10;
            } else {
                vag->unk_264 = vag->data_left - 0x10;
            }

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

        if (DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr, vag, 0)) {
            vag->unk0xc4 = 0;
            vag->unk0xc8 = 0;
            sibling->unk0xc8 = 0; // unguarded dereference

            UpdateIsoBuffer(buf, vag, sibling != 0);
        }
    } else if (vag->buffers_count == 1) {
        if (chunk_size >= vag->data_left && vag->unk_264 >= 0x4000) {
            if (vag->data_left < 0x20) {
                vag->unk_264 = 0x2010;
            } else {
                if (sibling) {
                    vag->unk_264 = vag->data_left / 2 + 0x1ff0;
                } else {
                    vag->unk_264 = vag->data_left + 0x1ff0;
                }
            }

            if (sibling) {
                sibling->unk_264 = vag->unk_264;
            }
        } else {
            VAG_MarkLoopEnd(buf->decomp_buffer, 0x2000);
            VAG_MarkLoopStart(buf->decomp_buffer);
            if (sibling) {
                VAG_MarkLoopEnd(buf->decomp_buffer, 0x4000);
                VAG_MarkLoopStart(buf->decomp_buffer + 0x2000);
            }
        }

        if (DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr + 0x2000, vag, 0)) {
            vag->msg.ready_for_data = 0;
            UpdateIsoBuffer(buf, vag, sibling != 0);
        }
    } else if ((vag->buffers_count & 1)) {
        if (chunk_size >= vag->data_left && vag->unk_264 >= 0x4000) {
            if (vag->data_left < 0x20) {
                vag->unk_264 = 0x2010;
            } else {
                if (sibling) {
                    vag->unk_264 = vag->data_left / 2 + 0x1ff0;
                } else {
                    vag->unk_264 = vag->data_left + 0x1ff0;
                }
            }
            if (sibling) {
                sibling->unk_264 = vag->unk_264;
            }
        } else {
            VAG_MarkLoopEnd(buf->decomp_buffer, 0x2000);
            VAG_MarkLoopStart(buf->decomp_buffer);
            if (sibling) {
                VAG_MarkLoopEnd(buf->decomp_buffer, 0x4000);
                VAG_MarkLoopStart(buf->decomp_buffer + 0x2000);
            }
        }

        if (DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr + 0x2000, vag, 0)) {
            vag->msg.ready_for_data = 0;
            UpdateIsoBuffer(buf, vag, sibling != 0);
        }
    } else {
        if (chunk_size >= vag->data_left && vag->unk_264 >= 0x4000) {
            if (vag->data_left < 0x20) {
                vag->unk_264 = 0x10;
            } else {
                if (sibling) {
                    vag->unk_264 = vag->data_left / 2 - 0x10;
                } else {
                    vag->unk_264 = vag->data_left - 0x10;
                }
            }
            if (sibling) {
                sibling->unk_264 = vag->unk_264;
            }
        } else {
            VAG_MarkLoopEnd(buf->decomp_buffer, 0x2000);
            VAG_MarkLoopStart(buf->decomp_buffer);
            if (sibling) {
                VAG_MarkLoopEnd(buf->decomp_buffer, 0x4000);
                VAG_MarkLoopStart(buf->decomp_buffer + 0x2000);
            }
        }

        if (DMA_SendToSPUAndSync(buf->decomp_buffer, 0x2000, vag->spu_base_addr, vag, 0)) {
            vag->msg.ready_for_data = 0;
            UpdateIsoBuffer(buf, vag, sibling != 0);
        }
    }

    CpuResumeIntr(oldintr);
    return -1;
}

INCLUDE_RODATA("asm/nonmatchings/spustreams", D_00013020);

INCLUDE_RODATA("asm/nonmatchings/spustreams", D_00013070);

INCLUDE_ASM("asm/nonmatchings/spustreams", GetVAGStreamPos);

INCLUDE_ASM("asm/nonmatchings/spustreams", CheckVAGStreamProgress);

INCLUDE_ASM("asm/nonmatchings/spustreams", CheckVagStreamsProgress);

INCLUDE_ASM("asm/nonmatchings/spustreams", StopVagStream);

void InitSpuStreamsThread() {
    struct ThreadParam th;

    th.attr = TH_C;
    th.entry = CheckVagStreamsProgress;
    th.initPriority = 50;
    th.stackSize = 0x800;
    th.option = 0;

    StreamsThread = CreateThread(&th);

    if (StreamsThread <= 0) {
        Kprintf("IOP: ======================================================================\n");
        Kprintf("IOP: spustreams InitSpuStreamsThread: Cannot create streams thread\n");
        Kprintf("IOP: ======================================================================\n");
        while (1)
            ;
    }

    StartThread(StreamsThread, 0);
}

void WakeSpuStreamsUp() { WakeupThread(StreamsThread); }

// INCLUDE_ASM("asm/nonmatchings/spustreams", GetSpuRamAddress);
u32 GetSpuRamAddress(struct VagCmd *cmd) {
    int voice = cmd->voice;
    u32 a1, a2, a3;
    u32 base = cmd->spu_base_addr;
    u32 address;

    do {
        do {
            a1 = sceSdGetAddr(SD_VA_NAX | voice);
            a2 = sceSdGetAddr(SD_VA_NAX | voice);
            a3 = sceSdGetAddr(SD_VA_NAX | voice);

            if (a1 == a2) {
                address = a1;
            }
            if (a2 == a3) {
                address = a2;
            }

        } while (address < base);
    } while (base + 0x4040 <= address);

    // while (1) {
    //     a1 = sceSdGetAddr(SD_VA_NAX | voice);
    //     a2 = sceSdGetAddr(SD_VA_NAX | voice);
    //     a3 = sceSdGetAddr(SD_VA_NAX | voice);

    //    if (a1 == a2) {
    //        address = a1;
    //        break;
    //    }

    //    if (a2 == a3) {
    //        address = a2;
    //        break;
    //    }

    //    if (address >= base && address < base + 0x4040) {
    //        break;
    //    }
    //}

    return address;
}

void ProcessStreamData() {
    struct PriStackEntry *ent;
    struct iso_message *msg;
    struct VagCmd *vag;
    struct Buffer *buf;
    int i;

    ent = &gPriStack[3];

    if (ent->count >= 8) {
        return;
    }

    for (i = ent->count - 1; i >= 0; i--) {
        msg = ent->entries[i];

        if (!msg) {
            continue;
        }

        if (msg->status == -1 && msg->ready_for_data && msg->cb_buf && msg->callback == ProcessVAGData) {
            buf = msg->cb_buf;
            vag = (struct VagCmd *)msg;

            msg->status = msg->callback(msg, buf);
            if (buf->decompressed_size == 0 && vag->safe_to_change_dma_fields == 1) {
                msg->cb_buf = buf->next;
                FreeBuffer(buf, 1);
            }

            if (msg->status == -1) {
                continue;
            }

            if (vag->safe_to_change_dma_fields == 1) {
                ReleaseMessage(msg, 1);
            }
        }

        if (msg->status != -1) {
            ReleaseMessage(msg, 1);
        }
    }
}

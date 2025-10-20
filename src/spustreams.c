#include "common.h"
#include "dma.h"
#include "iso_queue.h"
#include "srpc.h"
#include "streamlfo.h"
#include "streamlist.h"
#include "vag.h"

#include <intrman.h>
#include <libsd.h>
#include <string.h>
#include <sysmem.h>

int ActiveVagStreams;

void VAG_MarkLoopEnd(u8 *data, int offset);
void VAG_MarkLoopStart(u8 *data);

int ProcessVAGData(struct VagCmd *vag, struct ISOBuffer *buf);

void CheckVagStreamsProgress();
void UpdateIsoBuffer(struct ISOBuffer *buf, struct VagCmd *vag, int stereo);
int GetVAGStreamPos(struct VagCmd *vag);

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

u32 bswap(s32 in) { return ((u32)(in) >> 24) | (((s32)in) >> 8) & 0xFF00 | ((in) << 8) & 0xFF0000 | ((in) << 24); }

void UpdateIsoBuffer(struct ISOBuffer *buf, struct VagCmd *vag, int stereo) {
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

int ProcessVAGData(struct VagCmd *vag, struct ISOBuffer *buf) {
    struct VagCmd *sibling;
    struct Page *page;
    int chunk_size, chunk_size2;
    int oldstat;

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

    CpuSuspendIntr(&oldstat);
    CheckForIsoPageBoundaryCrossing(buf);
    page = buf->page;
    if (page->state != PAGE_SIX) {
        if (page->state != PAGE_ALLOCATED_FILLED) {
            CpuResumeIntr(oldstat);
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
            CpuResumeIntr(oldstat);
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
            vag->playPos = 0;
            vag->vagClock_unk2 = 0;
            sibling->vagClock_unk2 = 0; // unguarded dereference

            UpdateIsoBuffer(buf, vag, sibling != 0);
        }
    } else if (vag->buffers_count == 1) {
        if (chunk_size >= vag->data_left && vag->unk_264 >= 0x4000) {
            if (vag->data_left < 0x20) {
                vag->unk_264 = 0x2010;
            } else {
                if (sibling) {
                    vag->unk_264 = 0x2010 + vag->data_left / 2 - 0x20;
                } else {
                    vag->unk_264 = 0x2010 + vag->data_left - 0x20;
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
                    vag->unk_264 = 0x2010 + vag->data_left / 2 - 0x20;
                } else {
                    vag->unk_264 = 0x2010 + vag->data_left - 0x20;
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
                    vag->unk_264 = 0x10 + vag->data_left / 2 - 0x20;
                } else {
                    vag->unk_264 = 0x10 + vag->data_left - 0x20;
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

    CpuResumeIntr(oldstat);
    return -1;
}

u32 GetSpuRamAddress(struct VagCmd *cmd) {
    u32 base = cmd->spu_base_addr;
    u32 end = cmd->spu_base_addr + 0x4040;
    int voice = cmd->voice;
    u32 a1, a2, a3;
    u32 address;

    while (1) {
        address = 0;
        while (!address) {
            a1 = sceSdGetAddr(SD_VA_NAX | voice);
            a2 = sceSdGetAddr(SD_VA_NAX | voice);
            a3 = sceSdGetAddr(SD_VA_NAX | voice);

            if (a1 == a2) {
                address = a1;
            } else if (a2 == a3) {
                address = a2;
            } else if (a1 == a3) {
                address = a1;
            }
        }

        if (address >= base && address < end) {
            break;
        }
    }

    return address;
}

int GetVAGStreamPos(struct VagCmd *vag) {
    struct VagCmd *sibling;
    u32 addr1, addr2;
    int vagclock;
    int oldstat;
    u32 sample_rate;

    sibling = vag->sibling;
    if (!vag->id) {
        vag->vagClock_unk2 = 0;
        if (sibling) {
            sibling->vagClock_unk2 = 0;
        }
        return 0;
    }

    if (vag->status[6]) {
        vag->vagClock_unk2 = vag->unk0xc0;
        if (sibling) {
            sibling->vagClock_unk2 = sibling->unk0xc0;
        }

        return 0;
    }

    if (!vag->status[4] || !vag->status[1] || vag->status[2]) {
        vag->vagClock_unk2 = vag->vagClock_unk1;
        if (sibling) {
            sibling->vagClock_unk2 = sibling->vagClock_unk1;
        }

        return 0;
    }

    if (vag->status[11]) {
        vag->vagClock_unk2 = vag->vagClock_unk1;
        return 0;
    }

    CpuSuspendIntr(&oldstat);
    addr1 = GetSpuRamAddress(vag) - vag->spu_base_addr;
    if (sibling) {
        addr2 = GetSpuRamAddress(sibling) - sibling->spu_base_addr;
    } else {
        addr2 = 0;
    }
    CpuResumeIntr(oldstat);

    if (sibling && addr1 < 0x4000 && addr2 < 0x4000 && !vag->status[20] && !sibling->status[20]) {
        if (abs((s32)((addr1 - addr2) << 18) >> 18) >= 5) {
            PauseVAG(vag, 1);
            addr1 = vag->resume_addr - vag->spu_base_addr;
            addr2 = sibling->resume_addr - sibling->spu_base_addr;
            UnPauseVAG(vag, 1);
        }
    }

    if (sibling) {
        CpuSuspendIntr(&oldstat);
        if (addr1 > 0x4000 && !vag->status[20]) {
            vag->status[20] = 1;
            vag->status[21] = 0;
            vag->status[22] = 0;
            sibling->status[20] = 1;
            sibling->status[21] = 0;
            sibling->status[22] = 0;
        }

        if (addr2 > 0x4000) {
            if (!sibling->status[20]) {
                vag->status[20] = 1;
                vag->status[21] = 0;
                vag->status[22] = 0;
                sibling->status[20] = 1;
                sibling->status[21] = 0;
                sibling->status[22] = 0;
            }
        } else {
            if (addr1 < 0x2000) {
                if (vag->status[0x15] == 0) {
                    vag->status[0x15] = 1;
                    vag->status[0x16] = 0;
                    vag->status[0x14] = 0;
                    vag->unk0xcc += 1;
                }
            } else {
                if (vag->status[0x16] == 0) {
                    vag->status[0x16] = 1;
                    vag->status[0x15] = 0;
                    vag->status[0x14] = 0;
                    vag->unk0xcc += 1;
                }
            }

            if (addr2 < 0x2000) {
                if (sibling->status[0x15] == 0) {
                    sibling->status[0x15] = 1;
                    sibling->status[0x16] = 0;
                    sibling->status[0x14] = 0;
                    sibling->unk0xcc += 1;
                }
            } else if (sibling->status[0x16] == 0) {
                sibling->status[0x16] = 1;
                sibling->status[0x15] = 0;
                sibling->status[0x14] = 0;
                sibling->unk0xcc += 1;
            }
        }

        CpuResumeIntr(oldstat);

        switch (vag->unkec) {
        case 0:
            if (!vag->status[18] || !vag->status[21] || !sibling->status[18] || !sibling->status[21]) {
                break;
            }

            vag->status[18] = 0;
            sibling->status[18] = 0;
            vag->unkec = 2;
            sibling->unkec = 2;
        case 2:
            if (vag->status[19] && sibling->status[19]) {
                if (vag->status[20] || sibling->status[20]) {
                    addr1 = 0x2000;
                    addr2 = 0x2000;
                    RestartVag(vag, 1, 1);
                    vag->unkec = 9;
                    sibling->unkec = 9;
                    break;
                }

                CpuSuspendIntr(&oldstat);
                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->spu_base_addr + 0x2000);
                sceSdSetAddr(SD_VA_LSAX | sibling->voice, sibling->spu_base_addr + 0x2000);
                vag->status[15] = 1;
                vag->status[14] = 0;
                vag->status[13] = 0;
                sibling->status[15] = 1;
                sibling->status[14] = 0;
                sibling->status[13] = 0;

                vag->unkec = 3;
                sibling->unkec = 3;
                CpuResumeIntr(oldstat);
                break;
            }

            if (vag->status[20] || sibling->status[20]) {
                addr1 = 0x2000;
                addr2 = 0x2000;
                vag->status[17] = 1;
                vag->status[16] = 0;
                sibling->status[17] = 1;
                sibling->status[16] = 0;
                vag->unkec = 4;
                sibling->unkec = 4;
            }
            break;
        case 3:
            if (vag->status[20] || sibling->status[20]) {
                addr1 = 0x2000;
                addr2 = 0x2000;
                RestartVag(vag, 1, 1);
                vag->unkec = 9;
                sibling->unkec = 9;
                break;
            }

            if (vag->status[22] && sibling->status[22]) {
                CpuSuspendIntr(&oldstat);

                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->trap_addr);
                sceSdSetAddr(SD_VA_LSAX | sibling->voice, sibling->trap_addr);

                vag->status[13] = 1;
                vag->status[14] = 0;
                vag->status[15] = 0;
                sibling->status[13] = 1;
                sibling->status[14] = 0;
                sibling->status[15] = 0;
                vag->status[19] = 0;
                sibling->status[19] = 0;
                vag->unkec = 5;
                sibling->unkec = 5;
                CpuResumeIntr(oldstat);
            }
            break;
        case 4:
            addr1 = vag->playPos;
            addr2 = sibling->playPos;
            if (vag->status[19] && sibling->status[19]) {
                RestartVag(vag, 1, 1);
                vag->unkec = 9;
                sibling->unkec = 9;
            }
            break;
        case 9:
            if (vag->status[22] && sibling->status[22]) {
                vag->status[17] = 0;
                sibling->status[17] = 0;
                vag->unkec = 3;
                sibling->unkec = 3;
            } else {
                addr1 = vag->playPos;
                addr2 = sibling->playPos;
            }
            break;
        case 5:
            if (vag->status[18] && sibling->status[18]) {
                if (vag->status[20] || sibling->status[20]) {
                    addr1 = 0x4000;
                    addr2 = 0x4000;
                    RestartVag(vag, 0, 1);
                    vag->unkec = 8;
                    sibling->unkec = 8;
                    break;
                }
                CpuSuspendIntr(&oldstat);

                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->spu_base_addr);
                sceSdSetAddr(SD_VA_LSAX | sibling->voice, sibling->spu_base_addr);
                vag->status[14] = 1;
                vag->status[15] = 0;
                vag->status[13] = 0;
                sibling->status[14] = 1;
                sibling->status[15] = 0;
                sibling->status[13] = 0;
                vag->unkec = 6;
                sibling->unkec = 6;
                CpuResumeIntr(oldstat);
                break;
            }
            if (vag->status[20]) {
                addr1 = 0x4000;
                addr2 = 0x4000;
                vag->status[16] = 1;
                vag->status[17] = 0;
                sibling->status[16] = 1;
                sibling->status[17] = 0;
                vag->unkec = 7;
                sibling->unkec = 7;
                break;
            }
            break;
        case 6:
            if (vag->status[20] || sibling->status[20]) {
                addr1 = 0x4000;
                addr2 = 0x4000;
                RestartVag(vag, 0, 1);
                vag->unkec = 8;
                sibling->unkec = 8;
                break;
            }
            if (vag->status[21] && sibling->status[21]) {
                CpuSuspendIntr(&oldstat);

                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->trap_addr);
                sceSdSetAddr(SD_VA_LSAX | sibling->voice, sibling->trap_addr);
                vag->status[13] = 1;
                vag->status[14] = 0;
                vag->status[15] = 0;
                sibling->status[13] = 1;
                sibling->status[14] = 0;
                sibling->status[15] = 0;
                vag->status[18] = 0;
                sibling->status[18] = 0;
                vag->unkec = 2;
                sibling->unkec = 2;
                CpuResumeIntr(oldstat);
            }
            break;
        case 7:
            addr1 = vag->playPos;
            addr2 = addr1;
            if (vag->status[18] && sibling->status[18]) {
                RestartVag(vag, 0, 1);
                vag->unkec = 8;
                sibling->unkec = 8;
                break;
            }
            break;
        case 8:
            if (vag->status[21]) {
                if (sibling->status[21]) {
                    vag->status[16] = 0;
                    sibling->status[16] = 0;
                    // v24 = 6;
                    // goto LABEL_114;
                    vag->unkec = 6;
                    sibling->unkec = 6;
                    break;
                }
            }
            addr1 = vag->playPos;
            addr2 = addr1;
            break;
        }

        if (vag->unk0xcc) {
            vag->vagClockS = addr1 + ((vag->unk0xcc - 1) << 13);
            sibling->vagClockS = addr2 + ((sibling->unk0xcc - 1) << 13);
            if (addr1 > 0x2000) {
                vag->vagClockS -= 0x2000;
            }
            if (addr2 > 0x2000) {
                sibling->vagClockS -= 0x2000;
            }
        } else {
            vag->vagClockS = addr1;
            sibling->vagClockS = addr1;
        }

        // probably like jak1?
        // vagclock = 4 * (0x1c00 * (vag->vagClockS / 16) / vag->sample_rate);

        { /* 'vagclock' needs to be a new variable. */
            int vagclock = vag->vagClockS * 0x1c0;
            if (vag->sample_rate == 0) {
                vagclock = 0;
            } else {
                vagclock = (vagclock / vag->sample_rate);
            }

            vagclock *= 4;
            vag->vagClock_unk1 = vagclock;
            vag->vagClock_unk2 = vagclock;
            vag->playPos = addr1;
        }

        { /* 'vagclock' needs to be a new variable. */
            int vagclock = sibling->vagClockS * 0x1c0;
            if (sibling->sample_rate == 0) {
                vagclock = 0;
            } else {
                vagclock = (vagclock / sibling->sample_rate);
            }

            vagclock *= 4;
            sibling->vagClock_unk1 = vagclock;
            sibling->vagClock_unk2 = vagclock;
            sibling->playPos = addr2;
        }

        // goto l_1;
    } else {
        // l_1:
        if (addr1 > 0x4000) {
            if (vag->status[20] == 0) {
                vag->status[20] = 1;
                vag->status[21] = 0;
                vag->status[22] = 0;
            }
        } else if (addr1 < 0x2000) {
            if (vag->status[21] == 0) {
                vag->status[21] = 1;
                vag->status[22] = 0;
                vag->status[20] = 0;
                vag->unk0xcc++;
            }
        } else if (vag->status[22] == 0) {
            vag->status[22] = 1;
            vag->status[21] = 0;
            vag->status[20] = 0;
            vag->unk0xcc++;
        }

        switch (vag->unkec) {
        case 0:
            if (!vag->status[18] || !vag->status[21]) {
                break;
            }
            vag->status[18] = 0;
            vag->unkec = 2;
        // goto LABEL_142;
        case 2:
            if (vag->status[19]) {
                if (vag->status[20]) {
                    addr1 = 0x2000;
                    // goto LABEL_153;
                    RestartVag(vag, 1, 1);
                    vag->unkec = 9;
                    break;
                }

                CpuSuspendIntr(&oldstat);
                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->spu_base_addr + 0x2000);
                vag->status[15] = 1;
                vag->status[14] = 0;
                vag->status[13] = 0;
                vag->unkec = 3;
                CpuResumeIntr(oldstat);
                break;
            }

            if (vag->status[20]) {
                addr1 = 0x2000;
                vag->status[17] = 1;
                // v40 = 4;
                // goto LABEL_171;
                vag->status[16] = 0;
                vag->unkec = 4;
                break;
            }
            break;
        case 3:
            if (vag->status[20]) {
                addr1 = 0x2000;
                // goto LABEL_153;
                RestartVag(vag, 1, 1);
                vag->unkec = 9;
                break;
            }

            if (vag->status[22]) {
                CpuSuspendIntr(&oldstat);
                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->trap_addr);
                vag->status[13] = 1;
                vag->status[14] = 0;
                vag->status[15] = 0;
                vag->status[19] = 0;
                // v38 = oldstat[0];
                // v39 = 5;
                // goto LABEL_167;
                vag->unkec = 5;
                CpuResumeIntr(oldstat);
            }
            break;
        case 4:
            addr1 = vag->playPos;
            if (vag->status[19]) {
                // goto LABEL_153;
                RestartVag(vag, 1, 1);
                vag->unkec = 9;
                break;
            }
            break;
        case 9:
            if (vag->status[22]) {
                vag->status[17] = 0;
                // v41 = 3;
                vag->unkec = 3;
            } else {
                addr1 = vag->playPos;
            }
            break;
        case 5:
            if (vag->status[18]) {
                if (vag->status[20]) {
                    addr1 = 0x4000;
                    // goto LABEL_169;
                    RestartVag(vag, 0, 1);
                    vag->unkec = 8;
                    break;
                }

                CpuSuspendIntr(&oldstat);
                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->spu_base_addr);
                vag->status[14] = 1;
                vag->status[15] = 0;
                vag->status[13] = 0;
                // v38 = oldstat[0];
                // v39 = 6;
                // goto LABEL_167;
                vag->unkec = 6;
                CpuResumeIntr(oldstat);
                break;
            }
            if (vag->status[20]) {
                addr1 = 0x4000;
                vag->status[16] = 1;
                vag->status[17] = 0;
                // v41 = 7;
                // goto LABEL_162;
                vag->unkec = 7;
            }
            break;
        case 6:
            if (vag->status[20]) {
                addr1 = 0x4000;
                // goto LABEL_169;
                RestartVag(vag, 0, 1);
                vag->unkec = 8;
                break;
            }
            if (vag->status[21]) {
                CpuSuspendIntr(&oldstat);
                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->trap_addr);
                vag->status[13] = 1;
                vag->status[14] = 0;
                vag->status[15] = 0;
                vag->status[18] = 0;
                // v38 = oldstat[0];
                // v39 = 2;
                vag->unkec = 2;
                CpuResumeIntr(oldstat);
            }
            break;
        case 7:
            addr1 = vag->playPos;
            if (vag->status[18]) {
                RestartVag(vag, 0, 1);
                vag->unkec = 8;
            }
            break;
        case 8:
            if (vag->status[21]) {
                vag->status[16] = 0;
                vag->unkec = 6;
                break;
            }
            addr1 = vag->playPos;
            break;
        }

        if (vag->unk0xcc) {
            vag->vagClockS = addr1 + ((vag->unk0xcc - 1) << 13);
            if (addr1 > 0x2000) {
                vag->vagClockS -= 0x2000;
            }
        } else {
            vag->vagClockS = addr1;
        }

        { /* 'vagclock' needs to be a new variable. */
            int vagclock = vag->vagClockS * 0x1c0;
            if (vag->sample_rate == 0) {
                vagclock = 0;
            } else {
                vagclock = (vagclock / vag->sample_rate);
            }

            vagclock *= 4;
            vag->vagClock_unk1 = vagclock;
            vag->vagClock_unk2 = vagclock;
            vag->playPos = addr1;
        }
    }

    return 0;
}

int CheckVAGStreamProgress(struct VagCmd *vag) {
    struct VagCmd *sibling;
    u32 unk_264, unkc4;
    int oldstat;

    if (vag->status[11]) {
        return 1;
    }

    if (vag->invalid) {
        return 0;
    }

    if (!vag->status[1]) {
        return 1;
    }

    if (vag->status[2]) {
        return 1;
    }

    sibling = vag->sibling;
    unkc4 = vag->playPos;

    if (vag->unk_264 < 0x4000 &&
        ((unkc4 <= 0x2000 && vag->unk_264 <= 0x2000) || (unkc4 >= 0x2000 && vag->unk_264 >= 0x2000))) {
        if ((unkc4 & ~0xf) >= vag->unk_264) {
            return 0;
        }

        CpuSuspendIntr(&oldstat);
        if (!vag->unk10c) {
            if (unkc4 < vag->unk_264) {
                sceSdSetAddr(SD_VA_LSAX | vag->voice, vag->spu_base_addr + vag->unk_264);
                vag->unk10c = 1;
                if (sibling) {
                    sceSdSetAddr(SD_VA_LSAX | sibling->voice, sibling->spu_base_addr + vag->unk_264);
                    sibling->unk10c = 1;
                }
            }
        }

        vag->msg.ready_for_data = 0;
        CpuResumeIntr(oldstat);
        return 1;
    }

    if (!vag->status[1]) {
        return 1;
    }

    if (!vag->msg.ready_for_data && vag->safe_to_change_dma_fields == 1 && !vag->unk10c) {
        if (unkc4 < 0x2000) {
            if ((vag->buffers_count & 1)) {
                vag->msg.ready_for_data = 1;
            }
        } else {
            if ((vag->buffers_count & 1) == 0) {
                vag->msg.ready_for_data = 1;
            }
        }
    }

    return 1;
}

void StopVagStream(struct VagCmd *vag, int disable_intr) {
    struct VagStreamData vstream;
    struct LfoListData lfo;
    struct VagCmd *sibling;
    int oldstat;

    if (disable_intr == 1) {
        CpuSuspendIntr(&oldstat);
    }

    sibling = vag->sibling;

    vag->status[1] = 0;
    if (sibling) {
        sibling->status[1] = 0;
    }

    if (vag->sound_handler) {
        PauseVAG(vag, 0);
        strncpy(vstream.name, vag->name, 48);
        vstream.id = vag->id;
        RemoveVagStreamFromList(&vstream, &PluginStreamsList);
        RemoveVagStreamFromList(&vstream, &EEPlayList);
        lfo.unk0x24 = vag->id;
        lfo.unk0x28 = vag->plugin_id;
        RemoveLfoStreamFromList(&lfo, &LfoList);
    } else {
        PauseVAG(vag, 0);
        vag->status[9] = 1;
        if (sibling) {
            PauseVAG(sibling, 0);
            // BUG ??
            vag->status[9] = 1;
        }
    }

    if (disable_intr == 1) {
        CpuResumeIntr(oldstat);
    }
}

void ProcessStreamData() {
    struct PriStackEntry *ent;
    struct ISO_Hdr *msg;
    struct VagCmd *vag;
    struct ISOBuffer *buf;
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

void WakeSpuStreamsUp() { WakeupThread(StreamsThread); }

void CheckVagStreamsProgress() {
    struct VagCmd *vag;
    int i;

    while (1) {
        ProcessStreamData();

        vag = VagCmds;
        for (i = 0; i < 4; i++) {
            if (vag->status[1] || (vag->status[4] && vag->status[6]) || (vag->msg.ready_for_data == 1 && vag->id)) {
                if (!CheckVAGStreamProgress(vag)) {
                    if (!vag->status[11]) {
                        StopVagStream(vag, 1);
                    }
                } else {
                    GetVAGStreamPos(vag);
                }
            }

            vag++;
        }

        if (ActiveVagStreams > 0) {
            DelayThread(1000);
        } else {
            SleepThread();
        }
    }
}

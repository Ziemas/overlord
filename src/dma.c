#include "common.h"

INCLUDE_ASM("asm/nonmatchings/dma", SpuDmaIntr);

INCLUDE_ASM("asm/nonmatchings/dma", DMA_SendToEE);

INCLUDE_ASM("asm/nonmatchings/dma", DMA_SendToSPUAndSync);

INCLUDE_ASM("asm/nonmatchings/dma", DmaCancelThisVagCmd);

INCLUDE_ASM("asm/nonmatchings/dma", EeDmaIntr);

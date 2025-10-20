#ifndef DMA_H_
#define DMA_H_

#include "common.h"
#include "vag.h"

bool DMA_SendToSPUAndSync(u8 *iop_mem, int size_one_side, int spu_addr, struct VagCmd *cmd, int disable_intr);

#endif // DMA_H_

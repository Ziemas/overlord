#ifndef SPUSTREAMS_H_
#define SPUSTREAMS_H_

#include "common.h"
#include "vag.h"

void WakeSpuStreamsUp();
u32 GetSpuRamAddress(struct VagCmd *cmd);

#endif // SPUSTREAMS_H_

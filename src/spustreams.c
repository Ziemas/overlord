#include "common.h"

INCLUDE_ASM("asm/nonmatchings/spustreams", ProcessVAGData);

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

INCLUDE_ASM("asm/nonmatchings/spustreams", bswap);

INCLUDE_ASM("asm/nonmatchings/spustreams", ProcessStreamData);

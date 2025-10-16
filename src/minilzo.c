#include "common.h"

INCLUDE_ASM("asm/nonmatchings/minilzo", lzo1x_decompress);

INCLUDE_ASM("asm/nonmatchings/minilzo", __lzo_init2);

INCLUDE_RODATA("asm/nonmatchings/minilzo", __lzo_copyright);

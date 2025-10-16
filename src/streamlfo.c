#include "common.h"

static int sine[128] = {
    0,     202,   403,   604,   803,   1000,  1195,  1387,  1575,  1760,  1940,  2115,  2286,  2451,  2609,  2762,
    2908,  3046,  3178,  3301,  3417,  3524,  3623,  3713,  3793,  3865,  3927,  3980,  4023,  4056,  4079,  4092,
    4096,  4089,  4073,  4046,  4010,  3964,  3908,  3843,  3768,  3685,  3592,  3490,  3380,  3262,  3136,  3002,
    2861,  2713,  2559,  2398,  2231,  2059,  1882,  1700,  1514,  1325,  1132,  937,   739,   539,   338,   137,
    -65,   -267,  -468,  -668,  -867,  -1063, -1257, -1448, -1635, -1818, -1997, -2171, -2340, -2503, -2659, -2810,
    -2953, -3090, -3219, -3340, -3452, -3557, -3653, -3740, -3818, -3886, -3945, -3995, -4034, -4064, -4084, -4095,
    -4095, -4085, -4065, -4036, -3996, -3947, -3888, -3820, -3742, -3656, -3560, -3456, -3343, -3222, -3094, -2958,
    -2814, -2664, -2508, -2345, -2176, -2002, -1824, -1641, -1454, -1263, -1069, -873,  -675,  -475,  -273,  -71,
};

static int _seed = 0xA354E767;

INCLUDE_ASM("asm/nonmatchings/streamlfo", SineLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitSineLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitRandLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitStreamLfoHandler);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RandomLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RemoveLfoStreamFromList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", CheckLfoList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", UpdateLfoVars);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RandomLfoSetPitchVars);

INCLUDE_ASM("asm/nonmatchings/streamlfo", RandomLfoWaitForPitch);

INCLUDE_ASM("asm/nonmatchings/streamlfo", SineLfoSetPitchVars);

INCLUDE_ASM("asm/nonmatchings/streamlfo", SineLfoWaitForPitch);

INCLUDE_ASM("asm/nonmatchings/streamlfo", StreamLfo);

INCLUDE_ASM("asm/nonmatchings/streamlfo", InitStreamLfoList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", AddToCircularLfoStreamList);

INCLUDE_ASM("asm/nonmatchings/streamlfo", FindLfoStreamInList);

INCLUDE_RODATA("asm/nonmatchings/streamlfo", D_00012F70);

#ifndef SSOUND_H_
#define SSOUND_H_

#include "common.h"

struct VolumePair {
  s16 left;
  s16 right;
};

extern struct VolumePair gPanTable[361];

s32 CalculateFalloffVolume(s32 *pos, s32 volume, s32 fo_curve, s32 fo_min, s32 fo_max);
s32 CalculateAngle(s32 *pos);

#endif // SSOUND_H_

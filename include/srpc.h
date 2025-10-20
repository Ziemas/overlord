#ifndef SRPC_H_
#define SRPC_H_

#include "vag.h"

extern int StreamsThread;
extern int MasterVolume[17];

void SetVagStreamName(struct VagCmd* vag, int new_name, int disable_intr);

#endif // SRPC_H_

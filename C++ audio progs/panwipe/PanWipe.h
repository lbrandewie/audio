//
// panwipe.h
//
// declares for pan wiper app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#define uint unsigned int

#include "Buffer.h"


float  calcpan(uint, uint, float, float, int, float);
void   getsafefilename(char *);
void   mixspecial(Buffer *, Buffer *, float, float, int, float);
short  clamp(float);
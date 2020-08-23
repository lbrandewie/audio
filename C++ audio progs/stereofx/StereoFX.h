//
// stereofx.h
//
// declares for stereo effects app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#define uint unsigned int

#ifndef STEREOFX_H
#define STEREOFX_H


#include "Buffer.h"
#include "Wavefile.h"


void  cv_stereo(Buffer *, Buffer *, Buffer *);
void  getsafefilename(char *);

#endif






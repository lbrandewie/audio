//
// msa.cpp
//
// mid-sides analysis app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Buffer.h"
#include "Wavefile.h"


void main(int argc, char *argv[])
{
	struct min_maxf  mm;
	float  levMid, levSides;
    
    if (argc != 2)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

	if (nosuchfile(argv[1])) exit(1);

    Wavefile wf(argv[1]);
    
	if (wf.getchannels() != 2)
	{
		printf("%s is not a stereo file.\n", argv[1]);
		exit(0);
	}

    Buffer bufMid(wf.getsamples(), wf.getsamprate(), 1, 32);
    Buffer bufSides(wf.getsamples(), wf.getsamprate(), 1, 32);
    
    wf.cv_midsides(&bufMid, &bufSides);

    bufMid.scanbuf32(&mm);
    levMid = dothemath32(&mm, (float)0.0);
    
    bufSides.scanbuf32(&mm);
    levSides = dothemath32(&mm, (float)0.0);
    
	printf("Mid level: %.3f Sides level: %.3f\n", levMid, levSides);
}



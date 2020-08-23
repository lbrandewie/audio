//
// delay.cpp
//
// implement a digital delay
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Delay.h"
#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"


void main(int argc, char *argv[])
{
	float   normlevel = (float)0.0, time;
	float   delay, level;
	uint    samples;	
	char    newfilename[1024];

	if (argc < 4)
	{
		printf("Usage: %s wavfile [delay in ms] [level] [[normlevel]]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[2], "%f", &delay);
	sscanf(argv[3], "%f", &level);

	if (argc == 5)
		sscanf(argv[4], "%f", &normlevel);
	
	if (delay < 0.00001)
	{
		printf("Must set delay value\n");
		exit(0);
	}

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

    Wavefile wf(argv[1]);
    
    time = wf.gettime() + delay / 1000;
	samples = (uint)floor(time * wf.getsamprate() + 0.5) + 1;
    
    Buffer buf(samples, wf.getsamprate(), wf.getchannels(), 32);
    
	buf.mix32(&wf, (float)0.0, (float)0.0, (float)0.0);
	buf.mix32(&wf, delay / (float)1000.0, level, (float)0.0);
	
	if ((argc == 5) && (normlevel < 0.000001))
		buf.normalize32(normlevel);

    buf.saveaswavefile32(newfilename);
    
	if (buf.getclipped())
		printf("Clipping detected, %d samples affected.\n", buf.getclipped());
}

void getsafefilename(char *fname)
{
	char  *where;
	int   x = 1;

	where = findperiod(fname);

	if (where == NULL)
		die("Cannot find filename extension, exiting\n");

	sprintf(where, "~delay1.wav");

	while (fileexists(fname))
		sprintf(where, "~delay%d.wav", ++x);
}


//
// mixer.cpp
//
// mixer app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Mixer.h"
#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"


void main(int argc, char *argv[])
{
	float offset, level, normlevel = (float)1.0, pan = (float)0.0;
	
	if (argc < 5)
	{
		printf("Usage: %s wavefile1 wavefile2 [offset] [level] [normlevel] [pan]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[3], "%f", &offset);
	sscanf(argv[4], "%f", &level);

	if (argc > 5)
		sscanf(argv[5], "%f", &normlevel);

	if (argc == 7)
		sscanf(argv[6], "%f", &pan);
	
	if (offset < 0)
		die("Brain hurts! Negative offsets not allowed!");
		
	mixwaves(argv[1], argv[2], offset, level, normlevel, pan);
}

void mixwaves(char *fname1, char *fname2, float offset, float level, float normlevel, float pan)
{
	char   newfilename[1024];
	float  time2, maxtime;
	uint   samples;

	Wavefile wf1(fname1);
    Wavefile wf2(fname2);

	strcpy(newfilename, fname1);
	getsafefilename(newfilename);

	time2 = (float)(wf2.getsamples() - 1) / wf1.getsamprate();				// note the use of wf1's samprate here
	maxtime = getmaxf(wf1.gettime(), time2 + offset);
	samples = (uint)floor(maxtime * wf1.getsamprate()) + 1;
	
    Buffer buf(samples, wf1.getsamprate(), wf1.getchannels(), 32);

    buf.mix32(&wf1, (float)0.0, (float)0.0, (float)0.0);
    buf.mix32(&wf2, offset, level, pan);
    
    if (normlevel < .000001)
		buf.normalize32(normlevel);

	buf.saveaswavefile32(newfilename);
    
	if (buf.getclipped())
		printf("Clipping detected, %d samples affected.\n", buf.getclipped());
}

void getsafefilename(char *fname)
{
	char *where;
	int   x = 1;

	where = findperiod(fname);

	if (where == NULL)
		die("Cannot find filename extension, exiting.");

	sprintf(where, "~mix1.wav");

	while (fileexists(fname))
		sprintf(where, "~mix%d.wav", ++x);
}

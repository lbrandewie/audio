//
// panwipe.cpp
//
// pan wiper app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Panwipe.h"
#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"


double PI;
uint   clipped;


void main(int argc, char *argv[])
{
	float  delay, cycletime, cycles = (float) 1;
	char   newfilename[1024];
    int    pattern = 0;

	PI = 4 * atan(1);

	if (argc < 5)
	{
		printf("Usage: %s wavefile1 [delay] [cycle time] [pattern] [[cycles]]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[2], "%f", &delay);
	sscanf(argv[3], "%f", &cycletime);

	if (argc == 6)
		sscanf(argv[5], "%f", &cycles);

	if (delay < 0)
        die("Negative delays not allowed.");
	
    lcase(argv[4]);
    
	if (strcmp(argv[4], "lr") == 0)
		pattern = 1;
	else if (strcmp(argv[4], "rl") == 0)
	    pattern = 2;
	else if (strcmp(argv[4], "cr") == 0)
		pattern = 3;
	else if (strcmp(argv[4], "cl") == 0)
		pattern = 4;

	if (pattern == 0)
	{
		printf("I don't understand pattern %s, quitting.\n", argv[4]);
		exit(0);
	}

    Wavefile wf(argv[1]);
    
    if (wf.getchannels() != 1)
        die("Source file must be mono.");

    strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

    Buffer buf(wf.getsamples(), wf.getsamprate(), 2, 16);
    mixspecial(&buf, &wf, delay, cycletime, pattern, cycles);
        
    buf.saveaswavefile(newfilename);
}

void mixspecial(Buffer *outbuf, Buffer *inbuf, float delay, float cycletime, int pattern, float cycles)		// destructively mixes inbuf into outbuf
{
	panmult  pm;
	uint     x, lim;
	float    pan;

    lim = inbuf->getsamples();
    
	for (x = 0; x < lim; x++)		// only have to handle mono into stereo case
	{
		if ((x % 100) == 0)
		{
			pan = calcpan(x, outbuf->getsamprate(), delay, cycletime, pattern, cycles);
			dopancalcs(pan, &pm);
		}
        outbuf->setLeft16(x, clamp(outbuf->getLeft16(x) + inbuf->getLeft16(x) * pm.left));
        outbuf->setRight16(x, clamp(outbuf->getRight16(x) + inbuf->getLeft16(x) * pm.right));
	}
}

void getsafefilename(char *fname)
{
	char *where;
	int   x = 1;

	where = findperiod(fname);

	if (where == NULL)
	{
		printf("Cannot find filename extension, exiting\n");
		exit(0);
	}

	sprintf(where, "~pan1.wav");

	while (fileexists(fname))
		sprintf(where, "~pan%d.wav", ++x);
	
}

float calcpan(uint samplenum, uint samprate, float delay, float cycletime, int pattern, float cycles)
{
	float time = (float)samplenum / samprate;
	float frac;

	if (time < delay)
	{
		switch(pattern)
		{
			case 1:
				return (float) -1;
			case 2:
				return (float) 1;
			case 3:
			case 4:
				return (float) 0;
		}
	}

	if (time > (delay + cycletime * cycles))
	{
		switch(pattern)
		{
			case 1:
				return (float) -cos(PI * cycles);
			case 2:
				return (float) cos(PI * cycles);
			case 3:
				return (float) sin(PI * cycles);
			case 4:
				return (float) -sin(PI * cycles);
		}
	}

	frac = (time - delay) / cycletime;
	
	switch(pattern)
	{
		case 1:
			return (float) -cos(frac * PI);
		case 2:
			return (float) cos(frac * PI);
		case 3:
			return (float) sin(frac * PI);
		case 4:
			return (float) -sin(frac * PI);
	}
	return (float) 0;
}

short clamp(float x)
{
	x = (float) floor(x + 0.5);

	if (x > 32767)
	{
		clipped++;
		return 32767;
	}

	if (x < -32768)
	{
		clipped++;
		return -32768;
	}

	return (short)x;
}

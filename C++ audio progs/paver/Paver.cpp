//
// paver.cpp
//
// implement a wave-file paver
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream.h>

#include "GlobalProcs.h"
#include "Wavefile.h"


void main(int argc, char *argv[])
{
    int    channels = 2;
    uint   samprate = 0, samples;
	float  time = (float) 0.0;

	if (argc < 4)
	{
		printf("Usage: %s wavfile time samprate [channels]\n", argv[0]);
        exit(0);
	}
    
    if (findperiod(argv[1]) == NULL)
        die("That first argument does not look like a filename...");
        
	sscanf(argv[2], "%f", &time);
	sscanf(argv[3], "%d", &samprate);

	if (argc == 5)
		sscanf(argv[4], "%d", &channels);

	if (time < 0.001)
        die("Must set time parameter.");

	if (samprate == 0)
		die("Must set sample rate parameter.");

	samples = (uint) floor(time * samprate) + 1;

	Wavefile wf(samples, samprate, channels);
	wf.saveaswavefile(argv[1]);
}


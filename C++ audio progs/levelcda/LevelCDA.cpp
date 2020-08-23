//
// levelcda.cpp
//
// app to get waveform level (for cdaudio files)
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Buffer.h"
#include "CDaudio.h"


void main(int argc, char *argv[])
{
	float    level;
	min_max  mm;
	
	if (argc != 2)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

    if (nosuchfile(argv[1])) exit(0);
    
    CDaudio cda(argv[1]);
    cda.scanbuf16(&mm);
    level = dothemath16(&mm, (float) 0.0);

	printf("File level is %.3f\n", level);
}


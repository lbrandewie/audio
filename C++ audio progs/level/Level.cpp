//
// level.cpp
//
// app to get waveform level
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream.h>

#include "GlobalProcs.h"
#include "Buffer.h"
#include "Wavefile.h"


int main(int argc, char *argv[])
{
	float    level;
	min_max  mm;
	
	if (argc == 1)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

	Wavefile wf(argv[1]);
	wf.scanbuf16(&mm);
	level = dothemath16(&mm, (float)0.0);

	printf("File level is %.3f\n", level);
	return 0;
}




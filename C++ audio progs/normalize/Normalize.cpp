//
// normalize.cpp
//
// normalize app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Normalize.h"
#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"


void main(int argc, char *argv[])
{
	float  level = (float) 0.0;
	char   newfilename[1024];

	if (argc == 1)
	{
		printf("Usage: %s wavefile [level]\n", argv[0]);
		exit(0);
	}

	if (argc == 3)
		sscanf(argv[2], "%f", &level);

	if ((argc == 3) && (level > 0.00001))
	{
		printf("Levels above 0 not allowed (clipping).\n");
		exit(0);
	}

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

    Wavefile wf(argv[1]);
    
	wf.normalize16(level);
    wf.saveaswavefile(newfilename);
}

void getsafefilename(char *fname)
{
	char *where;
	int   x = 1;

	where = findperiod(fname);

	if (where == NULL)
	{
		printf("Cannot find filename extension, exiting.\n");
		exit(0);
	}

	sprintf(where, "~norm1.wav");

	while (fileexists(fname))
		sprintf(where, "~norm%d.wav", ++x);
	
}


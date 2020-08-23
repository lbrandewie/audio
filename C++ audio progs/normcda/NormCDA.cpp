//
// normcda.cpp
//
// normalize app (for cdaudio)
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "NormCDA.h"
#include "GlobalProcs.h"
#include "Buffer.h"
#include "CDaudio.h"


void main(int argc, char *argv[])
{
	float  level;
	char   newfilename[1024];

	if (argc == 1)
	{
		printf("Usage: %s wavefile [level]\n", argv[0]);
		exit(0);
	}

	if (argc == 3)
		sscanf(argv[2], "%f", &level);

	if ((argc == 3) && (level > 0.00001))
		die("Levels above 0 not allowed for 16-bit files!\n");

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

    CDaudio cda(argv[1]);
    
    cda.normalize16(level);
    cda.saveascdafile(newfilename);
    
	if (cda.getclipped())
		printf("Clipping detected. %d samples affected.\n", cda.getclipped());
}

void getsafefilename(char *fname)
{
	char  *where;
	int   x = 1;

	where = findperiod(fname);

	if (where == NULL)
		die("Cannot find filename extension, exiting.\n");

	sprintf(where, "~norm1.cda");

	while (fileexists(fname))
		sprintf(where, "~norm%d.cda", ++x);
}


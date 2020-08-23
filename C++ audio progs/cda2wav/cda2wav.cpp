//
// cda2wav.cpp
//
// cdaudio-to-wave app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "GlobalProcs.h"
#include "Buffer.h"
#include "CDaudio.h"


void main(int argc, char *argv[])
{
	char  newname[1024];
	char  *where;

	if (argc == 1)
	{
		printf("Usage: %s cdafile\n", argv[0]);
		exit(0);
	}

	if (nosuchfile(argv[1])) exit(0);
	
	strcpy(newname, argv[1]);
    where = findperiod(newname);
	sprintf(where, ".wav");
	
    CDaudio cda(argv[1]);
    cda.saveaswavefile(newname);
}

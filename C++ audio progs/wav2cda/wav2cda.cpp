//
// wav2cda.cpp
//
// wave-to-cdaudio app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream.h>

#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"


void main(int argc, char *argv[])
{
	char     newname[1024];
	char     *where;

	if (argc == 1)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

    Wavefile wf(argv[1]);
    
    if (wf.getbits() != 16)
        die("unsupported bit depth.");
    
    if (wf.getchannels() != 2)
        die("cannot convert a mono file.");
    
    if (wf.getsamprate() != 44100)
        die("unsuported sample rate.");
        
    strcpy(newname, argv[1]);
	where = findperiod(newname);
	sprintf(where, ".cda");
	
    wf.saveascdafile(newname);
}


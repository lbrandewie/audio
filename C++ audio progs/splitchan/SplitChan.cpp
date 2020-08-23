//
// splitchan.cpp
//
// channel splitter app
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
	uint  x, samps, samprate;

	char  newname1[1024];
	char  newname2[1024];
	char  *where;

	if (argc == 1)
	{
		cout << "Usage: " << argv[0] << "wavefile" << endl;
		exit(0);
	}

    if (nosuchfile(argv[1]))
    {
        cout << "Cannot open file " << argv[1] << endl;
        exit(1);
    }
 
    Wavefile wf(argv[1]);

    samps = wf.getsamples();
    samprate = wf.getsamprate();
    
	if (wf.getchannels() != 2)
	{
		cout << "Not a stereo file: " << argv[1] << endl;
		exit(0);
	}

	strcpy(newname1, argv[1]);
	strcpy(newname2, argv[1]);

	where = findperiod(newname1);
	sprintf(where, "~left.wav");
	
	where = findperiod(newname2);
	sprintf(where, "~right.wav");

    Buffer bufL(samps, samprate, 1, 16);
    Buffer bufR(samps, samprate, 1, 16);
    
    for (x = 0; x < samps; x++)
    {
        bufL.setLeft16(x, wf.getLeft16(x));
        bufR.setLeft16(x, wf.getRight16(x));
    }
    
    bufL.saveaswavefile(newname1);
    bufR.saveaswavefile(newname2);
}


//
// combinech.cpp
//
// channel combiner app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream.h>

#include "Combinech.h"
#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"


int main(int argc, char *argv[])
{
	uint   x, max_samp;

	char   newfilename[1024];
	char   *where;

	if (argc < 3)
	{
		printf("Usage: %s wavefile1 wavefile2\n", argv[0]);
		exit(0);
	}

	Wavefile wf1(argv[1]);
    Wavefile wf2(argv[2]);
    
    if ((wf1.getchannels() == 2) || (wf2.getchannels() == 2))
    {
        cout << "Must use mono source files." << endl;
        exit(0);
    }
       
    strcpy(newfilename, argv[1]);
    where = findperiod(newfilename);
	sprintf(where, "~comb.wav");
	
	if (fileexists2(newfilename)) exit(0);
	
    max_samp = getmaxd(wf1.getsamples(), wf2.getsamples());

    Buffer buf(max_samp, wf1.getsamprate(), 2, 16);

    for (x = 0; x < max_samp; x++)
    {
        buf.setLeft16(x, wf1.getLeft16(x));
        buf.setRight16(x, wf2.getLeft16(x));
    }
    
    buf.saveaswavefile(newfilename);

    return 0;
}

uint getmaxd(uint a, uint b)
{
	return (a > b) ? a : b;
}

//
// StereoFX.cpp
//
// stereo effects app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "StereoFX.h"
#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"


void main(int argc, char *argv[])
{
	float level, dbmult, normlevel = (float)0.0;

	char   newfilename[1024];
	uint   x, samps, srate;

	if (argc < 3)
	{
		printf("Usage: %s wavefile level [normalize level]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[2], "%f", &level);

	if (argc == 4)
		sscanf(argv[3], "%f", &normlevel);

	dbmult = (float) pow(2, level / 6);

    Wavefile wf(argv[1]);

	if (wf.getchannels() != 2)
	{
		printf("%s is not a stereo file.\n", argv[1]);
		exit(0);
	}

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

    samps = wf.getsamples();
    srate = wf.getsamprate();
    
    Buffer bufMid(samps, srate, 1, 32);
    Buffer bufSides(samps, srate, 1, 32);
    
	wf.cv_midsides(&bufMid, &bufSides);
	
	for (x = 0; x < samps; x++)                                 // amplify the sides info
		bufSides.setLeft32(x, bufSides.getLeft32(x) * dbmult);

    Buffer bufNew(samps, srate, 2, 32);
    
    cv_stereo(&bufNew, &bufMid, &bufSides);
        
	if ((argc == 4) && (normlevel < 0.000001))
		bufNew.normalize32(normlevel);
	
	bufNew.saveaswavefile32(newfilename);

	if (bufNew.getclipped())
		printf("Clipping detected, %d samples affected.\n", bufNew.getclipped());
}

void cv_stereo(Buffer *bufOut, Buffer *bufMid, Buffer *bufSides)
{
	uint   x, lim = bufOut->getsamples();

    for (x = 0; x < lim; x++)
	{
		bufOut->setRight32(x, bufMid->getLeft32(x) + bufSides->getLeft32(x));
        bufOut->setLeft32(x, bufMid->getLeft32(x) - bufSides->getLeft32(x));
	}
}

void getsafefilename(char *fname)
{
	char  *where;
	int   x = 1;

	where = findperiod(fname);

	if (where == NULL)
	{
		printf("Cannot find filename extension, exiting\n");
		exit(0);
	}

	sprintf(where, "~sfx1.wav");

	while (fileexists(fname))
		sprintf(where, "~sfx%d.wav", ++x);
}


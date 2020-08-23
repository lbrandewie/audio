//
// CDaudio.cpp
//
// code for CDaudio file class
//

#include "CDaudio.h"
#include "GlobalProcs.h"

#include <stdio.h>
#include <string.h>


CDaudio::CDaudio()
{
    setdata(0);
}

CDaudio::CDaudio(char *fname)
{
    uint len;
    FILE *infile;

    if (fileexists(fname))
    {
        len = getfilelength(fname);
        setsamples(len / 4);
        setchannels(2);
		setbits(16);
        infile = fopen(fname, "rb");
        setdata(new short[getsamples() * 2]);
        fread(getdata(), 4, getsamples(), infile);
        fclose(infile);
		settime((float)(getsamples() - 1) / 44100);
        setsamprate(44100);
    }
    else
    {
        setdata(0);
        error = 1;
    }
}

CDaudio::~CDaudio()
{
    // nothing to do (Buffer destructor gets it)
}


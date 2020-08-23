//
// wavefile.cpp
//
// code for wave file class
//

#include "Buffer.h"
#include "Wavefile.h"
#include "GlobalProcs.h"

#include <stdio.h>
#include <string.h>
#include <iostream.h>


Wavefile::Wavefile()
{
    setdata(0);
}

Wavefile::Wavefile(char *fname, short bits)
{
    FILE     *infile;
    wav_info wi;
    sample   samp;
    uint     x;
    
    if (bits != 32)
        die("used 32-bit wavefile constructor with wrong bits.");
    
    if (fileexists(fname))
    {
        getwaveinfo(fname, &wi);
        setbits(32);
        setchannels(wi.channels);
        setsamples(wi.samples);
        setsamprate(wi.samprate);
        settime((float)wi.time);
        setformat(wi.format);
        
        if ((wi.bits != 16) || (wi.format != 1))
        {
            error = 1;
            return;
        }
        
        if (wi.channels == 2)
            setdata(new float[wi.samples * 2]);
        else
            setdata(new float[wi.samples]);
    
        infile = fopen(fname, "rb");
        
        for (x = 0; x < wi.samples; x++)
        {
            fread(&samp, 2, wi.channels, infile);
            setLeft32(x, (float) samp.left);
            if (wi.channels == 2)
                setRight32(x, (float) samp.right);
        }
    }
}

Wavefile::Wavefile(char *fname)
{
    uint     bytes;
    FILE     *infile;
    wav_info wi;
    
    if (fileexists(fname))
    {
		getwaveinfo(fname, &wi);

		setbits(wi.bits);
        setchannels(wi.channels);
        setsamples(wi.samples);
        setsamprate(wi.samprate);
        settime((float)wi.time);
        setformat(wi.format);
        
        if ((wi.bits != 16) || (wi.format != 1))
        {
            error = 1;
            return;
        }
        
        setdata(new short[wi.samples * wi.channels]);
            
        infile = fopen(fname, "rb");
        fseek(infile, wi.dataoffset, SEEK_SET);
        bytes = fread(getdata(), 1, wi.datalength, infile);
        if (bytes != wi.datalength)
            error = 2;
        fclose(infile);
    }
    else
    {
        setdata(0);
        error = 3;
    }
}

Wavefile::~Wavefile()
{
    //delete [] getdata();
}

void Wavefile::setformat(short f)
{
	format = f;
}

Wavefile::Wavefile(uint samps, uint sr, short ch)
{
	setsamples(samps);
	setsamprate(sr);
	setchannels(ch);
	setbits(16);
	
	setdata(new short[samps * ch]);
    
    clearbuf();
}

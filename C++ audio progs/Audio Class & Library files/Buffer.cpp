//
// Buffer.cpp
//
// code for Buffer base class
//

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "Buffer.h"
#include "GlobalProcs.h"


Buffer::Buffer()
{
    samples = samprate = clipped = 0;
    channels = 0;
	bits = 0;
    data = 0;
    time = (float) 0.0;
}

Buffer::~Buffer()
{
    delete [] data;
}

Buffer::Buffer(uint samps, uint srate, short ch, short bts)
{
	samples = samps;
	samprate = srate;
	channels = ch;
	bits = bts;
    clipped = 0;
    
	time = (float) (samples - 1) / samprate;

	if (bits == 32)
		if (channels == 1)
			data = new float[samples];
		else
			data = new float[samples * 2];
	else if (bits == 16)
		if (channels == 1)
			data = new short[samples];
		else
			data = new short[samples * 2];
    
    clearbuf();
}

short Buffer::clamp(float x)
{
	x = (float) floor(x + 0.5);

	if (x > 32767)
	{
		clipped++;
		return 32767;
	}

	if (x < -32768)
	{
		clipped++;
		return -32768;
	}

	return (short)x;
}

void Buffer::clearbuf()
{
    uint x, lim = samples * channels;
    float *ptr;
    
    if (bits == 32)
    {
        ptr = (float *) data;
        for (x = 0; x < lim; x++)
            ptr[x] = (float) -0.5;
    }
    else
        memset(data, 0, 2 * samples * channels);
}

void Buffer::cv_midsides(Buffer *bufMid, Buffer *bufSides)
{
	uint   x;
    float  mid, sides;

    if (bits !=16)
        die("used cv_midsides on 32-bit buffer.");
    
    if ((bufMid->bits !=32) || (bufSides->bits != 32))
        die("cv_midsides requires 32-bit output buffers.");
    
    for(x = 0; x < samples; x++)
	{
        mid = (float) (getLeft16(x) + getRight16(x)) / 2;
        sides = (float) getRight16(x) - mid;
        bufMid->setLeft32(x, mid);
        bufSides->setLeft32(x, sides);
	}
}


short Buffer::getbits()
{
	return bits;
}

short Buffer::getchannels()
{
    return channels;
}

uint Buffer::getclipped()
{
    return clipped;
}

void *Buffer::getdata()
{
    return data;
}

short Buffer::getLeft16(uint sampnum)
{
    short *ptr = (short *) data;

    if (sampnum >= samples)
        return 0;
    
    if (channels == 1)
        return ptr[sampnum];
    else
        return ptr[2 * sampnum];
}

float Buffer::getLeft32(uint sampnum)
{
    float *ptr = (float *) data;
    
    if (sampnum >= samples)
        return (float)-0.5;
    
    if (channels == 1)
        return ptr[sampnum];
    else
        return ptr[2 * sampnum];
}

short Buffer::getRight16(uint sampnum)
{
    short *ptr = (short *) data; 
    
    if (sampnum >= samples)
        return 0;
        
    if (channels == 1)
    {
		die("Tried to read right sample in mono buffer!\n");
		return 0;
	}
    else
        return ptr[2 * sampnum + 1];
 }
 
 float Buffer::getRight32(uint sampnum)
{
    float *ptr = (float *) data;
    
    if (sampnum >= samples)
        return (float) -0.5;
        
    if (channels == 1)
    {
		die("Tried to read right sample in mono buffer!\n");
		return (float) 0.0;
	}
    else
        return ptr[2 * sampnum + 1];
}

uint Buffer::getsamples()
{
    return samples;
}

uint Buffer::getsamprate()
{
    return samprate;
}

float Buffer::gettime()
{
    return time;
}

void Buffer::mix32(Buffer *inbuf, float offset, float level, float pan)		    // destructively mixes inbuf into 32-bit buf
{
	panmult pm;
	uint    x, samp_off;
	float   dbmult, minus3, avg;

    if (bits == 16)
        die("Attempt to mix into 16-bit buffer using mix32 (not a good idea).");
    
	samp_off = (uint)floor(offset * samprate + 0.5);
	dbmult = (float)pow(2.0, level / 6.0);
    minus3 = (float)pow(2.0, -0.5);
    
	dopancalcs(pan, &pm);

	for (x = 0; x < inbuf->samples; x++)
	{
		if ((inbuf->channels == 2) && (channels == 2))			// stereo to stereo
		{
            setLeft32(x + samp_off, getLeft32(x + samp_off) + inbuf->getLeft16(x) * dbmult);
            setRight32(x + samp_off, getRight32(x + samp_off) + inbuf->getRight16(x) * dbmult);
		}
		else if ((inbuf->channels == 1) && (channels == 1))		// mono into mono
		{
			setLeft32(x + samp_off, getLeft32(x + samp_off) + inbuf->getLeft16(x) * dbmult);
		}
		else if ((inbuf->channels == 1) && (channels == 2))		// mono into stereo
		{
            setLeft32(x + samp_off, getLeft32(x + samp_off) + inbuf->getLeft16(x) * dbmult * pm.left);
            setRight32(x + samp_off, getRight32(x + samp_off) + inbuf->getLeft16(x) * dbmult * pm.right);
		}
		else								// stereo into mono
		{
			avg = (float)(inbuf->getLeft16(x) + inbuf->getRight16(x)) / 2;
            setLeft32(x + samp_off, getLeft32(x + samp_off) + avg * dbmult * minus3);
		}
	}
}

void Buffer::normalize16(float normlevel)
{
	uint     x;
	min_max  mm;
    float    dbmult, ratio, ratio_top, ratio_bot, maxtop, minbot;

	if (bits != 16)
		die("tried to call normalize16 on 32-bit buffer.");

	scanbuf16(&mm);

	dbmult = (float)pow(2, normlevel / 6);
	
	maxtop = getmax(mm.leftmax, mm.rightmax);
	minbot = getmin(mm.leftmin, mm.rightmin);

	ratio_top = (float)maxtop / (32767 * dbmult);
	ratio_bot = (float)fabs(minbot) / (32768 * dbmult);

	ratio = getmaxf(ratio_top, ratio_bot);

	dbmult = 1 / ratio;

	for (x = 0; x < samples; x++)
	{
		setLeft16(x, clamp(getLeft16(x) * dbmult));
        if (channels == 2)
            setRight16(x, clamp(getRight16(x) * dbmult));
	}
}

void Buffer::normalize32(float normlevel)
{
	uint     x;
	min_maxf mm;
    float    dbmult, ratio, ratio_top, ratio_bot, maxtop, minbot;

	if (bits != 32)
		die("Tried to call normalize32 on 16-bit buffer.");

	scanbuf32(&mm);

	dbmult = (float)pow(2, normlevel / 6);
	
	maxtop = getmaxf(mm.leftmax, mm.rightmax);
	minbot = getminf(mm.leftmin, mm.rightmin);

	ratio_top = (float)maxtop / (32767 * dbmult);
	ratio_bot = (float)fabs(minbot) / (32768 * dbmult);

	ratio = getmaxf(ratio_top, ratio_bot);

	dbmult = 1 / ratio;

	for (x = 0; x < samples; x++)
	{
		setLeft32(x, getLeft32(x) * dbmult);
        if (channels == 2)
            setRight32(x, getRight32(x) * dbmult);
	}
}

void Buffer::saveascdafile(char *fname)
{
    FILE *outfile;
    
    if (channels == 1) return;
    if (bits != 16) return;
    if (samprate != 44100) return;
    
    if (fileexists2(fname)) return;
    
    outfile = fopen(fname, "wb");
    fwrite(data, 4, samples, outfile);
    fclose(outfile);
}
    
void Buffer::saveaswavefile(char *fname)
{
    wav_hdr wh;
    FILE    *outfile;

    createheader(samples, samprate, channels, &wh);
    
    if (fileexists2(fname)) return;
    
    outfile = fopen(fname, "wb");
    
	fwrite(&wh, 1, 44, outfile);
	fwrite(data, 2, samples * channels, outfile);
    
	fclose(outfile);
}

void Buffer::saveaswavefile32(char *fname)
{
    wav_hdr wh;
    FILE    *outfile;
    uint    x;
    short   samp;
    
    if (bits != 32)
        die("used saveaswavefile32 on 16-bit buffer.");
        
    createheader(samples, samprate, channels, &wh);
    
    if (fileexists2(fname)) return;
    
    outfile = fopen(fname, "wb");
    fwrite(&wh, 1, 44, outfile);
    
    for (x = 0; x < samples; x++)
    {
        samp = clamp(getLeft32(x));
        fwrite(&samp, 1, 2, outfile);
        
        if (channels == 2)
        {
            samp = clamp(getRight32(x));
            fwrite(&samp, 1, 2, outfile);
        }
    }
	fclose(outfile);
}

void Buffer::scanbuf16(min_max *mm)
{
    uint x;
    
    mm->leftmin = 32767;
    mm->leftmax = -32768;
    mm->rightmin = 32767;
    mm->rightmax = -32768;
    
    for (x = 0; x < samples; x++)
    {
        mm->leftmin = getmin(mm->leftmin, getLeft16(x));
        mm->leftmax = getmax(mm->leftmax, getLeft16(x));
        
        if (channels == 2)
        {
            mm->rightmin = getmin(mm->rightmin, getRight16(x));
            mm->rightmax = getmax(mm->rightmax, getRight16(x));
        }
    }
}
   
void Buffer::scanbuf32(min_maxf *mm)
{
    uint x;
    
    mm->leftmin = (float)1e10;
    mm->leftmax = (float)-1e10;
    mm->rightmin = (float)1e10;
    mm->rightmax = (float)-1e10;
    
    for (x = 0; x < samples; x++)
    {
        mm->leftmin = getminf(mm->leftmin, getLeft32(x));
        mm->leftmax = getmaxf(mm->leftmax, getLeft32(x));
        
        if (channels == 2)
        {
            mm->rightmin = getminf(mm->rightmin, getRight32(x));
            mm->rightmax = getmaxf(mm->rightmax, getRight32(x));
        }
    }
}

void Buffer::setbits(short b)
{
	if ((b == 16) || (b == 32))
		bits = b;
}

void Buffer::setchannels(short ch)
{
	if ((ch > 0) && (ch < 3))
		channels = ch;
}

void Buffer::setclipped(uint c)
{
    clipped = c;
}

void Buffer::setdata(void *ptr)
{
	if (ptr != 0)
		data = ptr;
}

void Buffer::setLeft16(uint sampnum, short val)
{
    short *ptr = (short *) data;
    
    if (sampnum >= samples)
        return;
    
    if (channels == 1)
        ptr[sampnum] = val;
    else
        ptr[2 * sampnum] = val;
}

void Buffer::setLeft32(uint sampnum, float val)
{
    float *ptr = (float *) data;
    
    if (sampnum >= samples)
        return;
    
    if (channels == 1)
        ptr[sampnum] = val;
    else
        ptr[2 * sampnum] = val;
}

void Buffer::setRight16(uint sampnum, short val)
 {
    short *ptr = (short *) data;
    
    if (sampnum >= samples)
        return;
        
    if (channels == 1)
        die("Tried to set right channel value of mono file.");
    else
        ptr[2 * sampnum + 1] = val;
}

void Buffer::setRight32(uint sampnum, float val)
{
    float *ptr = (float *) data;
    
    if (sampnum >= samples)
        return;
        
    if (channels == 1)
        die("Tried to set right channel value in mono file.");
    else
        ptr[2 * sampnum + 1] = val;
}
        
void Buffer::setsamples(uint samp)
{
	if (samp > 0)
		samples = samp;
}

void Buffer::setsamprate(uint sr)
{
	if ((sr >= 4000) && (sr <= 96000))
		samprate = sr;
}

void Buffer::settime(float t)
{
	if (t >= 0) 
		time = t;
}


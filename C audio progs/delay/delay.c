//
// delay.c
//
// implement a digital delay
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "delay.h"

uint   clipped;
struct wav_info  info;

void main(int argc, char *argv[])
{
	float  normlevel = (float)0.0, time;
	float  delay, level;
	uint   samples, x, lim;	
	uchar  newfilename[1024];
	float  *buf;
	struct wav_hdr hdr;
	short  sample;

	FILE   *outfile;

	if (argc < 4)
	{
		printf("Usage: %s wavfile [delay in ms] [level] [[normlevel]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[2], "%f", &delay);
	sscanf(argv[3], "%f", &level);

	if (argc == 5)
		sscanf(argv[4], "%f", &normlevel);
	
	if (delay < 0.00001)
	{
		printf("Must set delay value\n");
		exit(0);
	}

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

	getwaveinfo(argv[1], &info);
	time = (float)info.time + delay / 1000;
	samples = (uint)floor(time * info.samprate) + 1;

	buf = zc(malloc(samples * info.channels * 4));

	gensilence(buf, samples, info.channels);
	mixtobuf(buf, argv[1], 0.0, 0.0);
	mixtobuf(buf, argv[1], delay / 1000, level);
	normalize(buf, normlevel);

	createheader(samples, info.samprate, info.channels, &hdr);

	outfile = fopen(newfilename, "wb");
	fwrite(&hdr, 1, 44, outfile);

	lim = samples * info.channels;

	for (x = 0; x < lim; x++)
	{
		sample = clamp(buf[x]);
		fwrite(&sample, 1, 2, outfile);

		if (info.channels == 2)
		{
			sample = clamp(buf[++x]);
			fwrite(&sample, 1, 2, outfile);
		}
	}

	fclose(outfile);
	free(buf);

	if (clipped)
		printf("Clipping detected, %d samples affected.\n", clipped);
}


uchar *findperiod(uchar *buf)
{
	int x, len;

	len = strlen(buf);

	for (x = len -1; x >= 0; x--)
		if (buf[x] == '.')
			return(&buf[x]);
	
	return NULL;
}

uchar *finddata(uchar *buf)
{
	int x;

	for (x = 0; x < 1024; x++)
		if (memcmp(&buf[x], "data", 4) == 0)
			return(&buf[x]);

	return(NULL);
}

short clamp(double x)
{
	x = floor(x + 0.5);

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

	return((short)x);
}

int nosuchfile(char *fname)		// returns 1 if a file fails to exist
{
	FILE *thefile;

	thefile = fopen(fname, "r");

	if (thefile == NULL)
		return 1;
	else
	{
		fclose(thefile);
		return 0;
	}
}

int fileexists(char *fname)		// returns 1 if a file exists
{
	FILE *thefile;

	thefile = fopen(fname, "r");

	if (thefile == NULL)
		return 0;

	fclose(thefile);
	return 1;
}

void gensilence(float *buf, uint samples, int channels)
{
	uint   x, lim;

	lim = channels * samples;

	for (x = 0; x < lim; x++)
		buf[x] = (float)-0.5;
}

void createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr)
{
	uint datasize = samples * 2 * channels;
	
	memcpy(&hdr->riff, "RIFF", 4);
	memcpy(&hdr->wave, "WAVEfmt ", 8);
	memcpy(&hdr->data, "data", 4);

	hdr->length1 = datasize + 36;
	hdr->length2 = 16;
	hdr->format = 1;
	hdr->channels = channels;
	hdr->samprate = samprate;
	hdr->bytespersec = samprate * 2 * channels;
	hdr->bytealign = 2 * channels;
	hdr->bits = 16;
	hdr->length3 = datasize;
}


void getsafefilename(uchar *fname)
{
	uchar *where;
	int   x = 1;

	where = findperiod(fname);

	if (where == NULL)
	{
		printf("Cannot find filename extension, exiting\n");
		exit(0);
	}

	sprintf(where, "~delay1.wav");

	while (fileexists(fname))
		sprintf(where, "~delay%d.wav", ++x);
	
}

uint getdatalength(uchar *buf)
{
	uchar *where;

	where = finddata(buf);

	return *((uint *)where + 1);
}

float getmaxf(float a, float b)
{
	return (a > b) ? a : b;
}

float getminf(float a, float b)
{
	return (a < b) ? a : b;
}

void mixtobuf(float *buf, uchar *infilename, double offset, double level)		// destructively mixes infile into outfile
{
	struct wav_info info_in;
	
	uint   x, y, samp_off, lim;

	short  *indata;
	short  *samp_in;
    
	double dbmult;

	FILE   *infile;

	getwaveinfo(infilename, &info_in);
	infile = fopen(infilename, "rb");

	fseek(infile, info_in.dataoffset, SEEK_SET);
	indata = malloc(info_in.datalength);
	fread(indata, 1, info_in.datalength, infile);

	samp_off = (uint)(offset * info_in.samprate);
	
	samp_in = indata;							// set up data pointers

	dbmult = pow(2.0, level / 6.0);

	lim = info_in.samples * info_in.channels;

	for (x = 0, y = 0; x < lim; x++, y++)
	{
		if (info_in.channels == 2)			// stereo to stereo
		{
			buf[x + samp_off] += samp_in[y] * (float)dbmult;
			buf[++x + samp_off] += samp_in[++y] * (float)dbmult;
		}
		else if (info_in.channels == 1)		// mono into mono
		{
			buf[x + samp_off] += samp_in[y] * (float)dbmult;
		}
	}

	free(indata);
}

void getwaveinfo(uchar *fname, struct wav_info *info)		// get info and check for errors
{
	FILE   *thefile;
	struct wav_hdr hdr;
	uchar  *data;
	int    diff;
	
	thefile = fopen(fname, "rb");
	
	if (thefile == NULL)			// signal error
	{
		printf("Cannot open file %s!\n", fname);
		exit(0);
	}

	fread(&hdr, 1, 1024, thefile);

	info->bits = hdr.bits;
	info->channels = hdr.channels;
	info->format = hdr.format;
	info->samprate = hdr.samprate;
	
	data = finddata((uchar *)&hdr);

	if (data == NULL)
	{
		printf("Can't find data header: %s\n", fname);
		exit(0);
	}
	
	diff = data - (uchar *)&hdr;

	info->datalength = getdatalength((uchar *)&hdr);
	info->dataoffset = diff + 8;

	info->samples = info->datalength / hdr.bytealign;
	info->time = (double)(info->samples - 1) / hdr.samprate;

	fclose(thefile);

	if (info->bits == 8)
	{
		printf("Unsupported bit resolution: %s\n", fname);
		exit(0);
	}

	if (info->format != 1)
	{
		printf("Not a PCM file: %s\n", fname);
		exit(0);
	}
}

void normalize(float *sf, float normlevel)
{
	uint x, lim;
	struct min_max2 mm;
    float  dbmult, ratio, ratio_top, ratio_bot, maxtop, minbot;

	mm.leftmin = (float)99999;
	mm.leftmax = (float)-99999;
	mm.rightmin = (float)99999;
	mm.rightmax = (float)-99999;

	lim = info.samples * info.channels;

	for (x = 0; x < lim; x++)
	{
		mm.leftmin = getminf(mm.leftmin, sf[x]);
		mm.leftmax = getmaxf(mm.leftmax, sf[x]);
		
		if (info.channels == 2)
		{
			mm.rightmin = getminf(mm.rightmin, sf[++x]);
			mm.rightmax = getmaxf(mm.rightmax, sf[x]);
		}
	}

	dbmult = (float)pow(2, normlevel / 6);
	
	maxtop = getmaxf(mm.leftmax, mm.rightmax);
	minbot = getminf(mm.leftmin, mm.rightmin);

	ratio_top = (float)maxtop / (32767 * dbmult);
	ratio_bot = (float)fabs(minbot) / (32768 * dbmult);

	ratio = getmaxf(ratio_top, ratio_bot);

	dbmult = 1 / ratio;

	for (x = 0; x < lim; x++)
	{
		sf[x] *= dbmult;
	}
}

void *zc(void *ptr)             // pointer sanity check for malloc
{
    if (ptr == NULL)
    {
		printf("Can't allocate memory!\n");
        exit(1);
		return NULL;
    }
    
    return ptr;
}

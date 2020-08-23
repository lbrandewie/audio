//
// panwipe.c
//
// pan wiper app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "panwipe.h"


double PI;


void main(int argc, char *argv[])
{
	double delay, ct, cycles = 1.0;
	uchar  newfilename[1024];
    struct wav_info info;
    int    x, len, pattern = 0;

	PI = 4 * atan(1);

	if (argc < 5)
	{
		printf("Usage: %s wavefile1 delay 'cycle time' pattern [cycles]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[2], "%lf", &delay);
	sscanf(argv[3], "%lf", &ct);

	if (argc == 6)
		sscanf(argv[5], "%lf", &cycles);

	if (delay < 0)
	{
		printf("Brain hurts! Negative delays not allowed!\n");
		exit(0);
	}
	
	len = strlen(argv[4]);

	for (x = 0; x < len; x++)
		argv[4][x] = tolower(argv[4][x]);

	if (strcmp(argv[4], "lr") == 0)
		pattern = 1;
	else if (strcmp(argv[4], "rl") == 0)
	    pattern = 2;
	else if (strcmp(argv[4], "cr") == 0)
		pattern = 3;
	else if (strcmp(argv[4], "cl") == 0)
		pattern = 4;

	if (pattern == 0)
	{
		printf("I don't understand pattern %s, quitting.\n", argv[4]);
		exit(0);
	}

    getwaveinfo(argv[1], &info);
    
    if (info.channels != 1)
    {
        printf("Source file must be mono.\n");
		exit(0);
	}

    strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

	gensilence(newfilename, info.time, info.samprate, 2);		// destination file must be stereo
	mixdestruct(newfilename, argv[1], 0, 0, delay, ct, pattern, cycles);
}

void mixdestruct(uchar *outfilename, uchar *infilename, double offset, double level, 
				 double delay, double ct, int pattern, double cycles)		// destructively mixes infile into outfile
{
	struct wav_info info_in, info_out;
	struct panmult  pm;

	uint   x, samp_off;

	short  *indata, *outdata;
	short  *samp_in, *samp_out;
	
	short  samp_l, samp_r, samp_m;
    
	double dbmult, pan;

	FILE   *infile, *outfile;

	getwaveinfo(infilename, &info_in);
	getwaveinfo(outfilename, &info_out);

	outfile = fopen(outfilename, "r+b");
	infile = fopen(infilename, "rb");

	fseek(outfile, 44, SEEK_SET);
	fseek(infile, info_in.dataoffset, SEEK_SET);

	indata = zc(malloc(info_in.datalength));
	outdata = zc(malloc(info_out.datalength));

	fread(indata, 1, info_in.datalength, infile);
	fread(outdata, 1, info_out.datalength, outfile);

	samp_off = (uint)(offset * info_out.samprate);
	
	samp_in = indata;
	samp_out = outdata + (samp_off * info_out.channels);

	dbmult = pow(2.0, level / 6.0);

	pan = calcpan(0, info_out.samprate, delay, ct, pattern, cycles);
	dopancalcs(pan, &pm);

	for (x = 0; x < info_in.samples; x++)
	{
		if ((x % 100) == 0)
		{
			pan = calcpan(x, info_out.samprate, delay, ct, pattern, cycles);
			dopancalcs(pan, &pm);
		}

		if ((info_in.channels == 2) && (info_out.channels == 2))			// stereo to stereo
		{
			samp_l = *(samp_in++);
			samp_r = *(samp_in++);

			*samp_out = clamp(*samp_out + dbmult * samp_l);
			samp_out++;
			*samp_out = clamp(*samp_out + dbmult * samp_r);
			samp_out++;
		}
		else if ((info_in.channels == 1) && (info_out.channels == 1))		// mono into mono
		{
			samp_l = *(samp_in++);
			*samp_out = clamp(*samp_out + dbmult * samp_l);
			samp_out++;
		}
		else if ((info_in.channels == 1) && (info_out.channels == 2))		// mono into stereo
		{
			samp_l = *(samp_in++);
			*samp_out = clamp(*samp_out + dbmult * samp_l * pm.left);
			samp_out++;
			*samp_out = clamp(*samp_out + dbmult * samp_l * pm.right);
			samp_out++;
		}
		else																// stereo into mono
		{
			samp_l = *(samp_in++);
			samp_r = *(samp_in++);
			samp_m = (samp_l + samp_r) / 2;
			*samp_out = clamp(*samp_out + dbmult * samp_m);
			samp_out++;
		}
	}

	fseek(outfile, 44, SEEK_SET);
	fwrite(outdata, 1, info_out.datalength, outfile);

	fclose(infile);
	fclose(outfile);

	free(indata);
	free(outdata);
}

uchar *findperiod(uchar *buf)		// return a pointer to the last period in a filename
{
	int x;

	for (x = strlen(buf) - 1; x >= 0; x--)
		if (buf[x] == '.')
			return(&buf[x]);
	
	return NULL;
}

int nosuchfile(uchar *fname)		// returns 1 if a file fails to exist
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

int fileexists(uchar *fname)		// returns 1 if a file exists
{
	FILE *thefile;

	thefile = fopen(fname, "r");

	if (thefile == NULL)
		return 0;

	fclose(thefile);
    return 1;
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

	sprintf(where, "~pan1.wav");

	while (fileexists(fname))
		sprintf(where, "~pan%d.wav", ++x);
	
}

double getmax(double a, double b)
{
	return (a > b) ? a : b;
}

void gensilence(uchar *fname, double time, uint samprate, int channels)
{
	struct wav_hdr hdr;
	uint   x, samples, sample = 0;

	FILE   *outfile;

	samples = (uint)floor(time * samprate) + 1;
	createheader(samples, samprate, channels, &hdr);

	outfile = fopen(fname, "wb");

	if (outfile == NULL)
	{
		printf("Cannot open output file %s!\n", fname);
		exit(0);
	}

	fwrite(&hdr, 1, 44, outfile);		// write header

	for (x = 0; x < samples; x++)							// take advantage of the fact that a uint set to 0
		fwrite(&sample, 1, 2 * channels, outfile);			// has nothing but zero bits
    
    fclose(outfile);
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

uchar *finddata(uchar *buf)
{
	int x;

	for (x = 0; x < 1024; x++)
		if (memcmp(&buf[x], "data", 4) == 0)
			return &buf[x];

	return NULL;
}

uint getdatalength(uchar *buf)
{
	uchar *where;

	where = finddata(buf);

	return *((uint *)where + 1);
}

short clamp(double x)
{
	x = floor(x + 0.5);

	if (x > 32767) return 32767;
	if (x < -32768) return -32768;

	return((short)x);
}

void dopancalcs(double pan, struct panmult *mults)
{
	double level_r, dbmult_r, power_r;
	double level_l, dbmult_l, power_l;

	if (pan > 0)
	{
		level_r = (1 - pan) * -3;
		dbmult_r = pow(2, level_r / 6);
		power_r = dbmult_r * dbmult_r;

		power_l = 1 - power_r;
		dbmult_l = sqrt(power_l);
	}
	else
	{
		level_l = (pan + 1) * -3;
		dbmult_l = pow(2, level_l / 6);
		power_l = dbmult_l * dbmult_l;

		power_r = 1 - power_l;
		dbmult_r = sqrt(power_r);
	}
	mults->left = dbmult_l;
	mults->right = dbmult_r;
}

void getwaveinfo(uchar *fname, struct wav_info *info)
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
	fclose(thefile);

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

	if (info->bits == 8)
	{
		printf("Unsupported bit resolution: %s\n", fname);
		exit(0);
	}
	;
	if (info->format != 1)
	{
		printf("Not a PCM file: %s\n", fname);
		exit(0);
	}
}

double calcpan(uint samplenum, uint samprate, double delay, double ct, int pattern, double cycles)
{
	double time = (double)samplenum / samprate;
	double frac;

	if (time < delay)
	{
		switch(pattern)
		{
			case 1:
				return -1;
			case 2:
				return 1;
			case 3:
			case 4:
				return 0;
		}
	}

	if (time > (delay + ct * cycles))
	{
		switch(pattern)
		{
			case 1:
				return -cos(PI * cycles);
			case 2:
				return cos(PI * cycles);
			case 3:
				return sin(PI * cycles);
			case 4:
				return -sin(PI * cycles);
		}
	}

	frac = (time - delay) / ct;
	
	switch(pattern)
	{
		case 1:
			return -cos(frac * PI);
		case 2:
			return cos(frac * PI);
		case 3:
			return sin(frac * PI);
		case 4:
			return -sin(frac * PI);
	}
}

void *zc(void *ptr)             // pointer sanity check for malloc
{
    if (ptr == NULL)
    {
		printf("Can't allocate memory!\n");
        exit(1);
    }
    return ptr;
}

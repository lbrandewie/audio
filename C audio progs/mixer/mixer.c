//
// mixer.c
//
// mixer app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mixer.h"


struct wav_info info1;
uint   clipped;


void main(int argc, char *argv[])
{
	float offset, level, normlevel = (float)1.0, pan = (float)0.0;      // positive normlevel prevents normalization
	
	if (argc < 5)
	{
		printf("Usage: %s wavefile1 wavefile2 offset level [normlevel] [pan]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[3], "%f", &offset);
	sscanf(argv[4], "%f", &level);

	if (argc > 5)
		if (strcmp(argv[5], "-") != 0)
			sscanf(argv[5], "%f", &normlevel);

	if (argc == 7)
		sscanf(argv[6], "%f", &pan);
	
	if (offset < 0)
	{
		printf("Brain hurts! Negative offsets not allowed!\n");
		exit(0);
	}
		
	mixwaves(argv[1], argv[2], offset, level, normlevel, pan);
}

void mixwaves(uchar *fname1, uchar *fname2, float offset, float level, float normlevel, float pan)
{
	struct wav_info info2;	
	struct wav_hdr  hdr;

	uchar  newfilename[1024];
	double time2, maxtime;
	float  *buf;
	uint   samples, lim, x;
	short  sample;

	FILE   *outfile;

	getwaveinfo(fname1, &info1);
	getwaveinfo(fname2, &info2);

	strcpy(newfilename, fname1);
	getsafefilename(newfilename);
	printf("Output filename is %s\n", newfilename);

	time2 = (double)(info2.samples - 1) / info1.samprate;				// note the use of file1's samprate here
	maxtime = getmax(info1.time, time2 + offset);
	samples = (uint)floor(maxtime * info1.samprate) + 1;
	
	buf = zc(malloc(samples * info1.channels * 4));

	gensilence(buf, samples, info1.channels);
	mixtobuf(buf, fname1, (float)0.0, (float)0.0, (float)0.0);
	mixtobuf(buf, fname2, offset, level, pan);

	if (normlevel < .000001)
		normalize(buf, normlevel, samples);

	createheader(samples, info1.samprate, info1.channels, &hdr);

	outfile = fopen(newfilename, "wb");
	fwrite(&hdr, 1, 44, outfile);

	lim = samples * info1.channels;

	for (x = 0; x < lim; x++)
	{
		sample = clamp(buf[x]);
		fwrite(&sample, 1, 2, outfile);

		if (info1.channels == 2)
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

void mixtobuf(float *buf, uchar *infilename, float offset, float level, float pan)		// destructively mixes infile into buf
{
	struct wav_info info_in;
	struct panmult  pm;

	uint   x, y, samp_off, lim;

	short  *samp_in;
	
	short  samp_l, samp_r, samp_m;
    
	float dbmult, minus3;

	FILE   *infile;

	getwaveinfo(infilename, &info_in);
	infile = fopen(infilename, "rb");

	fseek(infile, info_in.dataoffset, SEEK_SET);

	samp_in = zc(malloc(info_in.datalength));

	fread(samp_in, 1, info_in.datalength, infile);

	samp_off = (uint)(offset * info1.samprate);
	
	dbmult = (float)pow(2.0, level / 6.0);
    minus3 = (float)pow(2.0, -0.5);
    
	dopancalcs(pan, &pm);

	lim = info_in.samples * info_in.channels;

	for (x = 0, y = 0; x < lim; x++)
	{
		if ((info_in.channels == 2) && (info1.channels == 2))			// stereo to stereo
		{
			buf[x + samp_off] += (float)samp_in[y++] * dbmult;
			buf[++x + samp_off] += (float)samp_in[y++] * dbmult;
		}
		else if ((info_in.channels == 1) && (info1.channels == 1))		// mono into mono
		{
			buf[x + samp_off] += (float)samp_in[y++] * dbmult;
		}
		else if ((info_in.channels == 1) && (info1.channels == 2))		// mono into stereo
		{
			samp_l = samp_in[y++];
			buf[x] += (float)dbmult * samp_l * pm.left;
			samp_r = samp_in[y++];
			buf[++x] += (float)dbmult * samp_r * pm.right;
		}
		else																// stereo into mono
		{
			samp_l = samp_in[y++];
			samp_r = samp_in[y++];
			samp_m = (samp_l + samp_r) / 2;
			buf[x] += dbmult * samp_m;
		}
	}

	free(samp_in);
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

	sprintf(where, "~mix1.wav");

	while (fileexists(fname))
		sprintf(where, "~mix%d.wav", ++x);
	
}

double getmax(double a, double b)
{
	return (a > b) ? a : b;
}

void gensilence(float *buf, uint samples, int channels)
{
	uint x;
	uint lim = samples * channels;

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
	mults->left = (float)dbmult_l;
	mults->right = (float)dbmult_r;
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

void normalize(float *sf, float normlevel, uint samples)
{
	uint x, lim;
	struct min_max2 mm;
    float  dbmult, ratio, ratio_top, ratio_bot, maxtop, minbot;

	mm.leftmin = (float)99999;
	mm.leftmax = (float)-99999;
	mm.rightmin = (float)99999;
	mm.rightmax = (float)-99999;

	lim = samples * info1.channels;

	for (x = 0; x < lim; x++)
	{
		mm.leftmin = getminf(mm.leftmin, sf[x]);
		mm.leftmax = getmaxf(mm.leftmax, sf[x]);
		
		if (info1.channels == 2)
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

float getmaxf(float a, float b)
{
	return (a > b) ? a : b;
}

float getminf(float a, float b)
{
	return (a < b) ? a : b;
}

void *zc(void *ptr)             // pointer sanity check for malloc
{
    if (ptr == NULL)
    {
		printf("Can't allocate memory!\n");
        exit(1);
		return 0;
    }
    
    return ptr;
}

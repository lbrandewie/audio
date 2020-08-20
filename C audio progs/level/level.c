//
// level.c
//
// app to get waveform level
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "level.h"

void main(int argc, char *argv[])
{
	double leveladj;
	struct wav_info info;
	struct min_max  mm;
	
	if (argc == 1)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

    getwaveinfo(argv[1], &info);
	scanwave(argv[1], &mm);
	leveladj = dothemath(&mm, 0);

	printf("File level is %.3lf\n", leveladj);
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

short getmax(short a, short b)
{
	return (a > b) ? a : b;
}

short getmin(short a, short b)
{
	return (a < b) ? a : b;
}

double getmaxd(double a, double b)
{
	return (a > b) ? a : b;
}

double getmind(double a, double b)
{
	return (a < b) ? a : b;
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

void scanwave(uchar *fname, struct min_max *mm)
{
	struct wav_info  info;
	struct sample    samp;

	FILE   *infile;

	uint   x;

	getwaveinfo(fname, &info);

	mm->leftmax = -32768;
	mm->leftmin = 32767;
	mm->rightmax = -32768;
	mm->rightmin = 32767;

	infile = fopen(fname, "rb");
	fseek(infile, info.dataoffset, SEEK_SET);

	for (x = 0; x < info.samples; x++)
	{
		fread(&samp, 2, info.channels, infile);

		switch (info.channels)
		{
			case 1:
				mm->leftmin = getmin(samp.left, mm->leftmin);
				mm->leftmax = getmax(samp.left, mm->leftmax);
				break;
			
			case 2:
				mm->leftmin = getmin(samp.left, mm->leftmin);
				mm->leftmax = getmax(samp.left, mm->leftmax);
				mm->rightmin = getmin(samp.right, mm->rightmin);
				mm->rightmax = getmax(samp.right, mm->rightmax);
		}
	}
	fclose(infile);
}

double dothemath(struct min_max *mm, double leveltarg)
{
	short  maxtop, minbot;
	double dbmult, ratio, ratio_top, ratio_bot;

	dbmult = pow(2, leveltarg / 6);
	
	maxtop = getmax(mm->leftmax, mm->rightmax);
	minbot = getmin(mm->leftmin, mm->rightmin);

	ratio_top = (double)maxtop / (32767 * dbmult);
	ratio_bot = (double)abs(minbot) / (32768 * dbmult);

	ratio = getmaxd(ratio_top, ratio_bot);

	return 6 * log(ratio) / log(2);
}


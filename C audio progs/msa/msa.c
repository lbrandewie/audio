//
// msa.c
//
// mid-sides analysis app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "msa.h"

struct wav_info info;

void main(int argc, char *argv[])
{
	struct min_max  mm;
	
	struct mid_sides *ms;

	if (argc == 1)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

    getwaveinfo(argv[1], &info);
	
	if (info.channels == 1)
	{
		printf("%s is not a stereo file.\n", argv[1]);
		exit(0);
	}

	ms = zc(malloc(info.samples * 8));
	cv_midsides(argv[1], ms);
	
	scan_ms(ms, &mm);
	dothemath(&mm);
}

void cv_midsides(uchar *fname, struct mid_sides *ms)
{
	FILE   *infile;
	struct sample samp;
	uint   x;

	infile = fopen(fname, "rb");
	fseek(infile, info.dataoffset, SEEK_SET);

	for(x = 0; x < info.samples; x++)
	{
		fread(&samp, 1, 4, infile);

		ms[x].mid = (float)(samp.left + samp.right) / 2;
		ms[x].sides = (float) samp.right - ms[x].mid;
	}
	
	fclose(infile);
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

float getmaxf(float a, float b)
{
	return (a > b) ? a : b;
}

float getminf(float a, float b)
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

void scan_ms(struct mid_sides *ms, struct min_max *mm)
{
	uint x;

	mm->midmax = (float)-1e10;
	mm->midmin = (float)1e10;
	mm->sidesmax = (float)-1e10;
	mm->sidesmin = (float)1e10;

	for (x = 0; x < info.samples; x++)
	{
		mm->midmin = getminf(ms[x].mid, mm->midmin);
		mm->midmax = getmaxf(ms[x].mid, mm->midmax);
		mm->sidesmin = getminf(ms[x].sides, mm->sidesmin);
		mm->sidesmax = getmaxf(ms[x].sides, mm->sidesmax);
	}
}

void dothemath(struct min_max *mm)
{
	float  midlevel, sideslevel;
	
	midlevel = getmaxf(mm->midmax / 32767, (-mm->midmin) / 32768);
	sideslevel = getmaxf(mm->sidesmax / 32767, (-mm->sidesmax) / 32768);
	
	printf("Mid level: %.3f\n", 6 * log(midlevel) / log(2));
	printf("Sides level: %.3f\n", 6 * log(sideslevel) / log(2));
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

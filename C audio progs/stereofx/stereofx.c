//
// stereofx.c
//
// stereo effects app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "stereofx.h"


struct wav_info info;
uint   clipped = 0;

void main(int argc, char *argv[])
{
	struct mid_sides *ms;
	struct sample_f  *sf;

	float level, dbmult, normlevel = (float)0.0;

	uchar  newfilename[1024];
	uint   x;

	if (argc < 3)
	{
		printf("Usage: %s wavefile level [normalize level]\n", argv[0]);
		exit(0);
	}

	sscanf(argv[2], "%f", &level);

	if (argc == 4)
		sscanf(argv[3], "%f", &normlevel);

	dbmult = (float)pow(2, level / 6);

    getwaveinfo(argv[1], &info);
	
	if (info.channels == 1)
	{
		printf("%s is not a stereo file.\n", argv[1]);
		exit(0);
	}

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

	printf("Output filename is %s\n", newfilename);
	
	ms = zc(malloc(info.samples * 2 * sizeof(float)));
	sf = zc(malloc(info.samples * 2 * sizeof(float)));

	getfile(argv[1], sf);
	cv_midsides(sf, ms);
	
	for (x = 0; x < info.samples; x++)
		ms[x].sides *= dbmult;

	cv_stereo(sf, ms);

	if ((argc == 4) && (normlevel < 0.000001))
		normalize(sf, normlevel);
	
	output(newfilename, sf);

	if (clipped)
		printf("Clipping detected, %d samples affected.\n", clipped);
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

void cv_midsides(struct sample_f *sf, struct mid_sides *ms)
{
	uint   x;

	for(x = 0; x < info.samples; x++)
	{
		ms[x].mid = (sf[x].left + sf[x].right) / 2;
		ms[x].sides = (float) sf[x].right - ms[x].mid;
	}
}

void cv_stereo(struct sample_f *sf, struct mid_sides *ms)
{
	uint   x;

	for (x = 0; x < info.samples; x++)
	{
		sf[x].right = ms[x].mid + ms[x].sides;
		sf[x].left  = ms[x].mid - ms[x].sides;
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

uchar *finddata(uchar *buf)
{
	int x;

	for (x = 0; x < 1024; x++)
		if (memcmp(&buf[x], "data", 4) == 0)
			return &buf[x];

	return NULL;
}

uchar *findperiod(uchar *buf)		// return a pointer to the last period in a filename
{
	int x;

	for (x = strlen(buf) - 1; x >= 0; x--)
		if (buf[x] == '.')
			return(&buf[x]);
	
	return NULL;
}

uint getdatalength(uchar *buf)
{
	uchar *where;

	where = finddata(buf);

	return *((uint *)where + 1);
}

void getfile(uchar *fname, struct sample_f *sf)
{
	FILE   *infile;

	struct sample samp;
	uint   x;

	infile = fopen(fname, "rb");
	fseek(infile, info.dataoffset, SEEK_SET);

	for (x = 0; x < info.samples; x++)
	{
		fread(&samp, 1, 4, infile);
		sf[x].left = samp.left;
		sf[x].right = samp.right;
	}
	
	fclose(infile);
}

float getmaxf(float a, float b)
{
	return (a > b) ? a : b;
}

float getminf(float a, float b)
{
	return (a < b) ? a : b;
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

	sprintf(where, "~sfx1.wav");

	while (fileexists(fname))
		sprintf(where, "~sfx%d.wav", ++x);
	
}

void getwaveinfo(uchar *fname, struct wav_info *info)
{
	FILE   *thefile;
	struct wav_hdr hdr;
	uchar  *data;
	int    diff;
	
	thefile = fopen(fname, "rb");
	
	if (thefile == NULL)
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

void normalize(struct sample_f *sf, float normlevel)
{
	uint x;
	struct min_max2 mm;
    float  dbmult, ratio, ratio_top, ratio_bot, maxtop, minbot;

	mm.leftmin = (float)99999;
	mm.leftmax = (float)-99999;
	mm.rightmin = (float)99999;
	mm.rightmax = (float)-99999;

	for (x = 0; x < info.samples; x++)
	{
		mm.leftmin = getminf(mm.leftmin, sf[x].left);
		mm.leftmax = getmaxf(mm.leftmax, sf[x].left);
		mm.rightmin = getminf(mm.rightmin, sf[x].right);
		mm.rightmax = getmaxf(mm.rightmax, sf[x].right);
	}

	dbmult = (float)pow(2, normlevel / 6);
	
	maxtop = getmaxf(mm.leftmax, mm.rightmax);
	minbot = getminf(mm.leftmin, mm.rightmin);

	ratio_top = (float)maxtop / (32767 * dbmult);
	ratio_bot = (float)fabs(minbot) / (32768 * dbmult);

	ratio = getmaxf(ratio_top, ratio_bot);

	dbmult = 1 / ratio;

	for (x = 0; x < info.samples; x++)
	{
		sf[x].left *= dbmult;
		sf[x].right *= dbmult;
	}
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

void output(uchar *fname, struct sample_f *sf)
{
	struct wav_hdr hdr;
	struct sample  samp;
	
	uint   x;

	FILE   *outfile;

	createheader(info.samples, info.samprate, 2, &hdr);

	outfile = fopen(fname, "wb");
	fwrite(&hdr, 1, 44, outfile);

	for (x = 0; x < info.samples; x++)
	{
		samp.left = clamp(sf[x].left);
		samp.right = clamp(sf[x].right);

		fwrite(&samp, 2, 2, outfile);
	}

	fclose(outfile);
}


void scan_ms(struct mid_sides *ms, struct min_max *mm)
{
	uint x;

	mm->midmax = (float)-32768;
	mm->midmin = (float)32767;
	mm->sidesmax = (float)-32768;
	mm->sidesmin = (float)32767;

	for (x = 0; x < info.samples; x++)
	{
		mm->midmin = getminf(ms[x].mid, mm->midmin);
		mm->midmax = getmaxf(ms[x].mid, mm->midmax);
		mm->sidesmin = getminf(ms[x].sides, mm->sidesmin);
		mm->sidesmax = getmaxf(ms[x].sides, mm->sidesmax);
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

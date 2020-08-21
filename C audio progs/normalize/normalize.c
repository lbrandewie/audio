//
// normalize.c
//
// normalize app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "normalize.h"

uint clipped = 0;

void main(int argc, char *argv[])
{
	double level = 0, leveladj;
	struct wav_info info;
	struct min_max  mm;
	
	uchar  newfilename[1024];

	if (argc == 1)
	{
		printf("Usage: %s wavefile [level]\n", argv[0]);
		exit(0);
	}

	if (argc == 3)
		sscanf(argv[2], "%lf", &level);

	if ((argc == 3) && (level > 0.00001))
	{
		printf("Levels above 0 not allowed (clipping)\n");
		exit(0);
	}

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

    getwaveinfo(argv[1], &info);
	scanwave(argv[1], &mm);
	leveladj = dothemath(&mm, level);

	gensilence(newfilename, info.time, info.samprate, info.channels);
    mixdestruct(newfilename, argv[1], 0.0, leveladj);

	if (clipped)
		printf("Clipping detected. %d samples affected.\n", clipped);
}

void mixdestruct(uchar *outfilename, uchar *infilename, double offset, double level)		// destructively mixes infile into outfile
{
	struct wav_info info_in, info_out;
	
	uint   x, samp_off;

	short  *indata, *outdata;
	short  *samp_in, *samp_out;
	short  samp_l, samp_r, samp_m;
    
	double dbmult, minus3;

	FILE   *infile, *outfile;

	getwaveinfo(outfilename, &info_out);
	getwaveinfo(infilename, &info_in);

	outfile = fopen(outfilename, "r+b");
	infile = fopen(infilename, "rb");

	fseek(infile, info_in.dataoffset, SEEK_SET);
	fseek(outfile, 44, SEEK_SET);

	indata = zc(malloc(info_in.datalength));
	outdata = zc(malloc(info_out.datalength));

	fread(indata, 1, info_in.datalength, infile);
	fread(outdata, 1, info_out.datalength, outfile);

	samp_off = (uint)(offset * info_out.samprate);
	
	samp_in = indata;							// set up data pointers
	samp_out = outdata + (samp_off * info_out.channels);

	dbmult = pow(2.0, level / 6.0);
	minus3 = pow(2.0, -0.5);

	for (x = 0; x < info_in.samples; x++)
	{
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
			*samp_out = clamp(*samp_out + dbmult * samp_l * minus3);
			samp_out++;
			*samp_out = clamp(*samp_out + dbmult * samp_l * minus3);
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

	sprintf(where, "~norm1.wav");

	while (fileexists(fname))
		sprintf(where, "~norm%d.wav", ++x);
	
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

	return -6 * log(ratio) / log(2);
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

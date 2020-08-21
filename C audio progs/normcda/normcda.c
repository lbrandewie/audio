//
// normcda.c
//
// normalize app (for cdaudio)
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "normcda.h"


uint clipped = 0;


void main(int argc, char *argv[])
{
	double level, leveladj;
	struct min_max  mm;
	uint   samples;

	uchar  newfilename[1024];

	if (argc == 1)
	{
		printf("Usage: %s cdafile [level]\n", argv[0]);
		exit(0);
	}

	if (argc == 3)
		sscanf(argv[2], "%lf", &level);

	if ((argc == 3) && (level > 0.00001))
	{
		printf("Levels above 0 not allowed for 16-bit files!\n");
		exit(0);
	}

	samples = getfilelength(argv[1]) / 4;

	strcpy(newfilename, argv[1]);
	getsafefilename(newfilename);

	scanwave(argv[1], &mm);
	leveladj = dothemath(&mm, level);

	gensilence(newfilename, samples);
    mixdestruct(newfilename, argv[1], 0.0, leveladj);

	if (clipped)
		printf("Clipping detected. %d samples affected.\n", clipped);
}

void mixdestruct(uchar *outfilename, uchar *infilename, double offset, double level)		// destructively mixes infile into outfile
{
	
	uint   x, indatalen, outdatalen, samples;

	short  *indata, *outdata;
	short  *samp_in, *samp_out;
	short  samp_l, samp_r;
    
	double dbmult;

	FILE   *infile, *outfile;

	outfile = fopen(outfilename, "r+b");
	infile = fopen(infilename, "rb");

	indatalen = getfilelength(infilename);
	outdatalen = getfilelength(outfilename);

	samples = indatalen / 4;

	indata = zc(malloc(indatalen));
	outdata = zc(malloc(outdatalen));

	fread(indata, 1, indatalen, infile);
	fread(outdata, 1, outdatalen, outfile);

	samp_in = indata;							// set up data pointers
	samp_out = outdata;

	dbmult = pow(2.0, level / 6.0);

	for (x = 0; x < samples; x++)
	{
		samp_l = *(samp_in++);
		samp_r = *(samp_in++);

		*samp_out = clamp(*samp_out + dbmult * samp_l);
		samp_out++;
		*samp_out = clamp(*samp_out + dbmult * samp_r);
		samp_out++;
	}

	fseek(outfile, 0, SEEK_SET);
	fwrite(outdata, 1, outdatalen, outfile);

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

	sprintf(where, "~norm1.cda");

	while (fileexists(fname))
		sprintf(where, "~norm%d.cda", ++x);
	
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

void gensilence(uchar *fname, uint samples)
{
	uint   x, sample = 0;

	FILE   *outfile;

	outfile = fopen(fname, "wb");

	if (outfile == NULL)
	{
		printf("Cannot open output file %s!\n", fname);
		exit(0);
	}

	for (x = 0; x < samples; x++)							// take advantage of the fact that a uint set to 0
		fwrite(&sample, 1, 4, outfile);						// has nothing but zero bits
    
    fclose(outfile);
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

uint getfilelength(uchar *fname)
{
	FILE  *infile;
	uint  ret;

	infile = fopen(fname, "rb");
	fseek(infile, 0, SEEK_END);

	ret = ftell(infile);
	fclose(infile);

	return ret;
}

void scanwave(uchar *fname, struct min_max *mm)
{
	struct sample samp;

	FILE   *infile;

	uint   x, samples;

	mm->leftmax = -32768;
	mm->leftmin = 32767;
	mm->rightmax = -32768;
	mm->rightmin = 32767;

	samples = getfilelength(fname) / 4;

	infile = fopen(fname, "rb");

	for (x = 0; x < samples; x++)
	{
		fread(&samp, 4, 1, infile);

		mm->leftmin = getmin(samp.left, mm->leftmin);
		mm->leftmax = getmax(samp.left, mm->leftmax);
		mm->rightmin = getmin(samp.right, mm->rightmin);
		mm->rightmax = getmax(samp.right, mm->rightmax);
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
    }
    return ptr;
}

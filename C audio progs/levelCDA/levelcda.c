//
// levelcda.c
//
// app to get waveform level (for cdaudio files)
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "levelcda.h"


void main(int argc, char *argv[])
{
	double leveladj;
	struct min_max  mm;
	uint   samples;
	
	if (argc == 1)
	{
		printf("Usage: %s cdafile\n", argv[0]);
		exit(0);
	}

    samples = getfilelength(argv[1]) / 4;
	scanwave(argv[1], samples, &mm);
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

void scanwave(uchar *fname, uint samples, struct min_max *mm)
{
	struct sample    samp;

	FILE   *infile;

	uint   x;

	mm->leftmax = -32768;
	mm->leftmin = 32767;
	mm->rightmax = -32768;
	mm->rightmin = 32767;

	infile = fopen(fname, "rb");

	for (x = 0; x < samples; x++)
	{
		fread(&samp, 2, 2, infile);

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

	return 6 * log(ratio) / log(2);
}


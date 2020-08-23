//
// GlobalProcs.cpp
//
// global routines for audio apps
//


#define uint unsigned int

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream.h>

#include "GlobalProcs.h"


void createheader(uint samples, uint samprate, short channels, wav_hdr *hdr)
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

void die(char *msg)
{
    FILE *log;
    cout << msg << endl;
    
    log = fopen("logfile.txt", "a");
    fprintf(log, "%s\n", msg);
    exit(1);
}

void dopancalcs(float pan, panmult *mults)          // take a pan number (-1 to 1) and generate the channel multipliers
{
	float level_r, dbmult_r, power_r;
	float level_l, dbmult_l, power_l;

	if (pan > 0)
	{
		level_r = (1 - pan) * -3;
		dbmult_r = (float) pow(2, level_r / 6);
		power_r = dbmult_r * dbmult_r;

		power_l = 1 - power_r;
		dbmult_l = (float) sqrt(power_l);
	}
	else
	{
		level_l = (pan + 1) * -3;
		dbmult_l = (float) pow(2, level_l / 6);
		power_l = dbmult_l * dbmult_l;

		power_r = 1 - power_l;
		dbmult_r = (float) sqrt(power_r);
	}
	mults->left = dbmult_l;
	mults->right = dbmult_r;
}
    
float dothemath16(struct min_max *mm, float leveltarg)          // determine by how many decibels a signal must be amplfied to reach a given level
{
	short  maxtop, minbot;
	float  dbmult, ratio, ratio_top, ratio_bot;

	dbmult = (float)pow(2, leveltarg / 6);
	
	maxtop = getmax(mm->leftmax, mm->rightmax);
	minbot = getmin(mm->leftmin, mm->rightmin);

	ratio_top = (float)maxtop / (32767 * dbmult);
	ratio_bot = (float)abs(minbot) / (32768 * dbmult);

	ratio = getmaxf(ratio_top, ratio_bot);

	return (float)(6 * log(ratio) / log(2));
}

float dothemath32(struct min_maxf *mm, float leveltarg)         // same as previous but for 32-bit buffers
{
	float  maxtop, minbot;
	float  dbmult, ratio, ratio_top, ratio_bot;

	dbmult = (float)pow(2, leveltarg / 6);
	
	maxtop = getmaxf(mm->leftmax, mm->rightmax);
	minbot = getminf(mm->leftmin, mm->rightmin);

	ratio_top = (float)(maxtop / (32767.0 * dbmult));
	ratio_bot = (float)(fabs(minbot) / (32768.0 * dbmult));

	ratio = getmaxf(ratio_top, ratio_bot);

	return (float) (6 * log(ratio) / log(2));
}

int fileexists(char *fname)		    // returns 1 if a file exists
{
	FILE *thefile;

	thefile = fopen(fname, "r");

	if (thefile == NULL)
		return 0;

	fclose(thefile);
	return 1;
}

int fileexists2(char *fname)        // returns 1 if a file exists and the user doesn't want to overwrite
{
	FILE *thefile;
    char buf[20];
    
	thefile = fopen(fname, "r");

	if (thefile == NULL)
		return 0;

	printf("file %s exists! overwrite? ", fname);
	fgets(buf, 19, stdin);

	if ((buf[0] != 'y') && (buf[0] != 'Y'))
		return 1;

	return 0;
}

char *finddata(char *buf)           // find the data block in a wave file header
{
	int x;

	for (x = 0; x < 1024; x++)
		if (memcmp(&buf[x], "data", 4) == 0)
			return &buf[x];

	return NULL;
}

char *findperiod(char *buf)		    // return a pointer to the last period in a filename
{
	int x = strlen(buf) - 1;

	for (; x >= 0; x--)
		if (buf[x] == '.')
			return(&buf[x]);
	
	return NULL;
}

uint getdatalength(char *buf)       // get the data length from a wave file header
{
	char *where;

	where = finddata(buf);

	return *((uint *)where + 1);
}

uint getfilelength(char *fname)
{
	FILE  *infile;
	uint  ret;

	infile = fopen(fname, "rb");
	fseek(infile, 0, SEEK_END);

	ret = ftell(infile);
	fclose(infile);

	return ret;
}

short getmax(short a, short b)
{
	return (a > b) ? a : b;
}

short getmin(short a, short b)
{
	return (a < b) ? a : b;
}

float getmaxf(float a, float b)
{
	return (a > b) ? a : b;
}

float getminf(float a, float b)
{
	return (a < b) ? a : b;
}

void getwaveinfo(char *fname, wav_info *info)           // get all relevant info from a wave file header
{
	struct wav_hdr hdr;
	char   *data;
	int    diff;
	
	FILE *thefile = fopen(fname, "rb");
	
	if (thefile == NULL)			// fuggeddaboudit
	{
		printf("Cannot open file %s!\n", fname);
		exit(1);
	}

	fread(&hdr, 1, 1024, thefile);
	fclose(thefile);

	info->bits = hdr.bits;
	info->channels = hdr.channels;
	info->format = hdr.format;
	info->samprate = hdr.samprate;
	
	data = finddata((char *)&hdr);

	if (data == NULL)
	{
		printf("Can't find data header: %s\n", fname);
		exit(0);
	}
	
	diff = data - (char *)&hdr;

	info->datalength = getdatalength((char *)&hdr);
	info->dataoffset = diff + 8;

	info->samples = info->datalength / hdr.bytealign;
	info->time = (float)(info->samples - 1) / hdr.samprate;

	if (info->bits != 16)
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

void lcase(char *arg)
{
	uint x, len = strlen(arg);

	for (x = 0; x < len; x++)
		arg[x] = tolower(arg[x]);
}

int nosuchfile(char *fname)		    // returns 1 if a file fails to exist
{
	FILE *thefile;

	thefile = fopen(fname, "r");

	if (thefile == NULL)
	{
		printf("Can't open file %s\n", fname);
		return 1;
	}
	else
	{
		fclose(thefile);
		return 0;
	}
}


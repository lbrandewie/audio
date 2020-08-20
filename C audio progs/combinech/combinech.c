//
// combinech.c
//
// channel combiner app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "combinech.h"


void main(int argc, char *argv[])
{
	struct wav_info info1, info2;
	struct wav_hdr  hdr;

	short  *buf1, *buf2;
	
	uint   x, max_samp;

	FILE   *infile1, *infile2, *outfile;

	uchar  newfilename[1024];
	uchar  *where;

	if (argc == 1)
	{
		printf("Usage: %s wavefile1 wavefile2\n", argv[0]);
		exit(0);
	}

	getwaveinfo(argv[1], &info1);
	getwaveinfo(argv[2], &info2);

	strcpy(newfilename, argv[1]);

	where = findperiod(newfilename);
	sprintf(where, "~comb.wav");
	
	if (fileexists(newfilename)) exit(0);

	max_samp = getmaxd(info1.samples, info2.samples);

	buf1 = zc(malloc(2 * max_samp));
	buf2 = zc(malloc(2 * max_samp));

	memset(buf1, 0, 2 * max_samp);
	memset(buf2, 0, 2 * max_samp);

	infile1 = fopen(argv[1], "rb");
	fseek(infile1, info1.dataoffset, SEEK_SET);
	fread(buf1, 1, info1.datalength, infile1);
	fclose(infile1);

	infile2 = fopen(argv[2], "rb");
	fseek(infile2, info2.dataoffset, SEEK_SET);
	fread(buf2, 1, info2.datalength, infile2);
	fclose(infile2);

	createheader(max_samp, info1.samprate, 2, &hdr);

	outfile = fopen(newfilename, "wb");
	fwrite(&hdr, 1, 44, outfile);

	for (x = 0; x < max_samp; x++)
	{
		fwrite(&buf1[x], 1, 2, outfile);
		fwrite(&buf2[x], 1, 2, outfile);
	}
	
	fclose(outfile);
	free(buf1);
	free(buf2);
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

int fileexists(uchar *fname)		// returns 1 if a file exists and the user doesn't want to overwrite
{
	FILE *thefile;
    char buf[20];
    
	thefile = fopen(fname, "r");

	if (thefile == NULL)
		return 0;

	fclose(thefile);

	printf("File %s exists! Overwrite? ", fname);
	fgets(buf, 19, stdin);

	if ((buf[0] != 'y') && (buf[0] != 'Y'))
		return 1;
	else
		return 0;
}

uchar *finddata(uchar *buf)
{
	int x;

	for (x = 0; x < 1024; x++)
		if (memcmp(&buf[x], "data", 4) == 0)
			return(&buf[x]);

	return(NULL);
}

uint getmaxd(uint a, uint b)
{
	return (a > b) ? a : b;
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

	if (info->channels != 1)
	{
		printf("Not a mono file: %s\n", fname);
		exit(0);
	}
}

void *zc(void *ptr)             // pointer sanity check for malloc
{
    if (ptr == NULL)
    {
		printf("Can't allocate memory!\n");
		exit(1);
		return NULL;			// keep compiler happy...
    }
    
    return ptr;
}

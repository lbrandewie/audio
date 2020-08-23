//
// splitchan.c
//
// channel splitter app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "splitchan.h"


void main(int argc, char *argv[])
{
	struct wav_info info;
	struct wav_hdr  hdr;

	struct sample   samp;
	
	uint   x;

	FILE   *infile, *outfile1, *outfile2;

	uchar  newname1[1024];
	uchar  newname2[1024];
	uchar  *where;

	if (argc == 1)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

	getwaveinfo(argv[1], &info);

	if (info.channels != 2)
	{
		printf("Not a stereo file: %s\n", argv[1]);
		exit(0);
	}

	strcpy(newname1, argv[1]);
	strcpy(newname2, argv[1]);

	where = findperiod(newname1);
	sprintf(where, "~left.wav");
	where = findperiod(newname2);
	sprintf(where, "~right.wav");

	if (fileexists(newname1)) exit(0);
	if (fileexists(newname2)) exit(0);

	infile = fopen(argv[1], "rb");
	fseek(infile, info.dataoffset, SEEK_SET);

	outfile1 = fopen(newname1, "wb");
	outfile2 = fopen(newname2, "wb");

    createheader(info.samples, info.samprate, 1, &hdr);
    
	fwrite(&hdr, 1, 44, outfile1);
	fwrite(&hdr, 1, 44, outfile2);

	for (x = 0; x < info.samples; x++)
	{
		fread(&samp, 1, 4, infile);
		fwrite(&samp.left, 1, 2, outfile1);
		fwrite(&samp.right, 1, 2, outfile2);
	}

	fclose(infile);
	fclose(outfile1);
	fclose(outfile2);
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

	printf("File %s exists! Overwrite? ");
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
		exit(1);
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
		exit(1);
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


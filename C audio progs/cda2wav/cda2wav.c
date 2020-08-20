//
// cda2wav.c
//
// cdaudio-to-wave app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cda2wav.h"

void main(int argc, char *argv[])
{
	struct sample   samp;
	struct wav_hdr  hdr;

	uint   x, len, lim;

	FILE   *infile, *outfile;

	uchar  newname[1024];
	uchar  *where;

	if (argc == 1)
	{
		printf("Usage: %s cdafile\n", argv[0]);
		exit(0);
	}

	if (nosuchfile(argv[1])) exit(0);

	len = getfilelength(argv[1]);
	lim = len / 4;

	strcpy(newname, argv[1]);

	where = findperiod(newname);
	sprintf(where, ".wav");
	
	if (fileexists(newname)) exit(0);

	infile = fopen(argv[1], "rb");
	outfile = fopen(newname, "wb");
	
	createheader(lim, 44100, 2, &hdr);
	fwrite(&hdr, 1, 44, outfile);

	for (x = 0; x < lim; x++)
	{
		fread(&samp, 1, 4, infile);
		fwrite(&samp, 1, 4, outfile);
	}

	fclose(infile);
	fclose(outfile);
}

void createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr)
{
	uint datasize = samples * 2 * channels;
	
	memcpy(&hdr->riff, "RIFF", 4);          // set up constant fields
	memcpy(&hdr->wave, "WAVEfmt ", 8);
	memcpy(&hdr->data, "data", 4);

	hdr->length1 = datasize + 36;           // calculated fields
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


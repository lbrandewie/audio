//
// wav2cda.c
//
// wave-to-cdaudio app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "wav2cda.h"


void main(int argc, char *argv[])
{
	struct wav_info info;
	struct sample   samp;
	
	uint   x;

	FILE   *infile, *outfile;

	uchar  newname[1024];
	uchar  *where;

	if (argc == 1)
	{
		printf("Usage: %s wavefile\n", argv[0]);
		exit(0);
	}

	getwaveinfo(argv[1], &info);

	strcpy(newname, argv[1]);

	where = findperiod(newname);
	sprintf(where, ".cda");
	
	if (nosuchfile(argv[1])) exit(0);
	if (fileexists(newname)) exit(0);

	infile = fopen(argv[1], "rb");
	fseek(infile, info.dataoffset, SEEK_SET);

	outfile = fopen(newname, "wb");

	for (x = 0; x < info.samples; x++)
	{
		fread(&samp, 1, 4, infile);
		fwrite(&samp, 1, 4, outfile);
	}

	fclose(infile);
	fclose(outfile);
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
	if (info->channels != 2)
	{
		printf("%s is not a stereo file\n", fname);
		exit(0);
	}
	if (info->samprate != 44100)
	{
		printf("Unsupported sample rate: %d\n", info->samprate);
		exit(0);
	}
}


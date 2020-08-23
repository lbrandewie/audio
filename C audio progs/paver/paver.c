//
// paver.c
//
// implement a wave-file paver
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "paver.h"


void main(int argc, char *argv[])
{
    int    x, y, len, channels = 2;
    uint   samprate = 0;
	double time = 0.0;

	if (argc == 1)
	{
		printf("Usage: %s wavfile [options]\n", argv[0]);
        printf("Options: -t (time) -s (samprate) [-c (channels)]\n");
        exit(0);
	}
    
    for (x = 2; x < argc; x += 2)
    {
		len = strlen(argv[x]);

		for (y = 0; y < len; y++)
			argv[x][y] = tolower(argv[x][y]);

		if (strcmp(argv[x], "-t") == 0)
		{
			sscanf(argv[x + 1], "%lf", &time);
			printf("Time set to %.1lf\n", time);
		}
		else if (strcmp(argv[x], "-s") == 0)
		{
			sscanf(argv[x + 1], "%ld", &samprate);

			if (islegal(samprate))
			{
				printf("Sample rate set to %ld\n", samprate);
			}
			else
			{
				printf("Unsupported sample rate: %d\n", samprate);
				exit(0);
			}
		}
		else if (strcmp(argv[x], "-c") == 0)
		{
			sscanf(argv[x + 1], "%d", &channels);

			if ((channels < 1) || (channels > 2))
			{
				printf("Unsupported number of channels: %d\n", channels);
				exit(0);
			}

			printf("Channels set to %d\n", channels);
		}
		else
		{
			printf("I don't understand argument %s, quitting\n", argv[x]);
			exit(0);
		}
	}

	if (time < 0.001)
	{
		printf("Must set time (-t) parameter\n");
		exit(0);
	}

	if (samprate == 0)
	{
		printf("Must set sample rate (-s) parameter\n");
		exit(0);
	}
	
	gensilence(argv[1], time, samprate, channels);
}

void gensilence(char *fname, double time, uint samprate, int channels)
{
	struct wav_hdr hdr;
	uint   x, samples, sample = 0;

	FILE   *outfile;

	samples = (uint)floor(time * samprate) + 1;
	createheader(samples, samprate, channels, &hdr);

	if (fileexists(fname))
	{
		printf("Aborting.\n");
		exit(0);
	}
	
	outfile = fopen(fname, "wb");

	if (outfile == NULL)
	{
		printf("Cannot open output file %s!\n", fname);
		exit(0);
	}

	fwrite(&hdr, 1, 44, outfile);		// write header

	for (x = 0; x < samples; x++)
		fwrite(&sample, 1, 2 * channels, outfile);
    
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

int islegal(uint samprate)
{
    if ((samprate >= 4000) && (samprate <= 96000))
        return 1;
	
	return 0;
}

int fileexists(uchar *fname)		// returns 1 if a file exists and the user doesn't want to overwrite it
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


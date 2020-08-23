//
// paver.h
//
// declares for paver app
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#define uchar unsigned char
#define uint unsigned int

struct wav_hdr
{
	uchar riff[4];
	uint  length1;
	uchar wave[8];
    uint  length2;
	short format;
	short channels;
	uint  samprate;
	uint  bytespersec;
	short bytealign;
	short bits;
	uchar data[4];
	uint  length3;
};

void createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr);
int  fileexists(uchar *fname);
void gensilence(char *fname, double time, uint samprate, int channels);
int  islegal(uint samprate);

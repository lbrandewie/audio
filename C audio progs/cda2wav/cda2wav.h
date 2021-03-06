//
// cda2wav.h
//
// declares for cdaudio-to-wave app
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

struct sample
{
	short left;
	short right;
};

struct wav_info
{
	short  bits;
	short  channels;
	short  format;
	uint   samprate;
	uint   samples;
	uint   datalength;
	uint   dataoffset;
	double time;
};

void  createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr);
int   fileexists(uchar *fname);
uchar *findperiod(uchar *buf);
uint  getfilelength(uchar *fname);
int   nosuchfile(uchar *fname);

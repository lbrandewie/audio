//
// delay.h
//
// declares for delay app
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
    uchar extra[980];
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

struct sample_f
{
	float left;
	float right;
};

struct min_max2
{
	float leftmin;
	float leftmax;
	float rightmin;
	float rightmax;
};


short clamp(double x);
void  createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr);
uchar *finddata(uchar *buf);
uchar *findperiod(uchar *buf);
void  gensilence(float *buf, uint samples, int channels);
uint  getdatalength(uchar *buf);
void  getsafefilename(uchar *fname);
void  getwaveinfo(uchar *fname, struct wav_info *info);
void  mixtobuf(float *buf, uchar *infname, double offset, double level);
void  normalize(float *sf, float normlevel);
int   nosuchfile(char *fname);
void  *zc(void *);


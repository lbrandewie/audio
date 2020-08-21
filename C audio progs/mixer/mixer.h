//
// mixer.h
//
// declares for mixer app
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

struct sample
{
	short left;
	short right;
};

struct panmult
{
	float left;
	float right;
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

struct min_max2
{
	float leftmin;
	float leftmax;
	float rightmin;
	float rightmax;
};


short  clamp(double x);
void   createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr);
int    fileexists(uchar *fname);
uchar  *finddata(uchar *buf);
uchar  *findperiod(uchar *fname);
void   gensilence(float *buf, uint samples, int channels);
uint   getdatalength(uchar *buf);
double getmax(double a, double b);
void   getsafefilename(uchar *fname);
void   mixtobuf(float *buf, uchar *fname, float offset, float level, float pan);
void   mixwaves(uchar *fname1, uchar *fname2, float offset, float level, float normlevel, float pan);
int    nosuchfile(uchar *fname);
void   dopancalcs(double pan, struct panmult *mults);
void   getwaveinfo(uchar *fname, struct wav_info *info);
void   normalize(float *sf, float normlevel, uint samples);
float  getmaxf(float a, float b);
float  getminf(float a, float b);
void   *zc(void *);


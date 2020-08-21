//
// normalize.h
//
// declares for normalize app
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

struct min_max
{
	short leftmin;
	short leftmax;
	short rightmin;
	short rightmax;
};

short  clamp(double x);
void   createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr);
double dothemath(struct min_max *mm, double leveltarg);
int    fileexists(uchar *fname);
uchar  *finddata(uchar *buf);
uchar  *findperiod(uchar *buf);
void   gensilence(uchar *fname, double time, uint samprate, int channels);
uint   getdatalength(uchar *buf);
short  getmax(short a, short b);
double getmaxd(double a, double b);
short  getmin(short a, short b);
double getmind(double a, double b);
void   getsafefilename(uchar *fname);
void   getwaveinfo(uchar *fname, struct wav_info *info);
void   mixdestruct(uchar *outfile, uchar *infile, double offset, double level);
int    nosuchfile(uchar *fname);
void   scanwave(uchar *fname, struct min_max *mm);
void   *zc(void *ptr);

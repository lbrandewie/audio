//
// panwipe.h
//
// declares for pan wiper app
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
	double left;
	double right;
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


double calcpan(uint samplenum, uint samprate, double delay, double ct, int pattern, double cycles);
short  clamp(double x);
void   createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr);
int    fileexists(uchar *fname);
uchar  *finddata(uchar *buf);
uchar  *findperiod(uchar *fname);
void   gensilence(uchar *fname, double time, uint samprate, int channels);
uint   getdatalength(uchar *buf);
double getmax(double a, double b);
void   getsafefilename(uchar *fname);
void   mixdestruct(uchar *outfile, uchar *infile, double offset, double level,  double delay, double ct, int pattern, double cycles);
void   mixwaves(uchar *fname1, uchar *fname2, double offset, double level, double inputgain, double pan);
int    nosuchfile(uchar *fname);
void   dopancalcs(double pan, struct panmult *mults);
void   getwaveinfo(uchar *fname, struct wav_info *info);
void   *zc(void *);

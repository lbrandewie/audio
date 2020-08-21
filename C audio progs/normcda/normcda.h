//
// normcda.h
//
// declares for normalize app (for cdaudio)
//
// copyright 2020 L.P. "Lars" Brandewie. All rights reserved.
//

#define uchar unsigned char
#define uint unsigned int

struct sample
{
	short left;
	short right;
};

struct min_max
{
	short leftmin;
	short leftmax;
	short rightmin;
	short rightmax;
};

short  clamp(double x);
double dothemath(struct min_max *mm, double leveltarg);
int    fileexists(uchar *fname);
uchar  *finddata(uchar *buf);
uchar  *findperiod(uchar *buf);
void   gensilence(uchar *fname, uint samples);
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
uint   getfilelength(uchar *fname);
void   *zc(void *ptr);


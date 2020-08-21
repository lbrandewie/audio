//
// levelcda.h
//
// declares for app to get level (cdaudio version)
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

double dothemath(struct min_max *mm, double leveltarg);
int    fileexists(uchar *fname);
short  getmax(short a, short b);
short  getmin(short a, short b);
double getmaxd(double a, double b);
int    nosuchfile(uchar *fname);
void   scanwave(uchar *fname, uint samples, struct min_max *mm);
uint   getfilelength(uchar *fname);

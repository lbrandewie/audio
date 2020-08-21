//
// msa.h
//
// declares for mid-sides analysis app
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

struct mid_sides
{
	float mid;
	float sides;
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
	float midmin;
	float midmax;
	float sidesmin;
	float sidesmax;
};


void  cv_midsides(uchar *fname, struct mid_sides *ms);
int   nosuchfile(uchar *fname);
int   fileexists(uchar *fname);
float getmaxf(float a, float b);
float getminf(float a, float b);
uchar *finddata(uchar *buf);
uint  getdatalength(uchar *buf);
void  getwaveinfo(uchar *fname, struct wav_info *info);
void  scan_ms(struct mid_sides *ms, struct min_max *mm);
void  dothemath(struct min_max *mm);
void  *zc(void *);






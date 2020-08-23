//
// stereofx.h
//
// declares for stereo effects app
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

struct sample_f
{
	float left;
	float right;
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

struct min_max2
{
	float leftmin;
	float leftmax;
	float rightmin;
	float rightmax;
};


short clamp(double x);
void  createheader(uint samples, uint samprate, int channels, struct wav_hdr *hdr);
void  cv_midsides(struct sample_f *sf, struct mid_sides *ms);
void  cv_stereo(struct sample_f *sf, struct mid_sides *ms);
int   fileexists(uchar *fname);
uchar *finddata(uchar *buf);
uchar *findperiod(uchar *buf);
uint  getdatalength(uchar *buf);
void  getfile(uchar *fname, struct sample_f *sf);
float getmaxf(float a, float b);
float getminf(float a, float b);
void  getsafefilename(uchar *fname);
void  getwaveinfo(uchar *fname, struct wav_info *info);
void  normalize(struct sample_f *sf, float normlevel);
int   nosuchfile(uchar *fname);
void  output(uchar *fname, struct sample_f *sf);
void  scan_ms(struct mid_sides *ms, struct min_max *mm);
void  *zc(void *ptr);








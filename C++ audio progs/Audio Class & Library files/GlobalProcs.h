//
// globalprocs.h
//
// declarations for global funcs, audio project
//

#ifndef GLOBALPROCS_H

#define GLOBALPROCS_H

#define uint unsigned int


struct sample {
    short left;
    short right;
};

struct min_max {
    short leftmin;
    short leftmax;
    short rightmin;
    short rightmax;
};

struct min_maxf {
    float leftmin;
    float leftmax;
    float rightmin;
    float rightmax;
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

struct wav_hdr
{
	char riff[4];
	uint  length1;
	char wave[8];
    uint  length2;
	short format;
	short channels;
	uint  samprate;
	uint  bytespersec;
	short bytealign;
	short bits;
	char  data[4];
	uint  length3;
	char  extra[980];
};

void   createheader(uint, uint, short, wav_hdr *);
void   die(char *);
float  dothemath16(min_max *, float);
float  dothemath32(min_maxf *, float);
int    fileexists(char *);
int    fileexists2(char *);
char   *finddata(char *);
char   *findperiod(char *);
uint   getdatalength(char *);
uint   getfilelength(char *);
short  getmax(short, short);
short  getmin(short, short);
float  getmaxf(float, float);
float  getminf(float, float);
void   getwaveinfo(char *, wav_info *);
int    nosuchfile(char *);
void   dopancalcs(float, panmult *);
void   lcase(char *);

#endif

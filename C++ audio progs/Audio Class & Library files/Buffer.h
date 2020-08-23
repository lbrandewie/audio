//
// Buffer.h
//
// declares for Buffer base class
//


#define uint  unsigned int


#ifndef BUFFER_H

#define BUFFER_H

#include "GlobalProcs.h"


class Buffer
{
public:
    Buffer();
	Buffer(uint, uint, short, short);
    ~Buffer();
    short   clamp(float);
    void    clearbuf();
    void    cv_midsides(Buffer *, Buffer *);
	short   getbits();
    short   getchannels();
    uint    getclipped();
    void    *getdata();
    short   getLeft16(uint);
    float   getLeft32(uint);
    short   getRight16(uint);
    float   getRight32(uint);
    uint    getsamples();
    uint    getsamprate();
    float   gettime();
    void    mix32(Buffer *, float, float, float);
    void    normalize16(float);
    void    normalize32(float);
    void    saveascdafile(char *);
    void    saveaswavefile(char *);
	void    saveaswavefile32(char *);
    void    scanbuf16(min_max *);
    void    scanbuf32(min_maxf *);
	void    setbits(short);
	void    setchannels(short);
    void    setclipped(uint);
	void    setdata(void *);
	void    setLeft16(uint, short);
	void    setLeft32(uint, float);
	void    setRight16(uint, short);
	void    setRight32(uint, float);
	void    setsamples(uint);
	void    setsamprate(uint);
	void    settime(float);
private:
    uint   samples;
    uint   samprate;
    uint   clipped;
    short  channels;
	short  bits;
    float  time;
    void   *data;
};
    
#endif
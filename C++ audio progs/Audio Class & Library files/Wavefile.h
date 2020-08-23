//
// wavefile.h
//
// declares for wavefile class
//

#ifndef WAVEFILE_H

#define WAVEFILE_H

#include "GlobalProcs.h"
#include "Buffer.h"


class Wavefile : public Buffer
{
public:
    Wavefile();
    ~Wavefile();
    Wavefile(char *);
	Wavefile(char *, short);
	Wavefile(uint, uint, short);
    void setformat(short);
private:
    short format;
    short error;
};

#endif
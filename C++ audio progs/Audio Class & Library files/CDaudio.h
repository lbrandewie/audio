//
// CDaudio.h
//
// declares for CDaudio file class
//


#ifndef CDAUDIO_H

#define CDAUDIO_H


#include "GlobalProcs.h"
#include "Buffer.h"


class CDaudio : public Buffer
{
public:
    CDaudio();
    ~CDaudio();
    CDaudio(char *);
private:
    short error;
};

#endif
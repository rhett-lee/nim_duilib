#pragma once

struct APNGDATA
{
    unsigned char * pdata;
    unsigned short *pDelay;
    int   nWid,nHei;
    int   nFrames;
    int   nLoops;
};

APNGDATA * LoadAPNG_from_memory(const char * pBuf, size_t nLen);

void APNG_Destroy(APNGDATA *apng);

#ifndef DECODER_APNG_H_
#define DECODER_APNG_H_

//功能说明: 通过libpng + libpng-apng.patch的方式，支持APNG格式
struct APNGDATA
{
    unsigned char* pdata;
    unsigned short* pDelay;
    int nWid, nHei;
    int nFrames;
    int nLoops;
};

/** 通过内存数据加载APNG图片数据
* @return 返回加载后的图片数据
*/
APNGDATA* LoadAPNG_from_memory(const char* pBuf, size_t nLen, bool bLoadAllFrames, unsigned int& nFrameCount);

/** 释放APNG图片数据
* @param [in] apng 由LoadAPNG_from_memory返回的值
*/
void APNG_Destroy(APNGDATA* apng);

#endif //DECODER_APNG_H_

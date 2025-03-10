#include "ZipStreamIO.h"

namespace ui 
{
ZipStreamIO::ZipStreamIO(uint8_t* pData, uint32_t nDataLen):
    m_pData(pData),
    m_nDataLen((int32_t)nDataLen),
    m_nCurPos(0)
{
    ASSERT(m_pData != nullptr);
    ASSERT(m_nDataLen > 0);
    if (m_nDataLen < 0) {
        m_nDataLen = 0;
    }
}

ZipStreamIO::~ZipStreamIO()
{
}

voidpf ZCALLBACK ZipStreamIO::fopen_file_func(voidpf opaque, const char* /*filename*/, int /*mode*/)
{
    ASSERT(opaque != nullptr);
    voidpf stream = nullptr;
    if (opaque != nullptr) {
        ZipStreamIO* pStreamIO = (ZipStreamIO*)opaque;
        ASSERT(pStreamIO->m_pData != nullptr);
        ASSERT(pStreamIO->m_nDataLen > 0);
        pStreamIO->m_nCurPos = 0;
        if ((pStreamIO->m_pData != nullptr) && (pStreamIO->m_nDataLen > 0)) {
            stream = opaque;
        }
    }
    return stream;
}

uLong ZCALLBACK ZipStreamIO::fread_file_func(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    ASSERT_UNUSED_VARIABLE(opaque == stream);
    ASSERT_UNUSED_VARIABLE(opaque == stream);
    ASSERT(stream != nullptr);
    if (stream == nullptr) {
        return 0;
    }
    ZipStreamIO* pStreamIO = (ZipStreamIO*)stream;
    uLong nRet = 0;
    long nSize = (long)size;
    if ((nSize > 0) && (pStreamIO->m_pData != nullptr) && (pStreamIO->m_nDataLen > 0)) {
        if ((pStreamIO->m_nCurPos + nSize) <= pStreamIO->m_nDataLen) {
            //数据还没结束
            if (buf != nullptr) {
                ::memcpy(buf, pStreamIO->m_pData + pStreamIO->m_nCurPos, (size_t)nSize);
            }
            pStreamIO->m_nCurPos += nSize;
            nRet = nSize;
        }
        else {
            //读取完即结束
            if (pStreamIO->m_nCurPos < pStreamIO->m_nDataLen) {
                nRet = pStreamIO->m_nDataLen - pStreamIO->m_nCurPos;
                if (buf != nullptr) {
                    ::memcpy(buf, pStreamIO->m_pData + pStreamIO->m_nCurPos, (size_t)nRet);
                }
                pStreamIO->m_nCurPos += nRet;
            }
        }
    }
    return nRet;
}

uLong ZCALLBACK ZipStreamIO::fwrite_file_func(voidpf opaque, voidpf stream, const void* /*buf*/, uLong /*size*/)
{
    ASSERT_UNUSED_VARIABLE(opaque == stream);
    ASSERT(!"ZipStreamIO::fwrite_file_func!!");
    return 0;
}

long ZCALLBACK ZipStreamIO::ftell_file_func(voidpf opaque, voidpf stream)
{
    ASSERT_UNUSED_VARIABLE(opaque == stream);
    ASSERT(stream != nullptr);
    if (stream == nullptr) {
        return 0;
    }
    ZipStreamIO* pStreamIO = (ZipStreamIO*)stream;
    long nRet = (long)pStreamIO->m_nCurPos;
    return nRet;
}

long ZCALLBACK ZipStreamIO::fseek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    ASSERT_UNUSED_VARIABLE(opaque == stream);
    ASSERT(stream != nullptr);
    if (stream == nullptr) {
        return 0;
    }
    long nRet = 0;
    ZipStreamIO* pStreamIO = (ZipStreamIO*)stream;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR:
        pStreamIO->m_nCurPos += (int32_t)offset;
        break;
    case ZLIB_FILEFUNC_SEEK_END:
        pStreamIO->m_nCurPos = pStreamIO->m_nDataLen - (int32_t)offset;
        break;
    case ZLIB_FILEFUNC_SEEK_SET:
        pStreamIO->m_nCurPos = (int32_t)offset;
        break;
    default:
        nRet = -1;
    }
    if (pStreamIO->m_nCurPos > pStreamIO->m_nDataLen) {
        pStreamIO->m_nCurPos = pStreamIO->m_nDataLen;
        nRet = -1;
    }
    else if (pStreamIO->m_nCurPos < 0) {
        pStreamIO->m_nCurPos = 0;
        nRet = -1;
    }
    ASSERT(nRet == 0);
    return nRet;
}

int ZCALLBACK ZipStreamIO::fclose_file_func(voidpf opaque, voidpf stream)
{
    ASSERT_UNUSED_VARIABLE(opaque == stream);
    ASSERT(stream != nullptr);
    if (stream != nullptr) {
        ZipStreamIO* pStreamIO = (ZipStreamIO*)stream;
        pStreamIO->m_nCurPos = 0;
    }
    return 0;
}

int ZCALLBACK ZipStreamIO::ferror_file_func(voidpf opaque, voidpf stream)
{
    ASSERT_UNUSED_VARIABLE(opaque == stream);
    return 0;
}

void ZipStreamIO::FillFopenFileFunc(zlib_filefunc_def* pzlib_filefunc_def)
{
    ASSERT(pzlib_filefunc_def != nullptr);
    if (pzlib_filefunc_def == nullptr) {
        return;
    }
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = this;
}

} //namespace ui 

#ifndef UI_CORE_ZIP_STREAM_IO_H_
#define UI_CORE_ZIP_STREAM_IO_H_

#include "duilib/duilib_defs.h"
#include "duilib/third_party/zlib/contrib/minizip/ioapi.h"

namespace ui 
{
/** Image列表
 */
class UILIB_API ZipStreamIO
{
public:
    /** 构造内存数据
    * @param [in] pData 内存数据的起始地址
    * @param [in] nDataLen 内存数据的长度
    */
    ZipStreamIO(uint8_t* pData, uint32_t nDataLen);
    ~ZipStreamIO();
    ZipStreamIO(const ZipStreamIO&) = delete;
    ZipStreamIO& operator = (const ZipStreamIO&) = delete;

public:

    /** 填充unzip的IO回调函数接口
    */
    void FillFopenFileFunc(zlib_filefunc_def* pzlib_filefunc_def);

private:

    //数据读取操作
    static voidpf ZCALLBACK fopen_file_func(voidpf opaque, const char* filename, int mode);
    static uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, void* buf, uLong size);
    static uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size);
    static long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream);
    static long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin);
    static int ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream);
    static int ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream);

private:
    /** 内存数据的起始地址
    */
    uint8_t* m_pData;

    /** 内存数据的长度
    */
    int32_t m_nDataLen;

    /** 当前数据位置
    */
    int32_t m_nCurPos;
};

} //namespace ui 

#endif //UI_CORE_ZIP_STREAM_IO_H_

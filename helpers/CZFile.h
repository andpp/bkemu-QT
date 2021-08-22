#ifndef CZFILE_H
#define CZFILE_H

#include "CFile.h"
#include "basetypes.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <zlib.h>
#include <unistd.h>
#include <stdlib.h>

class ZFile : public CFile
{
private:
    Bytef * m_pData;
    size_t m_nDataSize;
    size_t m_nPos;

public:
    ZFile() : CFile(), m_pData(nullptr), m_nDataSize(0), m_nPos(0) {}
    ~ZFile() {
        Close();
    }

    virtual bool Open(const char *name, int mode)
    {
        bool res = CFile::Open(name, mode);
        if(res) {
            off_t out_size = 1024*1024;
            m_pData = (Bytef *)malloc(out_size * sizeof(Bytef));
            void *data = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, m_hFile, 0);
            m_nDataSize = decompress((Bytef*)data, fsize, m_pData, out_size);
            munmap(data, fsize);
            if(m_nDataSize > 0) {
                Bytef *p = (Bytef *)realloc(m_pData, m_nDataSize);
                if(p)
                    m_pData = p;
             }
             else
                res = false;
        }

        return res;

    }
    inline bool Open(std::string &name, int mode)
    {
        return Open(name.c_str(), mode);
    };

    inline bool Open(const CString& name, int mode) {
        return Open(name.toLocal8Bit().data(), mode);
    }

    virtual int GetLength() {return m_nDataSize;}

    virtual int Read(void *dst, int len) {
        if(m_hFile >= 0) {
            size_t readSize = min(m_nDataSize - m_nPos, len);
            memcpy(dst, m_pData + m_nPos, readSize);
            m_nPos += readSize;
            return readSize;
        } else
            throw CFileException();
        return 0;
    };
    virtual int Write(void *dst, int len) {
        if(m_hFile  >= 0) {
            size_t readSize = min(m_nDataSize - m_nPos, len);
            memcpy(m_pData + m_nPos, dst, readSize);
            m_nPos += readSize;
            return readSize;
        } else
            throw CFileException();
        return 0;
    };
    virtual int Close() {
        if(m_hFile  >= 0) {
            CFile::Close();
            if(m_pData)
                free(m_pData);

            m_pData = nullptr;
        }
        return 0;
    };
    virtual int Seek(uint pos, int s) {
        if(m_hFile  >= 0) {
            if(s == CFile::begin) {
                m_nPos = pos;
            } else if (s == CFile::current) {
                m_nPos += pos;
                if(m_nPos > m_nDataSize)
                    m_nPos = m_nDataSize;
            } else {
                m_nPos = m_nDataSize;
            }
            return m_nPos;
        } else {
            throw CFileException();
        }
        return 0;
    };
    virtual int SeekToBegin() {
        m_nPos = 0;
      return 0;
    };

private:
    size_t decompress(Bytef *src, uLong src_len, Bytef *dst, uLong dst_len)
    {
        z_stream strm;
        memset(&strm, 0, sizeof(z_stream));
        strm.zalloc = Z_NULL;
        strm.zfree  = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in   = src;
        strm.next_out  = dst;
        strm.avail_in  = src_len;
        strm.avail_out = dst_len;

        while (strm.total_in < src_len)
        {
            if(inflateInit2(&strm, (MAX_WBITS + 16)) == Z_OK) {
                if(decompress_block(&strm) < 0) {
                   return 0;
                }
            } else {
                return 0;
            }
        }
        return strm.total_out;
    }

    int decompress_block(z_stream *strm)
    {
        int ret;
        do
        {
            ret = inflate(strm, Z_NO_FLUSH);
            if(    ret == Z_STREAM_ERROR
                || ret == Z_NEED_DICT
                || ret == Z_MEM_ERROR
                || ret == Z_DATA_ERROR
                || ret == Z_BUF_ERROR
                || strm->avail_out <= 0)
                    return -1;
        }
        while (ret != Z_STREAM_END);
        inflateEnd(strm);
        return 0;
    }
};


#endif // CZFILE_H

#ifndef _CFILE_H
#define _CFILE_H

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <limits.h>
#include <stdio.h>
#include <QBuffer>
#include <QByteArray>

#include "CString.h"

#define _MAX_PATH PATH_MAX

//#define POSIX_LOCK

class CFileException {
public:
    CFileException() {};
};

class CFileStatus
{
public:
    CFileStatus() = default;
};

class CFile {
protected:
    int fsize;
    struct flock fl;
    bool locked;

public:
    const static int modeCreate = O_CREAT;
    const static int modeRead = O_RDONLY;
    const static int modeWrite = O_WRONLY;
    const static int modeReadWrite = O_RDWR;
    const static int shareDenyWrite = 0x20000;
    const static int shareDenyNone =  0x20000;
    const static int begin = SEEK_SET;
    const static int current = SEEK_CUR;
    const static int end = SEEK_END;
    const static int hFileNull = -1;

    const static int typeText = 0x10000;
    const static int typeBinary = 0;
    const static int modeTRead = O_RDONLY | typeText;
    const static int modeTWrite = O_WRONLY | typeText;
    const static int modeTReadWrite = O_RDWR | typeText;

    const static int osRandomAccess = 0;

    int m_hFile;

    CFile() : fsize(0), locked(false), m_hFile(-1) {}
    ~CFile() {
        if(m_hFile >= 0)
            Close();
    }

    virtual bool Open(const char *name, int mode)
    {
        m_hFile = open(name, mode & 0xFFFF, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if(m_hFile < 0)
            return false;
        fsize = lseek(m_hFile, 0, SEEK_END);
        lseek(m_hFile, 0L, SEEK_SET);
        if(mode & shareDenyWrite) {
#ifdef POSIX_LOCK
            fl.l_type = F_WRLCK;
            fl.l_whence = SEEK_SET;
            fl.l_start = 0;
            fl.l_len = 0;
            if (fcntl(m_hFile, F_SETLKW, &fl) != -1) {
                locked = true;
              }
#else  // Linux style Lock
            fl.l_type = F_WRLCK;
            fl.l_whence = SEEK_SET;
            fl.l_start = 0;
            fl.l_len = 0;
            fl.l_pid = 0;
            if (fcntl(m_hFile, F_OFD_SETLKW, &fl) != -1) {
                locked = true;
            }
#endif
          }
        return true;

    }
    inline bool Open(std::string &name, int mode)
    {
        return Open(name.c_str(), mode);
    };

    inline bool Open(const CString& name, int mode) {
        return Open(name.toLocal8Bit().data(), mode);
    }

    int GetLength() {return fsize;}
    virtual int Read(void *dst, int len) {
        if(m_hFile >= 0)
            return read(m_hFile, dst, len);
        else
            throw CFileException();
        return 0;
    };
    virtual int Write(void *dst, int len) {
        if(m_hFile  >= 0)
            return write(m_hFile, dst, len);
        else
            throw CFileException();
        return 0;
    };
    virtual int Close() {
        if(m_hFile  >= 0) {
           if(locked) {
#ifdef POSIX_LOCK
               fl.l_type = F_UNLCK;
                 if (fcntl(m_hFile, F_SETLK, &fl) == -1) {
                 }
#else
               fl.l_type = F_UNLCK;
                 if (fcntl(m_hFile, F_OFD_SETLK, &fl) == -1) {
                 }
#endif
           }
           close(m_hFile);
           m_hFile = -1;
           fsize = 0;
           locked = false;
        }
        return 0;
    };
    virtual int Seek(uint pos, int s) {
        if(m_hFile  >= 0) {
            return lseek(m_hFile, pos, s);
        } else {
            throw CFileException();
        }
        return 0;
    };
    virtual int SeekToBegin() {
      return Seek(0,CFile::begin);
    };

    static bool GetStatus(const CString filename, CFileStatus &fs)
    {
        (void)fs;
        QFileInfo check_file(filename);
        return (check_file.exists() && check_file.isFile());
    }
};

class CStdioFile : public CFile
{
public:
    FILE *m_pStream;

public:
    CStdioFile() : CFile(), m_pStream(nullptr) {}
    ~CStdioFile() {
        if(m_pStream)
            Close();
    }

    bool Open(std::string &name, int mode)
    {
        bool res = CFile::Open(name, mode);
        const char *smode;
        switch (mode  & 0xFFFF) {
            case O_RDONLY: smode = "rb"; break;
            case O_WRONLY: smode = "wb"; break;
            case O_RDWR: smode = "rwb"; break;
        }
        if (res)
            m_pStream = fdopen(m_hFile, smode);
        return res;
    };
    bool Open(const CString& name, int mode) {
        bool res = CFile::Open(name, mode);
        const char *smode;
        switch ((mode & ~O_CREAT) & 0x1FFFF) {
            case O_RDONLY: smode = "rb"; break;
            case O_WRONLY: smode = "wb"; break;
            case O_RDWR: smode = "rwb"; break;
            case modeTRead: smode = "r"; break;
            case modeTWrite: smode = "w"; break;
            case modeTReadWrite: smode = "rw"; break;
        }
        if (res)
            m_pStream = fdopen(m_hFile, smode);
        return res;
    }

    int Close() {
        fclose(m_pStream);
        m_pStream = nullptr;
        return true;
    }

    int ReadString(CString &str) {
        char *buff = nullptr;
        size_t buff_size;
        int res = getline(&buff, &buff_size, m_pStream);
        if (res <= 0)
            return 0;
        str = buff;
        free(buff);
        return res;
    }

    int WriteString(const CString &str) {
        return fputs(str.toLocal8Bit().data(), m_pStream);
    }

    int WriteString(const char *str) {
        return fputs(str, m_pStream);
    }
};

class CMemFile {
#ifdef USE_QBUFFER
    QBuffer    m_pBuffer;
    QByteArray m_pDataArr;

public:
    CMemFile(uint8_t *pBuff, uint nSize, uint blkSize) {
         (void)blkSize;
        m_pDataArr = QByteArray::fromRawData((const char *)pBuff, nSize);
        m_pBuffer.setBuffer(&m_pDataArr);
        m_pBuffer.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    }

    ~CMemFile() {
        m_pBuffer.close();
    }

    int Write(const CString &str, const uint size) {
        return m_pBuffer.write(str.toLocal8Bit().data(), size);
    }
    int GetLength() { return m_pBuffer.pos(); }
    int Close() {
        m_pBuffer.close();
        return 0;
    }
    int Read(void *dst, uint size) {
        return m_pBuffer.read((char *)dst, size);
    }
    uint GetPosition() { return m_pBuffer.pos(); }


#else

    uint8_t *m_pBuff;
    size_t m_nSize;
    uint m_nIncSize;
    uint m_nPosition;
public:
    CMemFile(uint8_t *pBuff, uint nSize, uint blkSize) :
        m_pBuff(pBuff), m_nSize(nSize), m_nIncSize(blkSize), m_nPosition(0)
    { }
    ~CMemFile() {
//        free(m_pBuff);
    }
    int Write(const CString &str, const uint size) {
        if (m_nPosition + size > m_nSize) {
            size_t newSize = m_nSize + m_nIncSize;
            while (newSize < m_nPosition + size)
                newSize += m_nIncSize;
            m_pBuff = (uint8_t *)realloc(m_pBuff, newSize);
            m_nSize = newSize;
        }
        memcpy(&m_pBuff[m_nPosition], str.toLatin1().data(), size);
        m_nPosition += size;
        return size;
    }
    int GetLength() { return m_nPosition; }
    int Close() {
        m_nPosition = 0;
        return 0;
    }
    int Read(void *dst, uint size) {
        if(m_nPosition + size > m_nSize)
            size = m_nSize - m_nPosition;
        memcpy(dst, &m_pBuff[m_nPosition], size);
        m_nPosition += size;
        return size;
    }
    uint GetPosition() { return m_nPosition; }
#endif
};

#endif // _CFILE_H

#if 0

#ifndef _CFILE_H
#define _CFILE_H

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#include "CString.h"
#include <QFile>
#include <QTextStream>

#define CStdioFile CFile

class CFile : public QFile{
public:
    const static constexpr OpenMode modeCreate = QIODevice::Truncate;
    const static constexpr OpenMode  modeRead = QIODevice::ReadOnly;
    const static constexpr OpenMode  modeWrite = QIODevice::WriteOnly;
    const static constexpr OpenMode  modeReadWrite = QIODevice::ReadWrite;
    const static constexpr OpenMode  shareDenyWrite = QIODevice::ReadOnly;
//    const static int begin = SEEK_SET;
//    const static int current = SEEK_CUR;
//    const static int end = SEEK_END;
//    const static int hFileNull = -1;
    const static constexpr OpenMode  typeText = QIODevice::Text;


//    int m_hFile;
    QFile *m_QFile;

    CFile() : m_QFile(NULL) {}
    bool Open(const CString& name, OpenMode mode)
    {
        m_QFile = new QFile(name);
        if (!m_QFile->open(mode)) {
            return false;
        }
        return true;
    };

    int GetLength() {
        if(m_QFile)
            return m_QFile->size();
        else
            return 0;
    }
    int Read(void *dst, int len) {
        if(m_QFile)
            return m_QFile->read((char *)dst, (qint64)len);
        else
            return 0;
    };

    int Write(void *src, int len) {
        if(m_QFile)
            return m_QFile->write((const char *)src, (qint64)len);
        else
            return 0;
    };

    int Close() {
        if(m_QFile) {
           m_QFile->close();
           delete m_QFile;
           m_QFile = NULL;
        }
        return 0;
    };

    int Seek(int pos, int s) {
        if(m_QFile) {
            return m_QFile->seek((qint64)pos);
        } else {
            return 0;
        }
    };

    int ReadString(CString &s) {
        if(m_QFile) {
            s = CString(m_QFile->readLine(256).data());
            return s.size();
        } else {
            return 0;
        }
     }

    int WriteString(CString &s) {
        if(m_QFile) {
            m_QFile->write(s.toLocal8Bit());
            return s.size();
        } else {
            return 0;

        }
    }

    void WriteString(const char *str) {
        if(m_QFile) {
            m_QFile->write(str);
        } else {

        }
    }

    void WriteString(const wchar_t *str) {
        if(m_QFile) {
            m_QFile->write(CString(str).toLocal8Bit());
        } else {

        }
    }


};

#endif // _CFILE_H

#if 0

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <winnetwk.h>
#include <shlobj.h>
#include <shellapi.h>
#include <Strsafe.h>
#include "sal.h"

AFX_STATIC void AFXAPI _AfxFillExceptionInfo(CFileException* pException,LPCTSTR lpszFileName)
{
    if (pException != NULL)
    {
        pException->m_lOsError = ::GetLastError();
        pException->m_cause =
                CFileException::OsErrorToException(pException->m_lOsError);

        // use passed file name (not expanded vesion) when reporting
        // an error while opening

        pException->m_strFileName = lpszFileName;
    }
}

AFX_STATIC BOOL AFXAPI _AfxFullPath2(__out_ecount_z(_MAX_PATH) LPTSTR lpszPathOut, LPCTSTR lpszFileIn,CFileException* pException);

#define new DEBUG_NEW

AFX_STATIC inline BOOL IsDirSep(TCHAR ch)
{
    return (ch == '\\' || ch == '/');
}

#ifndef _AFX_NO_OLE_SUPPORT

#undef DEFINE_GUID

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C AFX_COMDAT const GUID afx##name \
    = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define DEFINE_SHLGUID(name, l, w1, w2) DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

DEFINE_SHLGUID(CLSID_ShellLink, 0x00021401L, 0, 0);
#ifndef _UNICODE
DEFINE_SHLGUID(IID_IShellLinkA, 0x000214EEL, 0, 0);
#else
DEFINE_SHLGUID(IID_IShellLinkW, 0x000214F9L, 0, 0);
#endif
#define CLSID_ShellLink afxCLSID_ShellLink

#undef IID_IShellLink
#undef IShellLink
#ifndef _UNICODE
#define IID_IShellLink afxIID_IShellLinkA
#define IShellLink IShellLinkA
#else
#define IID_IShellLink afxIID_IShellLinkW
#define IShellLink IShellLinkW
#endif

#endif !_AFX_NO_OLE_SUPPORT

////////////////////////////////////////////////////////////////////////////
// CFile implementation

const HANDLE CFile::hFileNull = INVALID_HANDLE_VALUE;

CFile::CFile()
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_bCloseOnDelete = FALSE;
}

CFile::CFile(HANDLE hFile)
{
    ASSERT(hFile != INVALID_HANDLE_VALUE);
#ifdef _DEBUG
    DWORD dwFlags=0;
    ASSERT(GetHandleInformation(hFile,&dwFlags) != 0 );
#endif
    m_hFile = hFile;
    m_bCloseOnDelete = FALSE;
}

CFile::CFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
    ASSERT(AfxIsValidString(lpszFileName));
    m_hFile = INVALID_HANDLE_VALUE;

    CFileException e;
    if (!Open(lpszFileName, nOpenFlags, &e))
        AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
}

CFile::~CFile()
{
    AFX_BEGIN_DESTRUCTOR

            if (m_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete)
            Close();

    AFX_END_DESTRUCTOR
}

CFile* CFile::Duplicate() const
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    CFile* pFile = new CFile();
    HANDLE hFile;
    if (!::DuplicateHandle(::GetCurrentProcess(), m_hFile,
                           ::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        delete pFile;
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
    }
    pFile->m_hFile = hFile;
    ASSERT(pFile->m_hFile != INVALID_HANDLE_VALUE);
    pFile->m_bCloseOnDelete = m_bCloseOnDelete;
    return pFile;
}

BOOL CFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags,
                 CFileException* pException)
{
    ASSERT_VALID(this);
    ASSERT(AfxIsValidString(lpszFileName));

    ASSERT(pException == NULL ||
           AfxIsValidAddress(pException, sizeof(CFileException)));
    ASSERT((nOpenFlags & typeText) == 0);   // text mode not supported

    // shouldn't open an already open file (it will leak)
    ASSERT(m_hFile == INVALID_HANDLE_VALUE);

    // CFile objects are always binary and CreateFile does not need flag
    nOpenFlags &= ~(UINT)typeBinary;

    m_bCloseOnDelete = FALSE;

    m_hFile = INVALID_HANDLE_VALUE;
    m_strFileName.Empty();

    TCHAR szTemp[_MAX_PATH];
    if (lpszFileName != NULL && SUCCEEDED(StringCchLength(lpszFileName, _MAX_PATH, NULL)) )
    {
        if( _AfxFullPath2(szTemp, lpszFileName,pException) == FALSE )
            return FALSE;
    }
    else
    {
        // user passed in a buffer greater then _MAX_PATH
        if (pException != NULL)
        {
            pException->m_cause = CFileException::badPath;
            pException->m_strFileName = lpszFileName;
        }
        return FALSE; // path is too long
    }

    m_strFileName = szTemp;
    ASSERT(shareCompat == 0);

    // map read/write mode
    ASSERT((modeRead|modeWrite|modeReadWrite) == 3);
    DWORD dwAccess = 0;
    switch (nOpenFlags & 3)
    {
    case modeRead:
        dwAccess = GENERIC_READ;
        break;
    case modeWrite:
        dwAccess = GENERIC_WRITE;
        break;
    case modeReadWrite:
        dwAccess = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        ASSERT(FALSE);  // invalid share mode
    }

    // map share mode
    DWORD dwShareMode = 0;
    switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
    {
    default:
        ASSERT(FALSE);  // invalid share mode?
    case shareCompat:
    case shareExclusive:
        dwShareMode = 0;
        break;
    case shareDenyWrite:
        dwShareMode = FILE_SHARE_READ;
        break;
    case shareDenyRead:
        dwShareMode = FILE_SHARE_WRITE;
        break;
    case shareDenyNone:
        dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
        break;
    }

    // Note: typeText and typeBinary are used in derived classes only.

    // map modeNoInherit flag
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

    // map creation flags
    DWORD dwCreateFlag;
    if (nOpenFlags & modeCreate)
    {
        if (nOpenFlags & modeNoTruncate)
            dwCreateFlag = OPEN_ALWAYS;
        else
            dwCreateFlag = CREATE_ALWAYS;
    }
    else
        dwCreateFlag = OPEN_EXISTING;

    // special system-level access flags

    // Random access and sequential scan should be mutually exclusive
    ASSERT((nOpenFlags&(osRandomAccess|osSequentialScan)) != (osRandomAccess|
                                                              osSequentialScan) );

    DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
    if (nOpenFlags & osNoBuffer)
        dwFlags |= FILE_FLAG_NO_BUFFERING;
    if (nOpenFlags & osWriteThrough)
        dwFlags |= FILE_FLAG_WRITE_THROUGH;
    if (nOpenFlags & osRandomAccess)
        dwFlags |= FILE_FLAG_RANDOM_ACCESS;
    if (nOpenFlags & osSequentialScan)
        dwFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

    // attempt file creation
    HANDLE hFile = ::CreateFile(lpszFileName, dwAccess, dwShareMode, &sa,
                                dwCreateFlag, dwFlags, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        _AfxFillExceptionInfo(pException,lpszFileName);
        return FALSE;
    }
    m_hFile = hFile;
    m_bCloseOnDelete = TRUE;

    return TRUE;
}

UINT CFile::Read(void* lpBuf, UINT nCount)
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    if (nCount == 0)
        return 0;   // avoid Win32 "null-read"

    ASSERT(lpBuf != NULL);
    ASSERT(AfxIsValidAddress(lpBuf, nCount));

    DWORD dwRead;
    if (!::ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL))
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

    return (UINT)dwRead;
}

void CFile::Write(const void* lpBuf, UINT nCount)
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    if (nCount == 0)
        return;     // avoid Win32 "null-write" option

    ASSERT(lpBuf != NULL);
    ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

    DWORD nWritten;
    if (!::WriteFile(m_hFile, lpBuf, nCount, &nWritten, NULL))
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

    // Win95 will not return an error all the time (usually DISK_FULL)
    if (nWritten != nCount)
        AfxThrowFileException(CFileException::diskFull, -1, m_strFileName);
}

ULONGLONG CFile::Seek(LONGLONG lOff, UINT nFrom)
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);
    ASSERT(nFrom == begin || nFrom == end || nFrom == current);
    ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

    LARGE_INTEGER liOff;

    liOff.QuadPart = lOff;
    liOff.LowPart = ::SetFilePointer(m_hFile, liOff.LowPart, &liOff.HighPart,
                                     (DWORD)nFrom);
    if (liOff.LowPart  == (DWORD)-1)
        if (::GetLastError() != NO_ERROR)
            CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

    return liOff.QuadPart;
}

ULONGLONG CFile::GetPosition() const
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    LARGE_INTEGER liPos;
    liPos.QuadPart = 0;
    liPos.LowPart = ::SetFilePointer(m_hFile, liPos.LowPart, &liPos.HighPart , FILE_CURRENT);
    if (liPos.LowPart == (DWORD)-1)
        if (::GetLastError() != NO_ERROR)
            CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

    return liPos.QuadPart;
}

void CFile::Flush()
{
    ASSERT_VALID(this);

    if (m_hFile == INVALID_HANDLE_VALUE)
        return;

    if (!::FlushFileBuffers(m_hFile))
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

void CFile::Close()
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);
    BOOL bError = FALSE;
    if (m_hFile != INVALID_HANDLE_VALUE)
        bError = !::CloseHandle(m_hFile);

    m_hFile = INVALID_HANDLE_VALUE;
    m_bCloseOnDelete = FALSE;
    m_strFileName.Empty();

    if (bError)
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

void CFile::Abort()
{
    ASSERT_VALID(this);
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        // close but ignore errors
        ::CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
    m_strFileName.Empty();
}

void CFile::LockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    ULARGE_INTEGER liPos;
    ULARGE_INTEGER liCount;

    liPos.QuadPart = dwPos;
    liCount.QuadPart = dwCount;
    if (!::LockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,
                    liCount.HighPart))
    {
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
    }
}

void CFile::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    ULARGE_INTEGER liPos;
    ULARGE_INTEGER liCount;

    liPos.QuadPart = dwPos;
    liCount.QuadPart = dwCount;
    if (!::UnlockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,
                      liCount.HighPart))
    {
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
    }
}

void CFile::SetLength(ULONGLONG dwNewLen)
{
    ASSERT_VALID(this);
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    Seek(dwNewLen, (UINT)begin);

    if (!::SetEndOfFile(m_hFile))
        CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

ULONGLONG CFile::GetLength() const
{
    ASSERT_VALID(this);

    ULARGE_INTEGER liSize;
    liSize.LowPart = ::GetFileSize(m_hFile, &liSize.HighPart);
    if (liSize.LowPart == INVALID_FILE_SIZE)
        if (::GetLastError() != NO_ERROR)
            CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

    return liSize.QuadPart;
}

// CFile does not support direct buffering (CMemFile does)
UINT CFile::GetBufferPtr(UINT nCommand, UINT /*nCount*/,
                         void** /*ppBufStart*/, void** /*ppBufMax*/)
{
    ASSERT(nCommand == bufferCheck);
    UNUSED(nCommand);    // not used in retail build

    return 0;   // no support
}

void PASCAL CFile::Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
{
    if (!::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName))
        CFileException::ThrowOsError((LONG)::GetLastError(), lpszOldName);
}

void PASCAL CFile::Remove(LPCTSTR lpszFileName)
{
    if (!::DeleteFile((LPTSTR)lpszFileName))
        CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
}


/////////////////////////////////////////////////////////////////////////////
// CFile implementation helpers

#ifdef AfxGetFileName
#undef AfxGetFileName
#endif

#ifndef _AFX_NO_OLE_SUPPORT

HRESULT AFX_COM::CreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                                REFIID riid, LPVOID* ppv)
{
    // find the object's class factory
    LPCLASSFACTORY pf = NULL;

    if (ppv == NULL)
        return E_INVALIDARG;
    *ppv = NULL;

    HRESULT hRes = GetClassObject(rclsid, IID_IClassFactory, (LPVOID*)&pf);
    if (FAILED(hRes))
        return hRes;

    if (pf == NULL)
        return E_POINTER;

    // call it to create the instance
    hRes = pf->CreateInstance(pUnkOuter, riid, ppv);

    // let go of the factory
    pf->Release();
    return hRes;
}

HRESULT AFX_COM::GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;
    HINSTANCE hInst = NULL;

    // find server name for this class ID

    CString strCLSID = AfxStringFromCLSID(rclsid);
    CString strServer;
    if (!AfxGetInProcServer(strCLSID, strServer))
        return REGDB_E_CLASSNOTREG;

    // try to load it
    hInst = AfxCtxLoadLibrary(strServer);
    if (hInst == NULL)
        return REGDB_E_CLASSNOTREG;

#pragma warning(disable:4191)
    // get its entry point
    HRESULT (STDAPICALLTYPE* pfn)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    pfn = (HRESULT (STDAPICALLTYPE*)(REFCLSID rclsid, REFIID riid, LPVOID* ppv))
            GetProcAddress(hInst, "DllGetClassObject");
#pragma warning(default:4191)

    // call it, if it worked
    if (pfn != NULL)
        return pfn(rclsid, riid, ppv);
    return CO_E_ERRORINDLL;
}

CString AFXAPI AfxStringFromCLSID(REFCLSID rclsid)
{
    TCHAR szCLSID[256];
    _stprintf_s(szCLSID, _countof(szCLSID),
                _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
                rclsid.Data1, rclsid.Data2, rclsid.Data3,
                rclsid.Data4[0], rclsid.Data4[1], rclsid.Data4[2], rclsid.Data4[3],
            rclsid.Data4[4], rclsid.Data4[5], rclsid.Data4[6], rclsid.Data4[7]);
    return szCLSID;
}

BOOL AFXAPI AfxGetInProcServer(LPCTSTR lpszCLSID, CString& str)
{
    HKEY hKey = NULL;
    BOOL b = FALSE;
    LPTSTR lpsz = str.GetBuffer(_MAX_PATH);
    DWORD dwSize = _MAX_PATH * sizeof(TCHAR);
    DWORD dwType = REG_NONE;
    LONG lRes = ~ERROR_SUCCESS;

    if (RegOpenKey(HKEY_CLASSES_ROOT, _T("CLSID"), &hKey) == ERROR_SUCCESS)
    {
        HKEY hKeyCLSID = NULL;
        if (RegOpenKey(hKey, lpszCLSID, &hKeyCLSID) == ERROR_SUCCESS)
        {
            HKEY hKeyInProc = NULL;
            if (RegOpenKey(hKeyCLSID, _T("InProcServer32"), &hKeyInProc) ==
                    ERROR_SUCCESS)
            {
                lRes = ::RegQueryValueEx(hKeyInProc, _T(""),
                                         NULL, &dwType, (BYTE*)lpsz, &dwSize);
                b = (lRes == ERROR_SUCCESS);
                RegCloseKey(hKeyInProc);
            }
            RegCloseKey(hKeyCLSID);
        }
        RegCloseKey(hKey);
    }
    str.ReleaseBuffer();
    return b;
}
#endif  //!_AFX_NO_OLE_SUPPORT


BOOL AFXAPI AfxResolveShortcut(CWnd* pWnd, LPCTSTR lpszFileIn,
                               __out_ecount(cchPath) LPTSTR lpszFileOut, int cchPath)
{
    AFX_COM com;
    IShellLink* psl = NULL;
    *lpszFileOut = 0;   // assume failure

    if (!pWnd)
        return FALSE;

    SHFILEINFO info;
    if ((SHGetFileInfo(lpszFileIn, 0, &info, sizeof(info),
                       SHGFI_ATTRIBUTES) == 0) || !(info.dwAttributes & SFGAO_LINK))
    {
        return FALSE;
    }

    if (FAILED(com.CreateInstance(CLSID_ShellLink, NULL, IID_IShellLink,
                                  (LPVOID*)&psl)) || psl == NULL)
    {
        return FALSE;
    }

    IPersistFile *ppf = NULL;
    if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)))
    {
        CStringW strFileIn(lpszFileIn);
        if (ppf != NULL && SUCCEEDED(ppf->Load(strFileIn.GetString(), STGM_READ)))
        {
            /* Resolve the link, this may post UI to find the link */
            if (SUCCEEDED(psl->Resolve(pWnd->GetSafeHwnd(),
                                       SLR_ANY_MATCH)))
            {
                psl->GetPath(lpszFileOut, cchPath, NULL, 0);
                ppf->Release();
                psl->Release();
                return TRUE;
            }
        }
        if (ppf != NULL)
            ppf->Release();
    }
    psl->Release();
    return FALSE;
}


// turn a file, relative path or other into an absolute path
BOOL AFXAPI _AfxFullPath2(__out_ecount_z(_MAX_PATH) LPTSTR lpszPathOut, LPCTSTR lpszFileIn, CFileException* pException)
// lpszPathOut = buffer of _MAX_PATH
// lpszFileIn = file, relative path or absolute path
// (both in ANSI character set)
// pException - pointer to exception object - can be NULL.
{
    pException;
    ENSURE(lpszPathOut);
    ENSURE(lpszFileIn);
    ASSERT(AfxIsValidAddress(lpszPathOut, _MAX_PATH));

    // first, fully qualify the path name
    LPTSTR lpszFilePart;
    DWORD dwRet = GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);
    if (dwRet == 0)
    {
#ifdef _DEBUG
        if (lpszFileIn != NULL && lpszFileIn[0] != '\0')
            TRACE(traceAppMsg, 0, _T("Warning: could not parse the path '%s'.\n"), lpszFileIn);
#endif
        Checked::tcsncpy_s(lpszPathOut, _MAX_PATH, lpszFileIn, _TRUNCATE); // take it literally
        _AfxFillExceptionInfo(pException,lpszFileIn);
        return FALSE;
    }
    else if (dwRet >= _MAX_PATH)
    {
#ifdef _DEBUG
        if (lpszFileIn[0] != '\0')
            TRACE1("Warning: could not parse the path '%s'. Path is too long.\n", lpszFileIn);
#endif
        // GetFullPathName() returned a path greater than _MAX_PATH
        if (pException != NULL)
        {
            pException->m_cause = CFileException::badPath;
            pException->m_strFileName = lpszFileIn;
        }
        return FALSE; // long path won't fit in buffer
    }

    CString strRoot;
    // determine the root name of the volume
    AfxGetRoot(lpszPathOut, strRoot);

    if (!::PathIsUNC( strRoot ))
    {
        // get file system information for the volume
        DWORD dwFlags, dwDummy;
        if (!GetVolumeInformation(strRoot, NULL, 0, NULL, &dwDummy, &dwFlags,
                                  NULL, 0))
        {
            TRACE(traceAppMsg, 0, _T("Warning: could not get volume information '%s'.\n"),
                  (LPCTSTR)strRoot);
            _AfxFillExceptionInfo(pException,lpszFileIn);
            return FALSE;   // preserving case may not be correct
        }

        // not all characters have complete uppercase/lowercase
        if (!(dwFlags & FS_CASE_IS_PRESERVED))
            CharUpper(lpszPathOut);

        // assume non-UNICODE file systems, use OEM character set
        if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
        {
            WIN32_FIND_DATA data;
            HANDLE h = FindFirstFile(lpszFileIn, &data);
            if (h != INVALID_HANDLE_VALUE)
            {
                FindClose(h);
                if(lpszFilePart != NULL && lpszFilePart > lpszPathOut)
                {
                    int nFileNameLen = lstrlen(data.cFileName);
                    int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
                    if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
                    {
                        Checked::tcscpy_s(lpszFilePart, _MAX_PATH - nIndexOfPart, data.cFileName);
                    }
                    else
                    {
                        // the path+filename of the file is too long
                        if (pException != NULL)
                        {
                            pException->m_cause = CFileException::badPath;
                            pException->m_strFileName = lpszFileIn;
                        }
                        return FALSE; // Path doesn't fit in the buffer.
                    }
                }
                else
                {
                    _AfxFillExceptionInfo(pException,lpszFileIn);
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}
//Backward compatible wrapper that calls the new exception throwing func.
BOOL AFXAPI AfxFullPath(__out_z LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
{
    return _AfxFullPath2(lpszPathOut, lpszFileIn,NULL);
}

void AFXAPI AfxGetRoot(LPCTSTR lpszPath, CString& strRoot)
{
    ASSERT(lpszPath != NULL);

    LPTSTR lpszRoot = strRoot.GetBuffer(_MAX_PATH);
    memset(lpszRoot, 0, _MAX_PATH);
    Checked::tcsncpy_s(lpszRoot, _MAX_PATH, lpszPath, _TRUNCATE);
    PathStripToRoot(lpszRoot);
    strRoot.ReleaseBuffer();
}

BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
{
#pragma warning(push)
#pragma warning(disable:4068)
#pragma prefast(push)
#pragma prefast(disable:400, "lstrcmpi is ok here as we are backing it up with further comparison")
    // use case insensitive compare as a starter
    if (lstrcmpi(lpszPath1, lpszPath2) != 0)
    {
        return FALSE;
    }
#pragma prefast(pop)
#pragma warning(pop)

    // on non-DBCS systems, we are done
    if (!GetSystemMetrics(SM_DBCSENABLED))
        return TRUE;

    // on DBCS systems, the file name may not actually be the same
    // in particular, the file system is case sensitive with respect to
    // "full width" roman characters.
    // (ie. fullwidth-R is different from fullwidth-r).
    int nLen = lstrlen(lpszPath1);
    if (nLen != lstrlen(lpszPath2))
        return FALSE;
    ASSERT(nLen < _MAX_PATH);

    // need to get both CT_CTYPE1 and CT_CTYPE3 for each filename
    LCID lcid = GetThreadLocale();
    WORD aCharType11[_MAX_PATH];
    VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath1, -1, aCharType11));
    WORD aCharType13[_MAX_PATH];
    VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath1, -1, aCharType13));
    WORD aCharType21[_MAX_PATH];
    VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath2, -1, aCharType21));
#ifdef _DEBUG
    WORD aCharType23[_MAX_PATH];
    VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath2, -1, aCharType23));
#endif

    // for every C3_FULLWIDTH character, make sure it has same C1 value
    int i = 0;
    for (LPCTSTR lpsz = lpszPath1; *lpsz != 0; lpsz = _tcsinc(lpsz))
    {
        // check for C3_FULLWIDTH characters only
        if (aCharType13[i] & C3_FULLWIDTH)
        {
#ifdef _DEBUG
            ASSERT(aCharType23[i] & C3_FULLWIDTH); // should always match!
#endif

            // if CT_CTYPE1 is different then file system considers these
            // file names different.
            if (aCharType11[i] != aCharType21[i])
                return FALSE;
        }
        ++i; // look at next character type
    }
    return TRUE; // otherwise file name is truly the same
}

UINT AFXAPI AfxGetFileTitle(LPCTSTR lpszPathName, __out_ecount(nMax) LPTSTR lpszTitle, UINT nMax)
{
    ASSERT(lpszTitle == NULL ||
           AfxIsValidAddress(lpszTitle, _MAX_FNAME));
    ASSERT(AfxIsValidString(lpszPathName));

    // use a temporary to avoid bugs in ::GetFileTitle when lpszTitle is NULL
    TCHAR szTemp[_MAX_PATH];
    LPTSTR lpszTemp = lpszTitle;
    if (lpszTemp == NULL)
    {
        lpszTemp = szTemp;
        nMax = _countof(szTemp);
    }
    if (::GetFileTitle(lpszPathName, lpszTemp, (WORD)nMax) != 0)
    {
        // when ::GetFileTitle fails, use cheap imitation
        return AfxGetFileName(lpszPathName, lpszTitle, nMax);
    }
    return lpszTitle == NULL ? lstrlen(lpszTemp)+1 : 0;
}

void AFXAPI AfxGetModuleShortFileName(HINSTANCE hInst, CString& strShortName)
{
    TCHAR szLongPathName[_MAX_PATH];
    ::GetModuleFileName(hInst, szLongPathName, _MAX_PATH);
    if (::GetShortPathName(szLongPathName,
                           strShortName.GetBuffer(_MAX_PATH), _MAX_PATH) == 0)
    {
        // rare failure case (especially on not-so-modern file systems)
        strShortName = szLongPathName;
    }
    strShortName.ReleaseBuffer();
}



/////////////////////////////////////////////////////////////////////////////
// CFile diagnostics

#ifdef _DEBUG
void CFile::AssertValid() const
{
    CObject::AssertValid();
    // we permit the descriptor m_hFile to be any value for derived classes
}

void CFile::Dump(CDumpContext& dc) const
{
    CObject::Dump(dc);

    dc << "with handle " << (void*)m_hFile;
    dc << " and name \"" << m_strFileName << "\"";
    dc << "\n";
}
#endif


IMPLEMENT_DYNAMIC(CFile, CObject)

/////////////////////////////////////////////////////////////////////////////

#endif
#endif

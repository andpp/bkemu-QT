// pch.cpp: файл исходного кода, соответствующий предварительно скомпилированному заголовочному файлу

#include "pch.h"
#include <QThread>
#include <time.h>


// При использовании предварительно скомпилированных заголовочных файлов необходим следующий файл исходного кода для выполнения сборки.

void Sleep(uint mSec) {
    QThread::msleep(mSec);
}

unsigned int GetTickCount()
{
    struct timespec ts;
    unsigned theTick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    theTick  = ts.tv_nsec / 1000000;
    theTick += ts.tv_sec * 1000;
    return theTick;
}

void splitpath(const CString &str,  CString &f_path, CString &f_name, CString &f_ext)
{

    int pos = str.lastIndexOf('/');

    if (-1 == pos) {
        f_path = "";
        f_name = str;
    } else {
        f_path = str.left(pos);
        f_name = str.right(str.size()-(pos+1));
    }

    // Extension
    int idx;

    idx = f_name.lastIndexOf('.');
    if(-1 == idx)
    {
        f_ext = "";
    }
    else
    {
        f_ext = f_name.right(f_name.size()-(idx+1));
        f_name = f_name.left(idx);
    }
}

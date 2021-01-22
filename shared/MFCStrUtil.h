#pragma once
#include "CString.h"
// эти функции только для проектов, написанных под MFC

// набор функций для манипуляции с именем файла
CString         GetCurrentPath();
CString        &NormalizePath(CString &strPath);
CString         GetFilePath(const CString &strFile);
CString         GetFileTitle(const CString &strFile);
CString         GetFileName(const CString &strFile);
CString         GetFileExt(const CString &strFile);


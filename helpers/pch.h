// pch.h: это предварительно скомпилированный заголовочный файл.
// Перечисленные ниже файлы компилируются только один раз, что ускоряет последующие сборки.
// Это также влияет на работу IntelliSense, включая многие функции просмотра и завершения кода.
// Однако изменение любого из приведенных здесь файлов между операциями сборки приведет к повторной компиляции всех(!) этих файлов.
// Не добавляйте сюда файлы, которые планируете часто изменять, так как в этом случае выигрыша в производительности не будет.

#ifndef PCH_H
#define PCH_H

#pragma once

// Добавьте сюда заголовочные файлы для предварительной компиляции
#include "framework.h"
#include "basetypes.h"
#include "res.h"
#include "CFile.h"
#include "CString.h"

inline bool IsWindowsVistaOrGreater() {return false;}

void Sleep(uint mSec);
unsigned int GetTickCount();
void splitpath(const CString &str,  CString &f_path, CString &f_name, CString &f_ext);

#endif //PCH_H

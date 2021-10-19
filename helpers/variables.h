#ifndef VERSIONVARS_H
#define VERSIONVARS_H

#define VERSION_MAJ 3
#define VERSION_MIN 11
#define VERSION_NUM 2101
#define REVISION_NUM 8570


#ifdef _WIN64
#define A_POSTFIX64 "_x64"
#define L_POSTFIX64 L"_x64"
#else
#define A_POSTFIX64 ""
#define L_POSTFIX64 L""
#endif

#ifdef _DEBUG
#define A_POSTFIXDEBUG " (Debug)"
#define L_POSTFIXDEBUG L" (Debug)"
#else
#define A_POSTFIXDEBUG ""
#define L_POSTFIXDEBUG L""
#endif

// Macros utils
#define RES_STRINGIZE1(a) #a
#define RES_STRINGIZE(a) RES_STRINGIZE1(a)
#define RES_LONGIZE1(a) L##a
#define RES_LONGIZE(a) RES_LONGIZE1(a)

// Auto definitions : for *_version.rc
#define A_VERSION_MAJ_STR  RES_STRINGIZE(VERSION_MAJ)
#define L_VERSION_MAJ_STR  RES_LONGIZE(A_VERSION_MAJ_STR)
#define A_VERSION_MIN_STR  RES_STRINGIZE(VERSION_MIN)
#define L_VERSION_MIN_STR  RES_LONGIZE(A_VERSION_MIN_STR)
#define A_VERSION_NUM_STR  RES_STRINGIZE(VERSION_NUM)
#define L_VERSION_NUM_STR  RES_LONGIZE(A_VERSION_NUM_STR)
#define A_REVISION_NUM_STR RES_STRINGIZE(REVISION_NUM)
#define L_REVISION_NUM_STR RES_LONGIZE(A_REVISION_NUM_STR)

// Text and Links
#define APPNAME L"BK Emulator for Windows"


// These variables are replaced by the contents of resources in the deployment project and assembly.
// In the future, everything will not be affected if they do not change.
#define FILE_INTNAME L"BK"
#define FILE_COMPANY_STR L"gid productions"
#define FILE_PRODUCT_STR L"BK Emulator for Windows"
#define FILE_COPYRIGHT_STR L"Copyright (с) 2003 Ю.Калмыков, Копировать правильно (c) 2012-2020 gid. Используйте на своё усмотрение."

// Section with Perdelko-Sopelko and Design glutches
//


#endif // VERSIONVARS_H


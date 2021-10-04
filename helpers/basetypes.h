#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <QChar>

#include <qnamespace.h>

typedef unsigned long long ULONGLONG;
typedef long LONG;
typedef unsigned int UINT;
typedef bool BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t * DWORD_PTR;

//typedef wchar_t TCHAR;
//#define _T(a) L##a
//typedef const wchar_t* LPCTSTR;
typedef char TCHAR;
typedef const char* LPCTSTR;
#define _T(a) a
typedef char* LPSTR;
typedef const char* LPCSTR;

typedef unsigned int UINT;

inline int max(int a, int b) { return a > b ? a : b; }
inline int min(int a, int b) { return a > b ? b : a; }

#define LOWORD(a) (a & 0xFFFF)
#define HIWORD(a) ((a >> 16) & 0xFFFF)
#define LOBYTE(a) (a & 0xFF)
#define HIBYTE(a) ((a >> 8) & 0xFF)

#define TRUE true
#define FALSE false

#define RGB(r,g,b) ( ((DWORD)(BYTE)r)|((DWORD)((BYTE)g)<<8)|((DWORD)((BYTE)b)<<16) )
#define GetRValue(RGBColor) (BYTE) (RGBColor)
#define GetGValue(RGBColor) (BYTE) (((DWORD)RGBColor) >> 8)
#define GetBValue(RGBColor) (BYTE) (((DWORD)RGBColor) >> 16)

typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, CRect;

typedef struct _CPoint {
    int x;
    int y;
} CPoint, *PCPoint;

typedef uint32_t *HWND;
typedef uint32_t *HMODULE;
typedef uint32_t *HICON;
typedef uint32_t *HBITMAP;
typedef uint32_t *CDC;

typedef uint32_t COLORREF;

typedef int32_t HRESULT;
typedef RECT WINDOWPLACEMENT;

#define register

#define WPARAM(a) a

enum: int {
    S_OK = 0,
};

#define SUCCEEDED(a) (a) == S_OK

#define WM_USER 1000

#define EMPTY_STRING "<empty>"

//enum: int {
//    IDS_INI_ADDR_DISASM = 0,
//    IDS_INI_ADDR_DUMP,
//    IDS_INI_ASK_FOR_BREAK,
//    IDS_INI_AUTO_BEG_TAPE,
//    IDS_INI_AUTO_END_TAPE,
//    IDS_INI_AY8910,
//    IDS_INI_AY8910_FILTHER,
//    IDS_INI_BK10_RE2_017_MONITOR,
//    IDS_INI_BK10_RE2_018_FOCAL,
//    IDS_INI_BK10_RE2_019_MSTD,
//    IDS_INI_BK10_RE2_106_BASIC1,
//    IDS_INI_BK10_RE2_107_BASIC2,
//    IDS_INI_BK10_RE2_108_BASIC3,
//    IDS_INI_BK11M_RE2_324_BOS,
//    IDS_INI_BK11M_RE2_325_EXT,
//    IDS_INI_BK11M_RE2_327_BASIC1,
//    IDS_INI_BK11M_RE2_328_BASIC2,
//    IDS_INI_BK11M_RE2_329_BASIC3,
//    IDS_INI_BK11M_RE2_330_MSTD,
//    IDS_INI_BK11_RE2_198_BASIC1,
//    IDS_INI_BK11_RE2_199_BASIC2,
//    IDS_INI_BK11_RE2_200_BASIC3,
//    IDS_INI_BK11_RE2_201_BOS,
//    IDS_INI_BK11_RE2_202_EXT,
//    IDS_INI_BK11_RE2_203_MSTD,
//    IDS_INI_BK11_RE_OPT_PG12_1,
//    IDS_INI_BK11_RE_OPT_PG12_2,
//    IDS_INI_BK11_RE_OPT_PG13_1,
//    IDS_INI_BK11_RE_OPT_PG13_2,
//    IDS_INI_BKKEYBOARD,
//    IDS_INI_BKMODEL,
//    IDS_INI_BLACK_WHITE,
//    IDS_INI_COLOR_MODE,
//    IDS_INI_COVOX,
//    IDS_INI_COVOX_FILTHER,
//    IDS_INI_COVOX_STEREO,
//    IDS_INI_CPU_FREQUENCY,
//    IDS_INI_CPU_RUN_ADDR,
//    IDS_INI_DRIVEA,
//    IDS_INI_DRIVEB,
//    IDS_INI_DRIVEC,
//    IDS_INI_DRIVED,
//    IDS_INI_EMULATE_FDDIO,
//    IDS_INI_EMUL_LOAD_TAPE,
//    IDS_INI_EMUL_SAVE_TAPE,
//    IDS_INI_FDR,
//    IDS_INI_FDR253,
//    IDS_INI_FDR_A16M,
//    IDS_INI_FDR_SAMARA,
//    IDS_INI_FDR_SMK512,
//    IDS_INI_FULL_SCREEN,
//    IDS_INI_HDD0,
//    IDS_INI_HDD1,
//    IDS_INI_ICLBLOCK,
//    IDS_INI_IMG_DIRECTORY,
//    IDS_INI_JOYSTICK,
//    IDS_INI_LONGBIN,
//    IDS_INI_LUMINOFOREMODE,
//    IDS_INI_MEM_DIRECTORY,
//    IDS_INI_OSCRENDER_TYPE,
//    IDS_INI_PAUSE_CPU,
//    IDS_INI_PROGRAM_DIRECTORY,
//    IDS_INI_REGSDUMP_INTERVAL,
//    IDS_INI_ROM_DIRECTORY,
//    IDS_INI_SAVES_DEFAULT,
//    IDS_INI_SAVES_DIRECTORY,
//    IDS_INI_SCRIPTS_DIRECTORY,
//    IDS_INI_SCRRENDER_TYPE,
//    IDS_INI_SECTIONNAME_DIRECTORIES,
//    IDS_INI_SECTIONNAME_ROMMODULES,
//    IDS_INI_SHOW_PERFORMANCE_STRING,
//    IDS_INI_SOUNDCHIPFREQ,
//    IDS_INI_SOUNDCHIPMODEL,
//    IDS_INI_SOUND_SAMPLE_RATE,
//    IDS_INI_SOUNDVOLUME,
//    IDS_INI_SPEAKER,
//    IDS_INI_SPEAKER_FILTHER,
//    IDS_INI_SSHOT_DIRECTORY,
//    IDS_INI_SSHOT_NUM,
//    IDS_INI_TAPES_DIRECTORY,
//    IDS_INI_TOOLS_DIRECTORY,
//    IDS_INI_VKBD_TYPE,
//};

#define ASSERT(a) if(!(a)) { printf("ASSERT %s:%d\n", __FILE__, __LINE__); }
//#define CBKMessageBox(a,b) printf(a)

typedef struct {
  WORD  wFormatTag;
  WORD  nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD  nBlockAlign;
  WORD  wBitsPerSample;
  WORD  cbSize;
} WAVEFORMATEX;

#define WAVE_FORMAT_PCM 1

#define ZeroMemory(a,size) memset(a,0,size)

#ifdef DEBUG
#define TRACE  printf
#define TRACE0 printf
#define TRACE1 printf
#define TRACE2 printf
#define TRACE3 printf
#else
#define TRACE  (void)
#define TRACE0 (void)
#define TRACE1 (void)
#define TRACE2 (void)
#define TRACE3 (void)
#endif

#define VK_UP     Qt::Key_Up
#define VK_RIGHT  Qt::Key_Right
#define VK_DOWN   Qt::Key_Down
#define VK_LEFT   Qt::Key_Left
#define VK_HOME   Qt::Key_Home
#define VK_PRIOR  Qt::Key_PageUp
#define VK_END    Qt::Key_End
#define VK_NEXT   Qt::Key_PageDown

#define VK_INSERT Qt::Key_Insert
#define VK_DELETE Qt::Key_Delete
#define VK_TAB    Qt::Key_Tab

#define VK_RWIN   (Qt::Key_Control| 0x10000)
#define VK_LWIN   (Qt::Key_Control| 0x20000)
#define VK_ESCAPE Qt::Key_Escape
#define VK_RETURN Qt::Key_Return
#define VK_BACK   Qt::Key_Backspace
#define VK_F1     Qt::Key_F1
#define VK_F2     Qt::Key_F2
#define VK_F3     Qt::Key_F3
#define VK_F4     Qt::Key_F4
#define VK_F5     Qt::Key_F5
#define VK_F6     Qt::Key_F6
#define VK_F7     Qt::Key_F7
#define VK_F8     Qt::Key_F8
#define VK_F9     Qt::Key_F9
#define VK_F10    Qt::Key_F10
#define VK_F11    Qt::Key_F11
#define VK_F12    Qt::Key_F12
#define VK_SPACE  Qt::Key_Space



#define MAXINT32 0x7FFFFFFF

#endif // BASE_TYPES_H

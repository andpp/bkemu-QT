#pragma once
#include "pch.h"

constexpr auto INI_BUFFER_LENGTH = 1024;

class CIni
{
	public:
		enum class IniStatus { NOTREADED = 0, READED, FLUSHED };
		enum class IniError { OK_NOERROR = 0, READ_ERROR, WRITE_ERROR, PARSE_ERROR };
		struct INI_KEY_STRUCT
		{
			DWORD Hash;
			CString Key;
			CString Value;
			CString Comment;
			INI_KEY_STRUCT *next;
			INI_KEY_STRUCT() : Hash(0), next(nullptr) {}
		};

		struct INI_SECTION_STRUCT
		{
			DWORD Hash;
			CString Name;
			INI_KEY_STRUCT *Keys;
			INI_SECTION_STRUCT *next;
			INI_SECTION_STRUCT() : Hash(0), Keys(nullptr), next(nullptr) {}
		};

	protected:
		CString m_strFileName; // Имя ини файла
		IniStatus m_iniStatus;
		IniError m_iniError;
		int m_LineNum;
		int m_Encoding; // 0 - utf8, 1 - ansi -- сейчас не используется. Только АНСИ

		INI_SECTION_STRUCT *SetOfSections;

		bool ReadIni();
		DWORD GetHash(const CString &str);
		bool Add(const CString &strSection, const CString &strKey, const CString &strVal, const CString &strComm, bool bChangeComment = false);

		LPSTR ConvertUnicodetoStr(UINT type, const CString &ustr); // type = CP_ACP - ansi; CP_UTF8 - utf8
		CString ConvertStrtoUnicode(UINT type, LPCSTR astr); // type = CP_ACP - ansi; CP_UTF8 - utf8

		bool _intGetValueString(const CString &strSection, const CString &strKey, CString &strValue);

		bool DeleteKey(const CString &strSection, const CString &strKey);

	public:
		CIni();
		virtual ~CIni();

		inline void SetIniFileName(const CString &strFileName)
		{
			m_strFileName = strFileName;
		}
		inline const CString &GetIniFileName()
		{
			return m_strFileName;
		}
		inline IniStatus GetIniStatus()
		{
			return m_iniStatus;
		}
		inline IniError GetIniError()
		{
			return m_iniError;
		}
		inline void SetEncoding(int e)
		{
			m_Encoding = e;
		}
		inline int GetEncoding()
		{
			return m_Encoding;
		}

		void    Clear();
		bool    FlushIni();

		ULONGLONG FlushIniToMemory(uint8_t *pBuff, UINT nSize);
		bool    ReadIniFromMemory(uint8_t *pBuff, UINT nSize);

		CString GetValueString(int nSection, const CString &strKey, const CString &strDefault);
		CString GetValueString(int nSection, int nKey, const CString &strDefault);
		int     GetValueInt(int nSection, int nKey, const int nDefault);
		double  GetValueFloat(int nSection, int nKey, const double fDefault);
		bool    GetValueBool(int nSection, int nKey, const bool bDefault);

		bool    SetValueString(int nSection, const CString &strKey, const CString &strVal);
		bool    SetValueString(int nSection, int nKey, const CString &strVal);
		bool    SetValueInt(int nSection, int nKey, const int iVal);
		bool    SetValueFloat(int nSection, int nKey, const double fVal);
		bool    SetValueBool(int nSection, int nKey, const bool bVal);

		CString GetValueStringEx(const CString &strCustomName, int nSection, int nKey, const CString &strDefault);
		int     GetValueIntEx(const CString &strCustomName, int nSection, int nKey, const int nDefault);
		double  GetValueFloatEx(const CString &strCustomName, int nSection, int nKey, const double fDefault);
		bool    GetValueBoolEx(const CString &strCustomName, int nSection, int nKey, const bool bDefault);

		bool    SetValueStringEx(const CString &strCustomName, int nSection, int nKey, const CString &strVal, bool bForce = false);
		bool    SetValueIntEx(const CString &strCustomName, int nSection, int nKey, const int iVal, bool bForce = false);
		bool    SetValueFloatEx(const CString &strCustomName, int nSection, int nKey, const double fVal, bool bForce = false);
		bool    SetValueBoolEx(const CString &strCustomName, int nSection, int nKey, const bool bVal, bool bForce = false);

		// копирование секции из другого ини файла в этот
		bool    CopySection(CIni *pSrcIni, const CString &strSectionName);

// ----------------------------------------------------------------------
		// работа с секциями

		// получение указателя на секцию.
		// если такой секции нет - nullptr
		INI_SECTION_STRUCT *GetSectionPtr(int nSection);
};


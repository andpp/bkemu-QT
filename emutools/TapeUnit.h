// TapeUnit.h: interface for the CTapeUnit class.
//


#pragma once


class CTapeUnit
{
		enum : int      {UNKNOWN = -1};

		int             m_nType;
		int             m_nAddress;
		int             m_nLength;
		int             m_nMSTime;
		DWORD           m_nWaveLength;
		bool            m_bCRC;
		CString         m_strName;
		CString         m_strPath;

		bool            IsFileWave(CString &strPath);
		bool            IsFileMSF(CString &strPath);
		bool            IsFileBin(CString &strPath);

	public:
		enum : int      {TYPE_NONE = -1, TYPE_BIN = 0, TYPE_WAV, TYPE_MSF, TYPE_TMP};

		CTapeUnit();
		CTapeUnit(CTapeUnit &tapeUnit);
		virtual ~CTapeUnit();

		CTapeUnit       &operator= (CTapeUnit &tapeUnit);

		bool            SetFile(CString &strPath);
		bool            SetTmpFile(CString &strPath);

		inline int      GetType()
		{
			return m_nType;
		}
		inline int      GetAddress()
		{
			return m_nAddress;
		}
		inline int      GetLength()
		{
			return m_nLength;
		}
		inline int      GetTime()
		{
			return m_nMSTime;
		}
		inline DWORD    GetWaveLength()
		{
			return m_nWaveLength;
		}
		inline bool     GetCRC()
		{
			return m_bCRC;
		}
		inline const CString  GetName()
		{
			return m_strName;
		}
		inline const CString GetPath()
		{
			return m_strPath;
		}

		bool            RetrieveInfo();
		bool            SaveAs(CString &strPath, int type);
};


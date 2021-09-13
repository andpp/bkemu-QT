// ScriptRunner.h: interface for the CScriptRunner class.
//

#pragma once
#include "pch.h"

class CMotherBoard;

class CScriptRunner
{
		CMotherBoard       *m_pBoard;

		int                 m_nScriptLinePos;
		int                 m_nScriptLineLen;
		DWORD               m_nScriptCurrTick;
		CStdioFile          m_fileScript;
		CString             m_strScriptsPath;
		CString             m_strScriptFileName;
		CString             m_strScriptLine;
		CString             m_strArgument;
		int                 m_nTickDelay;
		bool                m_bStopButton;
		bool                m_bRus;
		bool                m_bIsAr2Press; // флаг нажатия АР2 (не реализован, т.к.  непонятно, на какой символ вешать)
		bool                m_bHasScript;
        bool                CheckEscChar(int ch);
        void                ParseNextChar(int ch);
		void                SendNewCharToBK(uint8_t ch);
	public:
		CScriptRunner();
		virtual ~CScriptRunner();

		void                AttachBoard(CMotherBoard *pBoard)
		{
			m_pBoard = pBoard;
		}

		void                SetScriptPath(const CString &strScriptPath, const CString &strScriptFileName, bool bXlat);
		void                SetArgument(const CString &strArg);
		bool                RunScript();
		void                StopScript();
};

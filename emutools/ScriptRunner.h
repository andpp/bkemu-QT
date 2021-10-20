// ScriptRunner.h: interface for the CScriptRunner class.
//

#pragma once
#include "pch.h"
#include <QList>

class CMotherBoard;

using ScriptArgList = QList<CString>;

class CScriptRunner
{
		CMotherBoard       *m_pBoard;

		int                 m_nScriptLinePos;
		int                 m_nScriptLineLen;
		DWORD               m_nScriptCurrTick;
		CStdioFile          m_fileScript;
        CString             m_strScriptFile;
		CString             m_strScriptLine;
		int                 m_nTickDelay;
		bool                m_bStopButton;
		bool                m_bRus;
		bool                m_bIsAr2Press; // флаг нажатия АР2 (не реализован, т.к.  непонятно, на какой символ вешать)
		bool                m_bHasScript;

        ScriptArgList       m_listArgs;

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

        void                SetScript(const CString &strScriptPath, const CString &strScriptFileName, bool bXlat);
        void                StopScript();
        void                SetArgumentList(ScriptArgList &ArgList);
        void                SetArgument(const CString &strArg);
        bool                RunScript();
};

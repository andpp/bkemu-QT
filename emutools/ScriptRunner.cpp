// ScriptRunner.cpp: implementation of the CScriptRunner class.
//


#include "pch.h"
#include "ScriptRunner.h"
#include "Board.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// замедлитель. чтобы передаваемые символы не терялись
constexpr auto TICK_DELAY = 5;
// задержка начала выполнения скрипта в секундах
constexpr auto SCRIPT_START_DELAY = 2;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CScriptRunner::CScriptRunner()
	: m_pBoard(nullptr)
	, m_nScriptLinePos(0)
	, m_nScriptLineLen(0)
	, m_nScriptCurrTick(0)
	, m_nTickDelay(TICK_DELAY)
	, m_bStopButton(false)
	, m_bRus(false)
	, m_bIsAr2Press(false)
	, m_bHasScript(false)
{
}

CScriptRunner::~CScriptRunner()
{
}


void CScriptRunner::SetScriptPath(const CString &strScriptPath, const CString &strScriptFileName, bool bXlat)
{
	if (m_fileScript.m_pStream)
	{
		m_fileScript.Close();
	}

	m_strScriptsPath = strScriptPath;
	m_strScriptFileName = strScriptFileName;
	m_bHasScript = !strScriptFileName.IsEmpty();
	m_bRus = bXlat;
}

void CScriptRunner::SetArgument(const CString &strArg)
{
	m_strArgument = strArg;
}


bool CScriptRunner::RunScript()
{
	ASSERT(m_pBoard);

	if (!m_bHasScript)
	{
		return false;    // Если скрипт не задан, выйдем с ошибкой
	}

	if (!m_pBoard) // Если нету эмулируемого устройства, выйдем с ошибкой
	{
		return false;
	}

	if (!m_fileScript.m_pStream) // если файл ещё не открыт, попробуем открыть
	{
		CString ofn = m_strScriptsPath + m_strScriptFileName;

		// если файл не открыт, откроем его
		if (!m_fileScript.Open(ofn, CFile::modeRead))
		{
			// Если файл скрипта не открывается, то реинициализируем обработчик
			// и выйдем с ошибкой
			m_strScriptFileName.Empty();
			m_bHasScript = false;
			return false;
		}

		// Если скрипт открылся, инициализируем переменные
		m_strScriptLine.Empty();
		m_nScriptLinePos = m_nScriptLineLen = 0;
		m_nScriptCurrTick = GetTickCount(); // Запускаем таймер
	}

	m_pBoard->m_reg177716in |= 0100;    // снимем флаг нажатия клавиши

	// Если скрипт открыт и всё в порядке
	// Ждём определённое время перед запуском скрипта
	if ((GetTickCount() - m_nScriptCurrTick) < SCRIPT_START_DELAY * 1000)
	{
		return true;
	}

	if (m_bStopButton)
	{
		m_bStopButton = false;
		m_pBoard->UnStopInterrupt();
	}

	if (m_nScriptLinePos < m_nScriptLineLen) // Если есть необработанные символы в строке
	{
		if (--m_nTickDelay <= 0)
		{
			m_nTickDelay = TICK_DELAY;

			if (CheckEscChar(m_strScriptLine[m_nScriptLinePos]))
			{
				return true;
			}

			// Передадим новый символ только если регистр БК 177660 готов
			// т.е. ждём, пока БК прочтёт следующий код из 177662
			if (!(m_pBoard->m_reg177660 & 0200))
			{
				// TRACE1("script send char: %c\n", m_strScriptLine[m_nScriptLinePos]);
				ParseNextChar(m_strScriptLine[m_nScriptLinePos++]);
			}
		}
	}
	else
	{
		// Если дошли до конца текущей строки,
		m_nScriptLinePos = 0;

		// прочитаем следующую
		if (!m_fileScript.ReadString(m_strScriptLine))
		{
			// Если строки закончились,
			// закроем файл и переинициализируем переменные
			SetScriptPath(_T(""), _T(""), false);
			return false;
		}

		m_strScriptLine += _T('\n'); // добавим перевод каретки в конец строки
		m_nScriptLineLen = m_strScriptLine.GetLength();
	}

	return true;
}

void CScriptRunner::StopScript()
{
	SetScriptPath(_T(""), _T(""), false);
}

// Проверка управляющих символов
// выход: true - управляющий символ обработан, и нужно выйти из обработчика
// false - нужно продолжить
bool CScriptRunner::CheckEscChar(TCHAR ch)
{
	if (ch == _T('|'))
	{
		m_nScriptLinePos++; // пропускаем управляющий символ
		ch = m_strScriptLine[m_nScriptLinePos++]; // смотрим следующий

		if (ch == _T('|'))
		{
			// это значит, что надо передать знак управляющего символа в качестве обычного символа
			m_nScriptLinePos--;
			return false;
		}

		if (ch == _T('#')) // это снова надо установить задержку
		{
			m_nScriptCurrTick = GetTickCount();
			return true;
		}

		if (ch == _T('^')) // эмуляция кнопки СТОП
		{
			m_bStopButton = true;
			m_pBoard->StopInterrupt();
			return true;
		}

		if (ch == _T('@')) // обработка аргумента
		{
			m_strScriptLine.Insert(m_nScriptLinePos, m_strArgument);
			m_nScriptLineLen = m_strScriptLine.GetLength();
			return false;
		}
	}

	return false;
}

void CScriptRunner::ParseNextChar(TCHAR ch)
{
	uint8_t koi_ch = WIDEtoBKChar(ch);

	if (koi_ch >= 0300 && !m_bRus) // если буквы русские, а режим - не русский
	{
		m_nScriptLinePos--; // откатим позицию
		koi_ch = BKKEY_RUS; // пошлём код переключения языка
		m_bRus = true; // включаем русский режим
	}
	else if ((0100 <= koi_ch && koi_ch <= 0177) && m_bRus) // если буквы не русские, а режим - русский
	{
		m_nScriptLinePos--; // откатим позицию
		koi_ch = BKKEY_LAT; // пошлём код переключения языка
		m_bRus = false; // выключаем русский режим
	}

	SendNewCharToBK(koi_ch);
}


void CScriptRunner::SendNewCharToBK(uint8_t ch)
{
	m_pBoard->m_reg177662in = ch & 0177; // Отправим код символа в 177662
	m_pBoard->m_reg177660 |= 0200;       // Установим состояние готовности в 177660
	m_pBoard->m_reg177716in &= ~0100;    // Установим флаг нажатия клавиши в 177716
	m_pBoard->KeyboardInterrupt(m_bIsAr2Press ? INTERRUPT_274 : INTERRUPT_60);
}

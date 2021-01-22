// Debugger.cpp: implementation of the CDebugger class.
//


#include "pch.h"
#include "Debugger.h"
#include "Config.h"
#include "Board.h"
#include "BKMessageBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

int CDebugger::m_outLevel = 0;

#define RGB(r,g,b) ( ((DWORD)(BYTE)r)|((DWORD)((BYTE)g)<<8)|((DWORD)((BYTE)b)<<16) )
#define GetRValue(RGBColor) (BYTE) (RGBColor)
#define GetGValue(RGBColor) (BYTE) (((DWORD)RGBColor) >> 8)
#define GetBValue(RGBColor) (BYTE) (((DWORD)RGBColor) >> 16)


constexpr auto SRC = 2;
constexpr auto DST = 0;

const COLORREF g_crDebugColorHighLighting[] =
{
	RGB(0, 0, 0), // HLCOLOR_DEFAULT
	RGB(0x66, 0, 0), // HLCOLOR_ADDRESS
	RGB(0, 0, 0xcc), // HLCOLOR_MNEMONIC
	RGB(0, 0x66, 0xcc), // HLCOLOR_REGISTER
	RGB(0xff, 0x66, 0), // HLCOLOR_NUMBER
	RGB(0x33, 0x33, 0x33) // HLCOLOR_SYMBOL
};


const CString CDebugger::m_strRegNames[8] =
{
	_T(COLORED_TAG)_T("3R0"),
	_T(COLORED_TAG)_T("3R1"),
	_T(COLORED_TAG)_T("3R2"),
	_T(COLORED_TAG)_T("3R3"),
	_T(COLORED_TAG)_T("3R4"),
	_T(COLORED_TAG)_T("3R5"),
	_T(COLORED_TAG)_T("3SP"),
	_T(COLORED_TAG)_T("3PC")
};
// Формат отображения режимов адресации
const CString CDebugger::m_strAddrFormat[8] =
{
	_T("%s"),
	_T(COLORED_TAG)_T("5(%s")_T(COLORED_TAG)_T("5)"),
	_T(COLORED_TAG)_T("5(%s")_T(COLORED_TAG)_T("5)+"),
	_T(COLORED_TAG)_T("5@(%s")_T(COLORED_TAG)_T("5)+"),
	_T(COLORED_TAG)_T("5-(%s")_T(COLORED_TAG)_T("5)"),
	_T(COLORED_TAG)_T("5@-(%s")_T(COLORED_TAG)_T("5)"),
	_T(COLORED_TAG)_T("4%o")_T(COLORED_TAG)_T("5(%s")_T(COLORED_TAG)_T("5)"),
	_T(COLORED_TAG)_T("5@")_T(COLORED_TAG)_T("4%o")_T(COLORED_TAG)_T("5(%s")_T(COLORED_TAG)_T("5)")
};
// Формат отображения режимов адресации, если регистр PC
const CString CDebugger::m_strAddrFormat_PC[8] =
{
	_T(COLORED_TAG)_T("3PC"),
	_T(COLORED_TAG)_T("5(")_T(COLORED_TAG)_T("3PC")_T(COLORED_TAG)_T("5)"),
	_T(COLORED_TAG)_T("5#")_T(COLORED_TAG)_T("4%o"),
	_T(COLORED_TAG)_T("5@#")_T(COLORED_TAG)_T("1%o"),
	_T(COLORED_TAG)_T("5-(")_T(COLORED_TAG)_T("3PC")_T(COLORED_TAG)_T("5)"),
	_T(COLORED_TAG)_T("5@-(")_T(COLORED_TAG)_T("3PC")_T(COLORED_TAG)_T("5)"),
	_T(COLORED_TAG)_T("1%o"),
	_T(COLORED_TAG)_T("5@")_T(COLORED_TAG)_T("1%o")
};
// Формат отображения аргумента - адрес
const CString CDebugger::m_strArgFormat_Addr = _T(COLORED_TAG)_T("1%06o");
// Формат отображения аргумента - число
const CString CDebugger::m_strArgFormat_Number = _T(COLORED_TAG)_T("4%o");
// запятая между аргументом 1 и 2
const CString CDebugger::m_strArgFormat_Comma = _T(COLORED_TAG)_T("5,");
//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CDebugger::CDebugger()
	: m_pBoard(nullptr)
	, m_pDisasmDlg(nullptr)
	, m_pInstrRefsMap(nullptr)
	, m_wTopAddress(g_Config.m_nAdrAsm)
	, m_bPrevCmdC(false)
	, m_wPC(0)
	, m_wInstr(0)
	, m_bCBug(false)
	, m_wFreg(0)
{
//	m_hBPIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DBG_BPT));
//	m_hCurrIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DBG_CUR));
	InitMaps();
}

CDebugger::~CDebugger()
{
	g_Config.m_nAdrAsm = m_wTopAddress;

//	if (m_hBPIcon)
//	{
//		DestroyIcon(m_hBPIcon);
//	}

//	if (m_hCurrIcon)
//	{
//		DestroyIcon(m_hCurrIcon);
//	}

	SAFE_DELETE_ARRAY(m_pInstrRefsMap);
}

void CDebugger::AttachBoard(CMotherBoard *pBoard)
{
	m_pBoard = pBoard;

	if (m_pBoard)
	{
		m_pBoard->AttachDebugger(this);
	}
}

void CDebugger::SetCurrentAddress(uint16_t address)
{
	m_wTopAddress = address;
//	m_pDisasmDlg->PostMessage(WM_DBG_CURRENT_ADDRESS_CHANGE, WPARAM(address));  // обновим адрес в поле адреса.
//	m_pDisasmDlg->GetDisasmCtrl()->Invalidate(FALSE); // перерисуем дизассемблер
}

// поиск в списке точек останова, заданной точки останова
bool CDebugger::IsBpeakpointExists(CBreakPoint &breakpoint)
{
    if (m_breakpointList.size())
    {
        int pos = 0;

        while (pos < m_breakpointList.size())
		{
            CBreakPoint &curr = m_breakpointList[pos++];

			if ((curr.GetType() == breakpoint.GetType())
			        && (curr.GetAddress() == breakpoint.GetAddress()))
			{
				return true;
			}
		}
	}

	return false;
}


// поиск в списке точек останова, точки с заданным адресом
bool CDebugger::GetDebugPCBreak(uint16_t addr)
{
	// оставим это излишество на случай, если решим алгоритмы поменять
	return IsBpeakpointAtAddress(addr);
}

// поиск в списке точек останова, точки с заданным адресом
bool CDebugger::IsBpeakpointAtAddress(uint16_t addr)
{
    if (!m_breakpointList.empty())
	{
        int pos = 0;

        while (pos < m_breakpointList.size())
		{
            CBreakPoint &curr = m_breakpointList[pos++];

			if (curr.IsAddress() && curr.GetAddress() == addr)
			{
				return true;
			}
		}
	}

	return false;
}


bool CDebugger::SetSimpleBreakpoint(uint16_t addr)
{
	CBreakPoint breakpoint(addr);

	if (IsBpeakpointExists(breakpoint))
	{
		return false;
	}

    m_breakpointList.append(breakpoint);
	return true;
}


bool CDebugger::SetSimpleBreakpoint()
{
	return SetSimpleBreakpoint(GetCursorAddress());
}


bool CDebugger::RemoveBreakpoint(uint16_t addr)
{
    int pos = 0;
    while (pos < m_breakpointList.size())
	{
        CBreakPoint &curr = m_breakpointList[pos++];

		if (curr.IsAddress() && curr.GetAddress() == addr)
		{
            m_breakpointList.removeAt(--pos);
			return true;
		}
	}

	return false;
}


bool CDebugger::RemoveBreakpoint()
{
	return RemoveBreakpoint(GetCursorAddress());
}


void CDebugger::ClearBreakpointList()
{
    m_breakpointList.clear();
}

void CDebugger::DrawDebuggerLine(int nNum, CDC *pDC, CRect *pRcSubs)
{
	if (!m_pBoard) // Нет чипа - нечего рисовать
	{
		return;
	}

	// Дизассемблируем машинный код
	uint16_t instrOpcode[8];
	CString strInstruction;
	CString strTxt;
	register uint16_t wLineAddr = GetLineAddress(nNum);
	register int len = DebugInstruction(wLineAddr, strInstruction, instrOpcode);

	// Выводим маркер
	if (IsBpeakpointAtAddress(wLineAddr))
	{
//		::DrawIconEx(pDC->m_hDC, pRcSubs[DISASM_LIST::COL_MARK].left, pRcSubs[DISASM_LIST::COL_MARK].top, m_hBPIcon, 16, 16, 0, nullptr, DI_NORMAL);
	}

	if (m_pBoard->IsCPUBreaked() && wLineAddr == m_pBoard->GetRON(CCPU::R_PC))
	{
//		::DrawIconEx(pDC->m_hDC, pRcSubs[DISASM_LIST::COL_MARK].left, pRcSubs[DISASM_LIST::COL_MARK].top, m_hCurrIcon, 16, 16, 0, nullptr, DI_NORMAL);
	}

	// Выводим адрес
	::WordToOctString(wLineAddr, strTxt);
//	pDC->SetTextColor(g_crDebugColorHighLighting[HLCOLOR_ADDRESS]);
//	pDC->DrawText(strTxt, &pRcSubs[DISASM_LIST::COL_ADDR], DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
	// Выводим инструкцию
//	pDC->SetTextColor(g_crDebugColorHighLighting[HLCOLOR_MNEMONIC]);
//	DrawColoredText(pDC, pRcSubs[DISASM_LIST::COL_INSTR], strInstruction);
	// Выводим комментарий. Это у нас просто машинные инструкции ассемблерной команды
	::WordToOctString(instrOpcode[0], strTxt); // код инструкции у нас по любому всегда есть

	// а дальше от 0 до 2-х слов аргументов
	// для инструкций FIS если регистр PC, то 4 слова аргументов
	for (int i = 1; i < len; ++i)
	{
		strTxt += _T(' ') + ::WordToOctString(instrOpcode[i]);
	}

//	pDC->SetTextColor(g_crDebugColorHighLighting[HLCOLOR_DEFAULT]);
//	pDC->DrawText(strTxt, &pRcSubs[DISASM_LIST::COL_COMMENT], DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
}

void CDebugger::DrawColoredText(CDC *pDC, CRect &rect, CString &str)
{
	bool bEnd = false;

	if (!str.IsEmpty())
	{
		// новый алгоритм. тут возможно начало строки без тега
		// и вообще можно вывести строку без тега
		register int beginpos = 0; // начальная позиция текста
		register int endpos = str.GetLength(); // конечная позиция текста

		while (true)
		{
			// ищем тег
			register int tagpos = str.Find(_T(COLORED_TAG), beginpos);

			if (tagpos == -1) // если не нашли
			{
				tagpos = endpos;
				bEnd = true;
			}

			if (beginpos < tagpos) // если перед тегом есть какой-то текст
			{
				CString substr = str.Mid(beginpos, tagpos - beginpos);
//				CSize size = pDC->GetTextExtent(substr); // текущая позиция вывода текста
//				pDC->DrawText(substr, &rect, DT_LEFT | DT_VCENTER);
//				rect.left += size.cx;

				if (rect.left >= rect.right)
				{
					bEnd = true;
				}
			}

			if (bEnd) // если тегов больше нет
			{
				break; // не надо их искать, выходим
			}

			register int clpos = tagpos + COLORED_TAG_LENGTH;

			if (clpos < endpos)
			{
				register int colornum = (str.GetAt(clpos) - _T('0')) & 7; // номер цвета

				if (colornum > HLCOLOR_NUM_COLS)
				{
					colornum = HLCOLOR_NUM_COLS;
				}

//				pDC->SetTextColor(g_crDebugColorHighLighting[colornum]); // устанавливаем цвет
				beginpos = clpos + 1; // пойдём искать новый тег
			}
			else
			{
				break; // строка оборвана
			}
		}
	}
}


void CDebugger::StepForward()
{
	m_wTopAddress += CalcInstructionLength(m_pBoard->GetWordIndirect(m_wTopAddress));
}


void CDebugger::StepBackward()
{
	m_wTopAddress -= sizeof(uint16_t);
}


uint16_t CDebugger::GetLineAddress(int nNum)
{
	if (nNum < 0)
	{
		return 0;
	}

	uint16_t wCurAddr = m_wTopAddress;

	while (nNum--)
	{
		wCurAddr += CalcInstructionLength(m_pBoard->GetWordIndirect(wCurAddr));
	}

	return wCurAddr;
}


int CDebugger::CalcInstructionLength(uint16_t instr)
{
	m_wInstr = instr; // фиксируем опкод инструкции, длину которой подсчитываем
	/* много скобок, но так надо. такой хитрый синтаксис */
	return (this->*(m_pInstrRefsMap[instr].InstrLenRef))();
}


// Проверяем на подходящую инструкцию, если задано условие отладки "шаг с выходом"
bool CDebugger::IsInstructionOut(uint16_t instruction)
{
	switch (instruction)
	{
		case PI_BPT:
		case PI_IOT:
			m_outLevel++; // Считаем заходы в прерывания по векторам 14 и 20
			return false;
	}

	switch (instruction & ~0377)
	{
		case PI_EMT:
		case PI_TRAP:
			m_outLevel++; // Считаем заходы в прерывания по векторам 30 и 34
			return false;
	}

	switch (instruction & ~0777)
	{
		case PI_JSR:
			m_outLevel++; // считаем заходы в подпрограммы
			return false;
	}

	// заходы считаем, чтобы не останавливаться на выходе из вложенных прерываний
	// а остановиться на выходе именно из той функции/прерывания, в которой сейчас находимся.

	if (instruction == PI_RTI || instruction == PI_RTT || (instruction & ~7) == PI_RTS)
	{
		if (--m_outLevel == -1) // Считаем выходы из прерываний/подпрограмм.
		{
			m_outLevel = 0; // если выход из прерывания/подпрограммы на один больше, чем заход,
			return true;        // то мы вышли из функции и надо сделать останов
		}
	}

	// смешиваем подпрограммы и прерывания потому, что бывает выход из прерывания по RTS PC.
	// в случаях, когда нужно усложнить понимание и код программы
	return false;
}

// Проверяем на подходящую инструкцию, если задано условие "шаг с обходом"
// т.е. не заходить в подпрограмму, а выполнить её как одну команду.
// Если при выполнении адрес возврата будет изменён, то управление будет потеряно.
bool CDebugger::IsInstructionOver(uint16_t instruction)
{
	switch (instruction)
	{
		case PI_BPT:
		case PI_IOT:
			return true;    // Внутрь BPT, IOT не заходим
	}

	switch (instruction & ~0377)
	{
		case PI_EMT:
		case PI_TRAP:
			return true;    // Внутрь EMT, TRAP не заходим
	}

	switch (instruction & ~0777)
	{
		case PI_JSR:
		case PI_SOB:
			return true;    //// Внутрь JSR не заходим, циклы SOB пошагово не выполняем.
	}

	return false;
}


uint16_t CDebugger::GetCursorAddress()
{
	ASSERT(m_pDisasmDlg);
//	register int nLine = m_pDisasmDlg->GetDisasmCtrl()->GetSelectionMark();
    register int nLine = 0;
    return GetLineAddress(nLine);
}


uint16_t CDebugger::GetBottomAddress()
{
	ASSERT(m_pDisasmDlg);
//	register int nLast = m_pDisasmDlg->GetDisasmCtrl()->GetItemCount();
    register int nLast = 0;
    return GetLineAddress(nLast);
}


uint16_t CDebugger::GetRegister(int reg)
{
	ASSERT(m_pBoard);

	if (m_pBoard)
	{
		return m_pBoard->GetRON(reg);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// используется для получения значений портов и только
uint16_t CDebugger::GetPortValue(int addr)
{
	ASSERT(m_pBoard);

	if (m_pBoard)
	{
		switch (addr)
		{
			case SYS_PORT_177660:
				return m_pBoard->m_reg177660;

			case SYS_PORT_177662_IN:
				return m_pBoard->m_reg177662in;

			case SYS_PORT_177662_OUT:
				return m_pBoard->m_reg177662out;

			case SYS_PORT_177664:
				return m_pBoard->m_reg177664;

			case SYS_PORT_177700:
				return m_pBoard->GetWordIndirect(0177700);

			case SYS_PORT_177702:
				return m_pBoard->GetWordIndirect(0177702);

			case SYS_PORT_177704:
				return m_pBoard->GetWordIndirect(0177704);

			case SYS_PORT_177706:
				return m_pBoard->GetWordIndirect(0177706);

			case SYS_PORT_177710:
				return m_pBoard->GetWordIndirect(0177710);

			case SYS_PORT_177712:
				return m_pBoard->GetWordIndirect(0177712);

			case SYS_PORT_177714_IN:
				return m_pBoard->m_reg177714in;

			case SYS_PORT_177714_OUT:
				return m_pBoard->m_reg177714out;

			case SYS_PORT_177716_IN:
				return m_pBoard->m_reg177716in;

			case SYS_PORT_177716_OUT_TAP:
				return m_pBoard->m_reg177716out_tap;

			case SYS_PORT_177716_OUT_MEM:
				return m_pBoard->m_reg177716out_mem;
		}

		ASSERT(false);
	}

	return 0;
}
// получение состояния режимов контроллера АльтПро
uint16_t CDebugger::GetAltProData(int reg)
{
	ASSERT(m_pBoard);

	if (m_pBoard)
	{
		switch (reg)
		{
			case 0:
				return m_pBoard->GetAltProMode();

			case 1:
				return m_pBoard->GetAltProCode();
		}

		ASSERT(false);
	}

	return 0;
}
// получение состояния режимов контроллера FDD
uint16_t CDebugger::GetFDDData(int reg)
{
	ASSERT(m_pBoard);

	if (m_pBoard)
	{
		switch (reg)
		{
			case 0:
				return m_pBoard->GetFDD()->GetStateDebug();

			case 1:
				return m_pBoard->GetFDD()->GetCmdDebug();

			case 2:
				return m_pBoard->GetFDD()->GetDataDebug();

			case 3:
				return m_pBoard->GetFDD()->GetWriteDataDebug();
		}

		ASSERT(false);
	}

	return 0;
}
uint16_t CDebugger::GetDebugHDDRegs(int nDrive, int num, bool bReadMode)
{
	ASSERT(m_pBoard);

	if (m_pBoard)
	{
		return m_pBoard->GetFDD()->ReadDebugHDDRegisters(nDrive, static_cast<HDD_REGISTER>(num), bReadMode);
	}

	return 0;
}

uint16_t CDebugger::GetDebugMemDumpWord(uint16_t addr)
{
	if (m_pBoard)
	{
		return m_pBoard->GetWordIndirect(addr & 0177776);
	}

	return 0;
}
uint8_t CDebugger::GetDebugMemDumpByte(uint16_t addr)
{
	if (m_pBoard)
	{
		return m_pBoard->GetByteIndirect(addr);
	}

	return 0;
}


inline int CDebugger::CalcLenOneWord()
{
	return 2;
}

inline int CDebugger::CalcLenTwoFields()
{
	return 2 + CalcArgLength(DST);
}

inline int CDebugger::CalcLenFourFields()
{
	return 2 + CalcArgLength(DST) + CalcArgLength(SRC);
}

inline int CDebugger::CalcLenFIS()
{
	register int reg = GetDigit(m_wInstr, 0);

	// если адрес блока параметров в PC, то длина команды 5 слов
	if (reg == CCPU::R_PC)
	{
		return 10;
	}

	return 2;
}


int CDebugger::CalcArgLength(int pos)
{
	register int reg = GetDigit(m_wInstr, pos++);
	register int meth = GetDigit(m_wInstr, pos);
	register int arg = 0;

	switch (meth)
	{
		/*
		case 0: // R0,      PC
		case 1: // (R0),    (PC)
		case 4: // -(R0),   -(PC)
		case 5: // @-(R0),  @-(PC)
		    break;
		*/
		case 2: // (R0)+,   #012345
		case 3: // @(R0)+,  @#012345
			if (reg == CCPU::R_PC)
			{
				arg = 2;
			}

			break;

		case 6: // 345(R0), 345
		case 7: // @345(R0),@345
			arg = 2;
			break;
	}

	return arg;
}

// разбор команды по заданному адресу.
// вход:    pc - адрес, по которому находится команда
// выход:   длина команды в словах
//          strInstr - мнемоника
//          strArg - аргументы команды, если есть
//          codes - массив машинных кодов, длиной [длина команды в словах]
//
int CDebugger::DebugInstruction(uint16_t pc, CString &strInstr, uint16_t *codes)
{
	ASSERT(m_pBoard);
	int length = 1;

	if (m_pBoard)
	{
		m_bPrevCmdCp = m_bPrevCmdC;
		m_bPrevCmdC = false;
		m_wInstr = m_pBoard->GetWordIndirect(pc);
		m_wPC = pc + 2;
		codes[0] = m_wInstr;
		m_strArg.Empty();
		m_strInstr = m_pInstrRefsMap[m_wInstr].Mnemonic;
		/* много скобок, но так надо. такой хитрый синтаксис */
		length += (this->*(m_pInstrRefsMap[m_wInstr].DisasmInstrRef))(codes); // эта функция ещё формирует m_strInstr и m_strArg
		strInstr = m_strInstr + m_strArg;
	}

	return length;
}

int CDebugger::DisassembleNoArgs(uint16_t *codes)
{
    (void)codes;
	return 0;
}

int CDebugger::DisassembleUnknown(uint16_t *codes)
{
    (void)codes;
    m_strArg.Format(m_strArgFormat_Addr, m_wInstr);
	return 0;
}

int CDebugger::DisassembleCLS(uint16_t *codes)
{
    (void)codes;
    if (m_wInstr == PI_CCC)
	{
		m_strInstr = _T("CCC");
	}
	else if (m_wInstr & 0xf) // если признаки есть
	{
		m_strInstr = _T("CL");

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::N)))
		{
			m_strInstr += _T('N');
		}

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::Z)))
		{
			m_strInstr += _T('Z');
		}

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::V)))
		{
			m_strInstr += _T('V');
		}

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::C)))
		{
			m_strInstr += _T('C');
		}
	}

	return 0;
}

int CDebugger::DisassembleSET(uint16_t *codes)
{
    (void)codes;
	if (m_wInstr == PI_SCC)
	{
		m_strInstr = _T("SCC");
	}
	else if (m_wInstr & 0xf) // если признаки есть
	{
		m_strInstr = _T("SE");

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::N)))
		{
			m_strInstr += _T('N');
		}

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::Z)))
		{
			m_strInstr += _T('Z');
		}

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::V)))
		{
			m_strInstr += _T('V');
		}

		if (m_wInstr & (1 << static_cast<int>(PSW_BIT::C)))
		{
			m_strInstr += _T('C');
		}
	}

	return 0;
}

int CDebugger::DisassembleRTS(uint16_t *codes)
{
    (void)codes;
	int r = GetDigit(m_wInstr, 0);

	if (r == CCPU::R_PC)
	{
		m_strInstr = _T("RETURN");
	}
	else
	{
		m_strArg = m_strRegNames[r];
	}

	return 0;
}

int CDebugger::DisassembleTwoField(uint16_t *codes)
{
	return ConvertArgToString(DST, m_wPC, m_strArg, codes[1]);
}

int CDebugger::DisassembleMARK(uint16_t *codes)
{
    (void)codes;
	m_strArg.Format(m_strArgFormat_Number, m_wInstr & 077);
	return 0;
}

int CDebugger::DisassembleEMT(uint16_t *codes)
{
    (void)codes;
	m_strArg.Format(m_strArgFormat_Number, LOBYTE(m_wInstr));
	return 0;
}

int CDebugger::DisassembleBR(uint16_t *codes)
{
    (void)codes;
    m_strArg.Format(m_strArgFormat_Addr, (m_wPC + (short)(char)LOBYTE(m_wInstr) * 2) & 0xffff);
	return 0;
}

int CDebugger::DisassembleBCC(uint16_t *codes)
{
    (void)codes;
    if (m_bPrevCmdCp)
	{
		m_strInstr = _T("BHIS  ");
	}

	return DisassembleBR(codes);
}

int CDebugger::DisassembleBCS(uint16_t *codes)
{
    (void)codes;
    if (m_bPrevCmdCp)
	{
		m_strInstr = _T("BLO   ");
	}

	return DisassembleBR(codes);
}

int CDebugger::DisassembleJSR(uint16_t *codes)
{
	int r = GetDigit(m_wInstr, 2);
	CString strDst;
	int length = ConvertArgToString(DST, m_wPC, strDst, codes[1]);

	if (r == CCPU::R_PC)
	{
		m_strInstr = _T("CALL  ");
		m_strArg = strDst;
	}
	else
	{
		m_strArg = m_strRegNames[r] + m_strArgFormat_Comma + strDst;
	}

	return length;
}

int CDebugger::DisassembleEISExt(uint16_t *codes)
{
	CString strDst;
	int length = ConvertArgToString(DST, m_wPC, strDst, codes[1]);
	m_strArg = strDst + m_strArgFormat_Comma + m_strRegNames[GetDigit(m_wInstr, 2)];
	return length;
}

int CDebugger::DisassembleXOR(uint16_t *codes)
{
	CString strDst;
	int length = ConvertArgToString(DST, m_wPC, strDst, codes[1]);
	m_strArg = m_strRegNames[GetDigit(m_wInstr, 2)] + m_strArgFormat_Comma + strDst;
	return length;
}

int CDebugger::DisassembleFIS(uint16_t *codes)
{
	register int reg = GetDigit(m_wInstr, 0);
	m_strArg = m_strRegNames[reg];

	// если адрес блока параметров в PC, то размер аргументов 4 слова
	if (reg == CCPU::R_PC)
	{
		uint16_t pc = m_wPC + 2;

		for (int i = 1; i <= 4; ++i)
		{
			codes[i] = m_pBoard->GetWordIndirect(pc);
			pc += 2;
		}

		return 4;
	}

	return 0;
}

int CDebugger::DisassembleSOB(uint16_t *codes)
{
    (void)codes;
    CString strDst;
	strDst.Format(m_strArgFormat_Addr, (m_wPC - (m_wInstr & 077) * 2) & 0xffff);
	m_strArg = m_strRegNames[GetDigit(m_wInstr, 2)] + m_strArgFormat_Comma + strDst;
	return 0;
}

int CDebugger::DisassembleFourField(uint16_t *codes)
{
	CString strSrc;
	CString strDst;
	int length = ConvertArgToString(SRC, m_wPC, strSrc, codes[1]);
	length += ConvertArgToString(DST, m_wPC + length + length, strDst, codes[1 + length]);
	m_strArg = strSrc + m_strArgFormat_Comma + strDst;
	return length;
}

int CDebugger::DisassembleCMP(uint16_t *codes)
{
	m_bPrevCmdC = true;
	return DisassembleFourField(codes);
}

int CDebugger::ConvertArgToString(int arg, uint16_t pc, CString &strSrc, uint16_t &code)
{
	ASSERT(m_pBoard);
	register int reg = GetDigit(m_wInstr, arg++);
	register int meth = GetDigit(m_wInstr, arg);

	if (reg == CCPU::R_PC)
	{
		switch (meth)
		{
			case 2:
			case 3:
				code = m_pBoard->GetWordIndirect(pc);
				strSrc.Format(m_strAddrFormat_PC[meth], code);
				return 1;

			case 6:
			case 7:
				code = m_pBoard->GetWordIndirect(pc);
				strSrc.Format(m_strAddrFormat_PC[meth], (pc + code + 2) & 0xffff);
				return 1;

			default:
				strSrc = m_strAddrFormat_PC[meth];
		}
	}
	else
	{
		switch (meth)
		{
			case 6:
			case 7:
				code = m_pBoard->GetWordIndirect(pc);
                strSrc.Format(m_strAddrFormat[meth], code, m_strRegNames[reg].toLatin1().data());
				return 1;

			default:
                strSrc.Format(m_strAddrFormat[meth], m_strRegNames[reg].toLatin1().data());
		}
	}

	return 0;
}

/*
расчёт следующего адреса, который будет после выполнения текущей инструкции.
вход: pc - текущий адрес.
выход:
    адрес следующий за инструкцией
    или ADDRESS_NONE, если его нельзя рассчитать
*/
uint16_t CDebugger::CalcNextAddr(uint16_t pc)
{
	m_wInstr = m_pBoard->GetWordIndirect(pc);
	m_wPC = pc + 2;

	if (m_bCBug)
	{
		m_bCBug = false;
		m_wFreg = m_pBoard->GetPSW() & 0177776;
	}
	else
	{
		m_wFreg = m_pBoard->GetPSW();
	}

	/* много скобок, но так надо. такой хитрый синтаксис */
	return (this->*(m_pInstrRefsMap[m_wInstr].NextAddrRef))(); // возвращаем из функции высчитанный адрес.
}

uint16_t CDebugger::CalcNextAddrRegular()
{
	return m_wPC - 2 + CalcInstructionLength(m_wInstr);
}


uint16_t CDebugger::CalcNextAddrRegular4()
{
	// тут перехватываем и обрабатываем ситуации типа MOV xxx,PC
	register uint16_t opcode = m_wInstr & 0170077;
	bool bByteOp = !!(m_wInstr & 0100000);
	uint16_t arg;
	uint16_t pc;

	if (bByteOp)
	{
		switch (opcode)
		{
			case 0110007:
				return short(char(GetArgD(SRC) & 0377));

			case 0140007:
				arg = GetArgD(SRC);
				pc = m_wPC & (~(arg & 0377));
				return pc;

			case 0150007:
				arg = GetArgD(SRC);
				pc = m_wPC | (arg & 0377);
				return pc;

			case 0160007:
				arg = GetArgD(SRC);
				pc = m_wPC - arg;
				return pc;
		}
	}
	else
	{
		switch (opcode)
		{
			case 010007:
				return GetArgD(SRC);

			case 040007:
				arg = GetArgD(SRC);
				pc = m_wPC & (~arg);
				return pc;

			case 050007:
				arg = GetArgD(SRC);
				pc = m_wPC | arg;
				return pc;

			case 060007:
				arg = GetArgD(SRC); // внутри функции GetArgD изменяется m_wPC, поэтому тут нельзя записать в одну строчку
				pc = m_wPC + arg;
				return pc;
		}
	}

	return CalcNextAddrRegular();
}


// вход: pos = источник/приёмник
// нужно, чтобы высчитать адрес перехода для команд типа mov xxx,PC
uint16_t CDebugger::GetArgD(int pos)
{
	register int reg = GetDigit(m_wInstr, pos++);
	register int meth = GetDigit(m_wInstr, pos);
	register uint16_t offs = (reg == CCPU::R_PC) ? 2 : 0;
	register uint16_t r = (reg == CCPU::R_PC) ? m_wPC : GetRegister(reg); // содержимое регистра
	register uint16_t arg = 0;
	register uint16_t index;

	switch (meth)
	{
		case 0: // R0,      PC
			arg = r;
			break;

		case 1: // (R0),    (PC)
		case 2: // (R0)+,   #012345
			arg = m_pBoard->GetWordIndirect(r);
			m_wPC += offs;
			break;

		case 3: // @(R0)+,  @#012345
			arg = m_pBoard->GetWordIndirect(r);
			arg = m_pBoard->GetWordIndirect(arg);
			m_wPC += offs;
			break;

		case 4: // -(R0),   -(PC)
			arg = m_pBoard->GetWordIndirect(r - 2);
			m_wPC -= offs;
			break;

		case 5: // @-(R0),  @-(PC)
			arg = m_pBoard->GetWordIndirect(r - 2);
			arg = m_pBoard->GetWordIndirect(arg);
			m_wPC -= offs;
			break;

		case 6: // 345(R0), 345
			index = m_pBoard->GetWordIndirect(m_wPC);
			m_wPC += offs;
			arg = m_pBoard->GetWordIndirect(index + r + offs); // после чтения индекса PC ещё увеличивается на 2
			break;

		case 7: // @345(R0),@345
			index = m_pBoard->GetWordIndirect(m_wPC);
			m_wPC += offs;
			arg = m_pBoard->GetWordIndirect(index + r + offs); // после чтения индекса PC ещё увеличивается на 2
			arg = m_pBoard->GetWordIndirect(arg);
			break;
	}

	return arg;
}

uint16_t CDebugger::CalcNextAddrRegular2()
{
	// тут перехватываем и обрабатываем ситуации типа CLR PC, SWAB PC, ASL PC и т.п.
	bool bByteOp = !!(m_wInstr & 0100000);
	register uint16_t opcode = m_wInstr & 07700;

	if ((m_wInstr & 0177700) == 07) // если приёмник PC
	{
		if (bByteOp)
		{
			char c = m_pBoard->GetPSWBit(PSW_BIT::C) ? 1 : 0;
			auto nPCl = char(m_wPC & 0377);

			switch (opcode)
			{
				case 05000: // clrb
					nPCl = 0;
					break;

				case 05100: // comb
					nPCl = ~nPCl;
					break;

				case 05200: // incb
					nPCl++;
					break;

				case 05300: // decb
					nPCl--;
					break;

				case 05400: // negb
					nPCl = -nPCl;
					break;

				case 05500: // adcb
					nPCl += c;
					break;

				case 05600: // sbcb
					nPCl -= c;
					break;

				case 06000: // rorb
					nPCl >>= 1;

					if (c)
					{
						nPCl |= 0200;
					}

					break;

				case 06100: // rolb
					nPCl <<= 1;
					nPCl += c;
					break;

				case 06200: // asrb
					nPCl /= 2;
					break;

				case 06300: // aslb
					nPCl <<= 1;
					break;

				case 06400: // MFPS
					nPCl = m_pBoard->GetPSW() & 0377;
					return short(nPCl);
			}

			return (m_wPC & 0177400) | nPCl;
		}
		else
		{
			short c = m_pBoard->GetPSWBit(PSW_BIT::C) ? 1 : 0;
			auto nPC = short(m_wPC);

			switch (opcode)
			{
				case 0300:
					nPC = SWAP_BYTE(nPC);
					break;

				case 05000: // clr
					nPC = 0;
					break;

				case 05100: // com
					nPC = ~nPC;
					break;

				case 05200: // inc
					nPC++;
					break;

				case 05300: // dec
					nPC--;
					break;

				case 05400: // neg
					nPC = -nPC;
					break;

				case 05500: // adc
					nPC += c;
					break;

				case 05600: // sbc
					nPC -= c;
					break;

				case 06000: // ror
					nPC >>= 1;

					if (c)
					{
						nPC |= 0100000;
					}

					break;

				case 06100: // rol
					nPC <<= 1;
					nPC += c;
					break;

				case 06200: // asr
					nPC = nPC / 2;
					break;

				case 06300: // asl
					nPC <<= 1;
					break;

				case 06700: // sxt
					break;
			}

			return nPC;
		}
	}

	return CalcNextAddrRegular();
}



uint16_t CDebugger::CalcNextAddrSWAB_MFPS()
{
	if ((m_wInstr & 070) == 0)
	{
		// баг процессора: если метод адресации 0, то внутренний регистр условий не обновляется
		m_bCBug = true;
	}

	return CalcNextAddrRegular2();
}


uint16_t CDebugger::CalcNextAddrMOVB()
{
	if ((m_wInstr & 070) == 0)
	{
		// баг процессора: если метод адресации 0, то внутренний регистр условий не обновляется
		m_bCBug = true;
	}

	return CalcNextAddrRegular4();
}


uint16_t CDebugger::CalcNextAddrUNKNOWN()
{
	return CMotherBoard::ADDRESS_NONE;
}

uint16_t CDebugger::CalcNextAddrRTI()
{
	return m_pBoard->GetWordIndirect(GetRegister(CCPU::R_SP));
}

uint16_t CDebugger::CalcNextAddrRTS()
{
	register int reg = GetDigit(m_wInstr, 0);

	if (reg == CCPU::R_PC)
	{
		return m_pBoard->GetWordIndirect(GetRegister(CCPU::R_SP));
	}
	else
	{
		return GetRegister(reg);
	}
}

uint16_t CDebugger::CalcNextAddrJMP()
{
	register int meth = GetDigit(m_wInstr, 1);

	if (meth == 0)
	{
		return CMotherBoard::ADDRESS_NONE;
	}
	else
	{
		return GetArgAddrD(meth, GetDigit(m_wInstr, 0));
	}
}


// нужно, чтобы высчитать адрес перехода для JMP и JSR
uint16_t CDebugger::GetArgAddrD(int meth, int reg)
{
	register uint16_t offs = (reg == CCPU::R_PC) ? 2 : 0;
	register uint16_t r = (reg == CCPU::R_PC) ? m_wPC : GetRegister(reg);
	register uint16_t arg = 0;
	register uint16_t index;

	switch (meth)
	{
		case 0: // R0,      PC
			arg = reg;
			break;

		case 1: // (R0),    (PC)
		case 2: // (R0)+,   #012345
			arg = r;
			break;

		case 3: // @(R0)+,  @#012345
			arg = m_pBoard->GetWordIndirect(r);
			break;

		case 4: // -(R0),   -(PC)
			arg = r - 2;
			break;

		case 5: // @-(R0),  @-(PC)
			arg = m_pBoard->GetWordIndirect(r - 2);
			break;

		case 6: // 345(R0), 345
			index = m_pBoard->GetWordIndirect(m_wPC);
			arg = index + r + offs;
			break;

		case 7: // @345(R0),@345
			index = m_pBoard->GetWordIndirect(m_wPC);
			arg = m_pBoard->GetWordIndirect(index + r + offs); // после чтения индекса PC ещё увеличивается на 2
			break;
	}

	return arg;
}

uint16_t CDebugger::CalcNextAddrMARK()
{
	return GetRegister(CCPU::R_R5);
}

uint16_t CDebugger::CalcNextAddrSOB()
{
	register int reg = GetDigit(m_wInstr, 2);
	return m_wPC - ((GetRegister(reg) - 1) ? (m_wInstr & 077) * 2 : 0);
}

uint16_t CDebugger::CalcNextAddrBR()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + br;
}

uint16_t CDebugger::CalcNextAddrBNE()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::Z) ? 0 : br);
}

uint16_t CDebugger::CalcNextAddrBEQ()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::Z) ? br : 0);
}

uint16_t CDebugger::CalcNextAddrBGE()
{
	register bool bn = GetFREGBit(PSW_BIT::N);
	register bool bv = GetFREGBit(PSW_BIT::V);
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + ((bn == bv) ? br : 0);
}

uint16_t CDebugger::CalcNextAddrBLT()
{
	register bool bn = GetFREGBit(PSW_BIT::N);
	register bool bv = GetFREGBit(PSW_BIT::V);
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + ((bn != bv) ? br : 0);
}

uint16_t CDebugger::CalcNextAddrBGT()
{
	register bool bn = GetFREGBit(PSW_BIT::N);
	register bool bz = GetFREGBit(PSW_BIT::Z);
	register bool bv = GetFREGBit(PSW_BIT::V);
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + ((bz || (bn != bv)) ? 0 : br);
}

uint16_t CDebugger::CalcNextAddrBLE()
{
	register bool bn = GetFREGBit(PSW_BIT::N);
	register bool bz = GetFREGBit(PSW_BIT::Z);
	register bool bv = GetFREGBit(PSW_BIT::V);
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + ((bz || (bn != bv)) ? br : 0);
}

uint16_t CDebugger::CalcNextAddrBPL()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::N) ? 0 : br);
}

uint16_t CDebugger::CalcNextAddrBMI()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::N) ? br : 0);
}

uint16_t CDebugger::CalcNextAddrBHI()
{
	register bool bz = GetFREGBit(PSW_BIT::Z);
	register bool bc = GetFREGBit(PSW_BIT::C);
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + ((bz || bc) ? 0 : br);
}

uint16_t CDebugger::CalcNextAddrBLOS()
{
	register bool bz = GetFREGBit(PSW_BIT::Z);
	register bool bc = GetFREGBit(PSW_BIT::C);
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + ((bz || bc) ? br : 0);
}

uint16_t CDebugger::CalcNextAddrBVC()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::V) ? 0 : br);
}

uint16_t CDebugger::CalcNextAddrBVS()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::V) ? br : 0);
}

uint16_t CDebugger::CalcNextAddrBCC()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::C) ? 0 : br);
}

uint16_t CDebugger::CalcNextAddrBCS()
{
	register short br = ((short)(char)LOBYTE(m_wInstr)) * 2;
	return m_wPC + (GetFREGBit(PSW_BIT::C) ? br : 0);
}

void CDebugger::RegisterMethodRef(uint16_t start, uint16_t end, const char *mnemonic, CalcInstrLenRef ilenmref, CalcNextAddrRef nxamref, DisassembleInstrRef dsimref)
{
	for (int opcode = start; opcode <= end; ++opcode)
	{
		m_pInstrRefsMap[opcode].Mnemonic = mnemonic;
		m_pInstrRefsMap[opcode].InstrLenRef = ilenmref;
		m_pInstrRefsMap[opcode].NextAddrRef = nxamref;
		m_pInstrRefsMap[opcode].DisasmInstrRef = dsimref;
	}
}

void CDebugger::InitMaps()
{
	if (m_pInstrRefsMap)
	{
		return;
	}

	m_pInstrRefsMap = new InstrFuncRefs[65536];

	if (m_pInstrRefsMap)
	{
        RegisterMethodRef(0000000, 0177777, (_T(".WORD ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleUnknown);   // uint16_t
        RegisterMethodRef(0000000, 0000000, (_T("HALT  ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleNoArgs);    // HALT
        RegisterMethodRef(0000001, 0000001, (_T("WAIT  ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleNoArgs);    // WAIT
        RegisterMethodRef(0000002, 0000002, (_T("RTI   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRTI,      &CDebugger::DisassembleNoArgs);    // RTI
        RegisterMethodRef(0000003, 0000003, (_T("BPT   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleNoArgs);    // BPT
        RegisterMethodRef(0000004, 0000004, (_T("IOT   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleNoArgs);    // IOT
        RegisterMethodRef(0000005, 0000005, (_T("RESET ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleNoArgs);    // RESET
        RegisterMethodRef(0000006, 0000006, (_T("RTT   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRTI,      &CDebugger::DisassembleNoArgs);    // RTT
        RegisterMethodRef(0000010, 0000013, (_T("START ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleNoArgs);    // START
        RegisterMethodRef(0000014, 0000017, (_T("STEP  ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleNoArgs);    // STEP
        RegisterMethodRef(0000100, 0000177, (_T("JMP   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrJMP,      &CDebugger::DisassembleTwoField);  // JMP
        RegisterMethodRef(0000200, 0000207, (_T("RTS   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRTS,      &CDebugger::DisassembleRTS);       // RTS / RETURN
        RegisterMethodRef(0000240, 0000257, (_T("NOP   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleCLS);       // CLS
        RegisterMethodRef(0000260, 0000277, (_T("NOP260")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleSET);       // SET
        RegisterMethodRef(0000300, 0000377, (_T("SWAB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrSWAB_MFPS, &CDebugger::DisassembleTwoField); // SWAB
        RegisterMethodRef(0000400, 0000777, (_T("BR    ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBR,       &CDebugger::DisassembleBR);        // BR
        RegisterMethodRef(0001000, 0001377, (_T("BNE   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBNE,      &CDebugger::DisassembleBR);        // BNE
        RegisterMethodRef(0001400, 0001777, (_T("BEQ   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBEQ,      &CDebugger::DisassembleBR);        // BEQ
        RegisterMethodRef(0002000, 0002377, (_T("BGE   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBGE,      &CDebugger::DisassembleBR);        // BGE
        RegisterMethodRef(0002400, 0002777, (_T("BLT   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBLT,      &CDebugger::DisassembleBR);        // BLT
        RegisterMethodRef(0003000, 0003377, (_T("BGT   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBGT,      &CDebugger::DisassembleBR);        // BGT
        RegisterMethodRef(0003400, 0003777, (_T("BLE   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBLE,      &CDebugger::DisassembleBR);        // BLE
        RegisterMethodRef(0004000, 0004777, (_T("JSR   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrJMP,      &CDebugger::DisassembleJSR);       // JSR / CALL
        RegisterMethodRef(0005000, 0005077, (_T("CLR   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // CLR
        RegisterMethodRef(0005100, 0005177, (_T("COM   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // COM
        RegisterMethodRef(0005200, 0005277, (_T("INC   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // INC
        RegisterMethodRef(0005300, 0005377, (_T("DEC   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // DEC
        RegisterMethodRef(0005400, 0005477, (_T("NEG   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // NEG
        RegisterMethodRef(0005500, 0005577, (_T("ADC   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ADC
        RegisterMethodRef(0005600, 0005677, (_T("SBC   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // SBC
        RegisterMethodRef(0005700, 0005777, (_T("TST   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleTwoField);  // TST
        RegisterMethodRef(0006000, 0006077, (_T("ROR   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ROR
        RegisterMethodRef(0006100, 0006177, (_T("ROL   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ROL
        RegisterMethodRef(0006200, 0006277, (_T("ASR   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ASR
        RegisterMethodRef(0006300, 0006377, (_T("ASL   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ASL
        RegisterMethodRef(0006400, 0006477, (_T("MARK  ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrMARK,     &CDebugger::DisassembleMARK);      // MARK
        RegisterMethodRef(0006500, 0006577, (_T("MFPI  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleTwoField);  // MFPI
        RegisterMethodRef(0006600, 0006677, (_T("MTPI  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleTwoField);  // MTPI
        RegisterMethodRef(0006700, 0006777, (_T("SXT   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // SXT
        RegisterMethodRef(0010000, 0017777, (_T("MOV   ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular4, &CDebugger::DisassembleFourField); // MOV
        RegisterMethodRef(0020000, 0027777, (_T("CMP   ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleCMP);       // CMP
        RegisterMethodRef(0030000, 0037777, (_T("BIT   ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleFourField); // BIT
        RegisterMethodRef(0040000, 0047777, (_T("BIC   ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular4, &CDebugger::DisassembleFourField); // BIC
        RegisterMethodRef(0050000, 0057777, (_T("BIS   ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular4, &CDebugger::DisassembleFourField); // BIS
        RegisterMethodRef(0060000, 0067777, (_T("ADD   ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular4, &CDebugger::DisassembleFourField); // ADD
        RegisterMethodRef(0070000, 0070777, (_T("MUL   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleEISExt);    // MUL
        RegisterMethodRef(0071000, 0071777, (_T("DIV   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleEISExt);    // DIV
        RegisterMethodRef(0072000, 0072777, (_T("ASH   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleEISExt);    // ASH
        RegisterMethodRef(0073000, 0073777, (_T("ASHC  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleEISExt);    // ASHC
        RegisterMethodRef(0074000, 0074777, (_T("XOR   ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleXOR);       // XOR
        RegisterMethodRef(0075000, 0075007, (_T("FADD  ")), &CDebugger::CalcLenFIS,        &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleFIS);       // FADD
        RegisterMethodRef(0075010, 0075017, (_T("FSUB  ")), &CDebugger::CalcLenFIS,        &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleFIS);       // FSUB
        RegisterMethodRef(0075020, 0075027, (_T("FMUL  ")), &CDebugger::CalcLenFIS,        &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleFIS);       // FMUL
        RegisterMethodRef(0075030, 0075037, (_T("FDIV  ")), &CDebugger::CalcLenFIS,        &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleFIS);       // FDIV
        RegisterMethodRef(0077000, 0077777, (_T("SOB   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrSOB,      &CDebugger::DisassembleSOB);       // SOB
        RegisterMethodRef(0100000, 0100377, (_T("BPL   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBPL,      &CDebugger::DisassembleBR);        // BPL
        RegisterMethodRef(0100400, 0100777, (_T("BMI   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBMI,      &CDebugger::DisassembleBR);        // BMI
        RegisterMethodRef(0101000, 0101377, (_T("BHI   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBHI,      &CDebugger::DisassembleBR);        // BHI
        RegisterMethodRef(0101400, 0101777, (_T("BLOS  ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBLOS,     &CDebugger::DisassembleBR);        // BLOS
        RegisterMethodRef(0102000, 0102377, (_T("BVC   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBVC,      &CDebugger::DisassembleBR);        // BVC
        RegisterMethodRef(0102400, 0102777, (_T("BVS   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBVS,      &CDebugger::DisassembleBR);        // BVS
        RegisterMethodRef(0103000, 0103377, (_T("BCC   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBCC,      &CDebugger::DisassembleBCC);       // BHIS/BCC
        RegisterMethodRef(0103400, 0103777, (_T("BCS   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrBCS,      &CDebugger::DisassembleBCS);       // BLO/BCS
        RegisterMethodRef(0104000, 0104377, (_T("EMT   ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleEMT);       // EMT
        RegisterMethodRef(0104400, 0104777, (_T("TRAP  ")), &CDebugger::CalcLenOneWord,    &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleEMT);       // TRAP
        RegisterMethodRef(0105000, 0105077, (_T("CLRB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // CLRB
        RegisterMethodRef(0105100, 0105177, (_T("COMB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // COMB
        RegisterMethodRef(0105200, 0105277, (_T("INCB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // INCB
        RegisterMethodRef(0105300, 0105377, (_T("DECB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // DECB
        RegisterMethodRef(0105400, 0105477, (_T("NEGB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // NEGB
        RegisterMethodRef(0105500, 0105577, (_T("ADCB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ADCB
        RegisterMethodRef(0105600, 0105677, (_T("SBCB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // SBCB
        RegisterMethodRef(0105700, 0105777, (_T("TSTB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleTwoField);  // TSTB
        RegisterMethodRef(0106000, 0106077, (_T("RORB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // RORB
        RegisterMethodRef(0106100, 0106177, (_T("ROLB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ROLB
        RegisterMethodRef(0106200, 0106277, (_T("ASRB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ASRB
        RegisterMethodRef(0106300, 0106377, (_T("ASLB  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular2, &CDebugger::DisassembleTwoField);  // ASLB
        RegisterMethodRef(0106400, 0106477, (_T("MTPS  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleTwoField);  // MTPS
        RegisterMethodRef(0106500, 0106577, (_T("MFPD  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleTwoField);  // MFPD
        RegisterMethodRef(0106600, 0106677, (_T("MTPD  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrUNKNOWN,  &CDebugger::DisassembleTwoField);  // MTPD
        RegisterMethodRef(0106700, 0106777, (_T("MFPS  ")), &CDebugger::CalcLenTwoFields,  &CDebugger::CalcNextAddrSWAB_MFPS, &CDebugger::DisassembleTwoField); // MFPS
        RegisterMethodRef(0110000, 0117777, (_T("MOVB  ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrMOVB,     &CDebugger::DisassembleFourField); // MOVB
        RegisterMethodRef(0120000, 0127777, (_T("CMPB  ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleCMP);       // CMPB
        RegisterMethodRef(0130000, 0137777, (_T("BITB  ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular,  &CDebugger::DisassembleFourField); // BITB
        RegisterMethodRef(0140000, 0147777, (_T("BICB  ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular4, &CDebugger::DisassembleFourField); // BICB
        RegisterMethodRef(0150000, 0157777, (_T("BISB  ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular4, &CDebugger::DisassembleFourField); // BISB
        RegisterMethodRef(0160000, 0167777, (_T("SUB   ")), &CDebugger::CalcLenFourFields, &CDebugger::CalcNextAddrRegular4, &CDebugger::DisassembleFourField); // SUB
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}


bool CDebugger::OnDebugModify_Regs(int nAddress, uint16_t nValue)
{
	if (m_pBoard)
	{
		if (CCPU::R_R0 <= nAddress && nAddress <= CCPU::R_PC)
		{
			m_pBoard->SetRON(nAddress, nValue);
		}
		else if (CCPU::R_PSW == nAddress)
		{
			m_pBoard->SetPSW(nValue);
		}
		else
		{
			ASSERT(false);
		}

		if (nAddress == CCPU::R_PC)
		{
			SetCurrentAddress(nValue);
		}
	}

	return true;
}

bool CDebugger::OnDebugModify_Ports(int nAddress, uint16_t nValue)
{
	if (m_pBoard)
	{
		switch (nAddress)
		{
			case SYS_PORT_177660:
				m_pBoard->SetWordIndirect(0177660, nValue);
				break;

			case SYS_PORT_177662_IN:
				m_pBoard->m_reg177662in = nValue;
				break;

			case SYS_PORT_177662_OUT:
				m_pBoard->SetWordIndirect(0177662, nValue);
				break;

			case SYS_PORT_177664:
				m_pBoard->SetWordIndirect(0177664, nValue);
				break;

			case SYS_PORT_177700:
				m_pBoard->SetWordIndirect(0177700, nValue);
				break;

			case SYS_PORT_177702:
				m_pBoard->SetWordIndirect(0177702, nValue);
				break;

			case SYS_PORT_177704:
				m_pBoard->SetWordIndirect(0177704, nValue);
				break;

			case SYS_PORT_177706:
				m_pBoard->SetWordIndirect(0177706, nValue);
				break;

			case SYS_PORT_177710:
				m_pBoard->SetWordIndirect(0177710, nValue);
				break;

			case SYS_PORT_177712:
				m_pBoard->SetWordIndirect(0177712, nValue);
				break;

			case SYS_PORT_177714_IN:
				m_pBoard->m_reg177714in = nValue;
				break;

			case SYS_PORT_177714_OUT:
				m_pBoard->SetWordIndirect(0177714, nValue);
				break;

			case SYS_PORT_177716_IN:
				m_pBoard->m_reg177716in = nValue;
				break;

			case SYS_PORT_177716_OUT_TAP:
				m_pBoard->Set177716RegTap(nValue);
				// В БК 2й разряд SEL1 фиксирует любую запись в этот регистр, взводя триггер D9.1 на неограниченное время, сбрасывается который любым чтением этого регистра.
				m_pBoard->m_reg177716in |= 4;
				break;

			case SYS_PORT_177716_OUT_MEM:
				m_pBoard->Set177716RegMem(nValue);
				m_pBoard->m_reg177716in |= 4;
				break;

			default:
				ASSERT(false);
		}
	}

	return true;
}

bool CDebugger::OnDebugModify_Memory(int nAddress, uint16_t nValue)
{
	if (m_pBoard)
	{
		m_pBoard->SetWordIndirect(nAddress, nValue);
	}

	return true;
}

bool CDebugger::OnDebugModify_Memory(int nAddress, uint8_t nValue)
{
	if (m_pBoard)
	{
		m_pBoard->SetByteIndirect(nAddress, nValue);
	}

	return true;
}

bool CDebugger::OnDebugModify_AltProData(int nAddress, uint16_t nValue)
{
	if (m_pBoard)
	{
		switch (nAddress)
		{
			case 0:
				m_pBoard->SetAltProMode(nValue);
				break;

			case 1:
				m_pBoard->SetAltProCode(nValue);
				break;
		}
	}

	return true;
}



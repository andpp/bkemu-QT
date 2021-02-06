// Debugger.h: interface for the CDebugger class.
//


#pragma once

#include "DisasmDlg.h"
#include "BreakPoint.h"
#include "CPU.h"

#define COLORED_TAG "<C>"
#define COLORED_TAG_LENGTH 3

enum : int
{
	SYS_PORT_177660 = 0,
	SYS_PORT_177662_IN,
	SYS_PORT_177662_OUT,
	SYS_PORT_177664,
	SYS_PORT_177700,
	SYS_PORT_177702,
	SYS_PORT_177704,
	SYS_PORT_177706,
	SYS_PORT_177710,
	SYS_PORT_177712,
	SYS_PORT_177714_IN,
	SYS_PORT_177714_OUT,
	SYS_PORT_177716_IN,
	SYS_PORT_177716_OUT_TAP,
	SYS_PORT_177716_OUT_MEM
};


// массив цветов для подсветки синтаксиса
enum : int
{
	HLCOLOR_DEFAULT = 0,
	HLCOLOR_ADDRESS,
	HLCOLOR_MNEMONIC,
	HLCOLOR_REGISTER,
	HLCOLOR_NUMBER,
	HLCOLOR_SYMBOL,
	HLCOLOR_NUM_COLS    // количество цветов
};
extern const COLORREF g_crDebugColorHighLighting[];


class CMotherBoard;
class CDisasmDlg;

class CDebugger: public QObject
{
    Q_OBJECT

		using CalcInstrLenRef = int (CDebugger::*)(); // в качестве аргумента - значение в m_wInstr, результат - возвращаемое значение
		using CalcNextAddrRef = uint16_t (CDebugger::*)(); // в качестве аргумента - значение в m_wInstr, m_wPC - адрес за опкодом, результат - следующий адрес, или ADDRESS_NONE, если рассчитать невозможно
		using DisassembleInstrRef = int (CDebugger::*)(uint16_t *); // в качестве аргумента - значение в m_wInstr и m_wPC, результат - в m_strInstr и m_strArg, возвращает кол-во доп.слов инструкции
		struct InstrFuncRefs
		{
			CString Mnemonic;
			CalcInstrLenRef InstrLenRef;
			CalcNextAddrRef NextAddrRef;
			DisassembleInstrRef DisasmInstrRef;
		};
		InstrFuncRefs *m_pInstrRefsMap;
		void RegisterMethodRef(uint16_t start, uint16_t end, CString &mnemonic, CalcInstrLenRef ilenmref, CalcNextAddrRef nxamref, DisassembleInstrRef dsimref);
        void RegisterMethodRef(uint16_t start, uint16_t end, const char *mnemonic, CalcInstrLenRef ilenmref, CalcNextAddrRef nxamref, DisassembleInstrRef dsimref);
        void InitMaps();

        QImage              m_hBPIcon, m_hCurrIcon;

		static int          m_outLevel;
		static const CString m_strRegNames[8];
		static const CString m_strAddrFormat[8];
		static const CString m_strAddrFormat_PC[8];
		static const CString m_strArgFormat_Addr;
		static const CString m_strArgFormat_Number;
		static const CString m_strArgFormat_Comma;

		CMotherBoard       *m_pBoard;
		CDisasmDlg         *m_pDisasmDlg;
		CBreakPointList     m_breakpointList;

		uint16_t            m_wTopAddress;  // адрес самой первой строки для дизассемблирования
		bool                m_bPrevCmdC;    // флаг, как дизассемблировать BCC/BHIS BCS/BLO
		bool                m_bPrevCmdCp;   // после команды CMP - сравнение, иначе - битС

		// для того, чтобы не передавать функциям кучу лишней информации, заведём глобальные переменные

		uint16_t            m_wPC;          // текущий адрес текущей инструкции
		uint16_t            m_wInstr;       // сама инструкция
		bool                m_bCBug;        // флаг бага флага С
		uint16_t            m_wFreg;        // регистр флагов состояний NZVC с учётом бага
		CString             m_strInstr;     // мнемоника
		CString             m_strArg;       // аргументы, если есть

		inline bool         GetFREGBit(PSW_BIT pos)
		{
			return !!(m_wFreg & (1 << static_cast<int>(pos)));
		}

		int                 ConvertArgToString(int arg, uint16_t pc, CString &strSrc, uint16_t &code);

		// Breakpoint managment methods
		bool                IsBpeakpointExists(CBreakPoint &breakpoint);
		bool                IsBpeakpointAtAddress(uint16_t addr);

		uint16_t            GetArgD(int pos);
		uint16_t            GetArgAddrD(int meth, int reg);
		int                 CalcArgLength(int pos);

		inline int          CalcLenOneWord();
		inline int          CalcLenTwoFields();
		inline int          CalcLenFourFields();
		inline int          CalcLenFIS();

		uint16_t            CalcNextAddrRegular4();
		uint16_t            CalcNextAddrRegular2();
		uint16_t            CalcNextAddrRegular();
		uint16_t            CalcNextAddrUNKNOWN();
		uint16_t            CalcNextAddrRTI();
		uint16_t            CalcNextAddrRTS();
		uint16_t            CalcNextAddrJMP();
		uint16_t            CalcNextAddrMARK();
		uint16_t            CalcNextAddrSOB();
		uint16_t            CalcNextAddrBR();
		uint16_t            CalcNextAddrBNE();
		uint16_t            CalcNextAddrBEQ();
		uint16_t            CalcNextAddrBGE();
		uint16_t            CalcNextAddrBLT();
		uint16_t            CalcNextAddrBGT();
		uint16_t            CalcNextAddrBLE();
		uint16_t            CalcNextAddrBPL();
		uint16_t            CalcNextAddrBMI();
		uint16_t            CalcNextAddrBHI();
		uint16_t            CalcNextAddrBLOS();
		uint16_t            CalcNextAddrBVC();
		uint16_t            CalcNextAddrBVS();
		uint16_t            CalcNextAddrBCC();
		uint16_t            CalcNextAddrBCS();
		uint16_t            CalcNextAddrSWAB_MFPS();
		uint16_t            CalcNextAddrMOVB();

		int                 DisassembleNoArgs(uint16_t *codes);
		int                 DisassembleUnknown(uint16_t *codes);
		int                 DisassembleCLS(uint16_t *codes);
		int                 DisassembleSET(uint16_t *codes);
		int                 DisassembleRTS(uint16_t *codes);
		int                 DisassembleTwoField(uint16_t *codes);
		int                 DisassembleFourField(uint16_t *codes);
		int                 DisassembleMARK(uint16_t *codes);
		int                 DisassembleEMT(uint16_t *codes);
		int                 DisassembleBR(uint16_t *codes);
		int                 DisassembleBCC(uint16_t *codes);
		int                 DisassembleBCS(uint16_t *codes);
		int                 DisassembleJSR(uint16_t *codes);
		int                 DisassembleEISExt(uint16_t *codes);
		int                 DisassembleXOR(uint16_t *codes);
		int                 DisassembleFIS(uint16_t *codes);
		int                 DisassembleSOB(uint16_t *codes);
		int                 DisassembleCMP(uint16_t *codes);

	public:
		CDebugger();
        virtual ~CDebugger();
		inline CMotherBoard *GetBoard()
		{
			return m_pBoard;
		}
		bool                GetDebugPCBreak(uint16_t addr);

		uint16_t            CalcNextAddr(uint16_t pc);

		int                 CalcInstructionLength(uint16_t instr);
		static void         InitOutMode()
		{
			m_outLevel = 0;
		}
		static bool         IsInstructionOut(uint16_t instruction);
		bool                IsInstructionOver(uint16_t instruction);

		void                AttachBoard(CMotherBoard *pBoard);
		inline void         AttachWnd(CDisasmDlg *pDlg)
		{
			m_pDisasmDlg = pDlg;
		}
		void                SetCurrentAddress(uint16_t address);
		inline uint16_t     GetCurrentAddress()
		{
			return m_wTopAddress;
		}

		uint16_t            GetLineAddress(int nNum);
		int                 DebugInstruction(uint16_t pc, CString &strInstr, uint16_t *codes);
		void                DrawDebuggerLine(int nNum, CDC *pDC, CRect *pRcSubs);
        void                DrawDebuggerLine(int nNum, QPainter &pnt);
		void                DrawColoredText(CDC *pDC, CRect &rect, CString &str);
        void                DrawColoredText(QPainter &pnt, int x, int y, CString &str);

		void                StepForward();
		void                StepBackward();

		uint16_t            GetCursorAddress();
		uint16_t            GetBottomAddress();

		uint16_t            GetRegister(int reg);
		uint16_t            GetPortValue(int addr);
		uint16_t            GetAltProData(int reg);
		uint16_t            GetFDDData(int reg);
		uint16_t            GetDebugHDDRegs(int nDrive, int num, bool bReadMode);
		uint16_t            GetDebugMemDumpWord(uint16_t addr);
		uint8_t             GetDebugMemDumpByte(uint16_t addr);

		bool                OnDebugModify_Regs(int nAddress, uint16_t nValue);
		bool                OnDebugModify_Ports(int nAddress, uint16_t nValue);
		bool                OnDebugModify_Memory(int nAddress, uint16_t nValue);
		bool                OnDebugModify_Memory(int nAddress, uint8_t nValue);
		bool                OnDebugModify_AltProData(int nAddress, uint16_t nValue);
		// Breakpoint managment methods
		bool                SetSimpleBreakpoint(uint16_t addr);
		bool                SetSimpleBreakpoint();
		bool                RemoveBreakpoint(uint16_t addr);
		bool                RemoveBreakpoint();
		void                ClearBreakpointList();
};

/*
Перерисовка окна дизассемблера.
всё начинается с функции CMotherBoard::BreakCPU()
    в ней посылается сообщение WM_CPU_DEBUGBREAK,
    в результате срабатывает функция CMainFrame::OnCpuBreak
        там берётся текущий PC и передаётся функции CDebugger::SetCurrentAddress
            этот адрес делается начальным и перерисовывается список дизассемблера

Просто, примитивно и неудобно.
особенно, работа с курсором.
 */

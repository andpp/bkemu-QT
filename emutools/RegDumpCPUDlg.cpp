//#include <QVBoxLayout>
#include <QGridLayout>
//#include <QToolBar>
#include <QResizeEvent>


#include "RegDumpCPUDlg.h"
#include "Debugger.h"
#include "MainWindow.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CUSTOMVIEW_MODE_PARAM CRegDumpCPUDlg::m_cvArray[CUSTOMVIEW_REGS_NUM] =
{
    { CV_DEC_VIEW, IDS_REGDUMPCPU_DEC },
    { CV_HEX_VIEW, IDS_REGDUMPCPU_HEX }
};

constexpr auto COLUMN_WIDTH     = 60;
constexpr auto COLUMN_WIDTH_PRT = 78;
constexpr auto COLUMN_WIDTH_REG = 38;
constexpr auto COLUMN_WIDTH_TXT = 70;

const UINT CRegDumpCPUDlg::m_pListCpuIDs[9] =
{
    IDS_MEMORY_R0, IDS_MEMORY_R1, IDS_MEMORY_R2, IDS_MEMORY_R3,
    IDS_MEMORY_R4, IDS_MEMORY_R5, IDS_MEMORY_SP, IDS_MEMORY_PC,
    IDS_MEMORY_PSW
};

const UINT CRegDumpCPUDlg::m_pListCpuRegs[9] =
{
    CCPU::R_R0, CCPU::R_R1, CCPU::R_R2, CCPU::R_R3,
    CCPU::R_R4, CCPU::R_R5, CCPU::R_SP, CCPU::R_PC,
    CCPU::R_PSW
};

const UINT CRegDumpCPUDlg::m_pListSysIDs[2][9] =
{
    {
        IDS_MEMORY_177660, IDS_MEMORY_177662IN, IDS_MEMORY_177662OUT, IDS_MEMORY_177664,
        IDS_MEMORY_177714IN, IDS_MEMORY_177714OUT, IDS_MEMORY_177716IN, IDS_MEMORY_177716OUT_TAP,
        IDS_MEMORY_177716OUT_MEM
    },
    {
        IDS_MEMORY_177700, IDS_MEMORY_177702, IDS_MEMORY_177704, IDS_MEMORY_177706,
        IDS_MEMORY_177710, IDS_MEMORY_177712, 0, 0, 0
    }
};

const UINT CRegDumpCPUDlg::m_pListSysRegs[2][9] =
{
    {
        SYS_PORT_177660, SYS_PORT_177662_IN, SYS_PORT_177662_OUT, SYS_PORT_177664,
        SYS_PORT_177714_IN, SYS_PORT_177714_OUT, SYS_PORT_177716_IN, SYS_PORT_177716_OUT_TAP,
        SYS_PORT_177716_OUT_MEM
    },
    {
        SYS_PORT_177700, SYS_PORT_177702, SYS_PORT_177704, SYS_PORT_177706,
        SYS_PORT_177710, SYS_PORT_177712, 0, 0, 0
    }
};


const UINT CRegDumpCPUDlg::m_pListAltProIDs[2] =
{
    IDS_ALTPRO_MODE, IDS_ALTPRO_CODE
};

CRegDumpCPUDlg::CRegDumpCPUDlg(QWidget *parent) : QWidget(parent)
   , m_pDebugger(nullptr)
{

    CString name;

    for (int i = LISTCPU_L::LINE_R0; i <= LISTCPU_L::LINE_PSW; ++i)
    {
        name.LoadString(m_pListCpuIDs[i]);
        m_listCPU[i] = new CRegDumpCPUCtrl(m_pListCpuRegs[i], name, this);
        m_listCPU[i]->move(5, i * 20);
    }

    setMinimumSize(165, 200);
}

CRegDumpCPUDlg::~CRegDumpCPUDlg()
{
}

void CRegDumpCPUDlg::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    size.setHeight(size.height() - 10);
}

void CRegDumpCPUDlg::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
    for (int i = LISTCPU_L::LINE_R0; i <= LISTCPU_L::LINE_PSW; ++i)
    {
        m_listCPU[i]->AttachDebugger((pDebugger));
    }
}


/////////////////////////////////////////////////////////////////////////////
// CRegDumpDlg message handlers

//BEGIN_MESSAGE_MAP(CRegDumpCPUDlg, CDialogEx)
//	ON_WM_SIZE()
//	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RDC_CPUREGS, &CRegDumpCPUDlg::OnLvnItemchangedMdCpuRegisters)
//	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RDC_SYSREGS, &CRegDumpCPUDlg::OnLvnItemchangedMdSysRegisters)
//	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RDC_ALTPRO_DATA, &CRegDumpCPUDlg::OnLvnItemchangedMdAltProData)
//	ON_EN_CHANGE(IDC_EDIT_RDC_CPUFREQ, &CRegDumpCPUDlg::OnEnChangeEditDbg1Cpufreq)
//	ON_BN_CLICKED(IDC_BUTTON_RDC_CPUFREQ_DEC, &CRegDumpCPUDlg::OnBnClickedButtonDbg1CpufreqDec)
//	ON_BN_CLICKED(IDC_BUTTON_RDC_CPUFREQ_INC, &CRegDumpCPUDlg::OnBnClickedButtonDbg1CpufreqInc)
//	ON_BN_CLICKED(IDC_BUTTON_RDC_CPUFREQ_BASESET, &CRegDumpCPUDlg::OnBnClickedButtonDbg1CpufreqBaseset)
//	ON_BN_CLICKED(IDC_BUTTON_RDC_CVMODE, &CRegDumpCPUDlg::OnBnClickedRegdumpCvmode)
//END_MESSAGE_MAP()
#if 0
BOOL CRegDumpCPUDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    m_hAccelTable = ::LoadAccelerators(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
    LOGFONT lf;
    ::GetObject((HGDIOBJ)::SendMessage(GetSafeHwnd(), WM_GETFONT, 0, 0), sizeof(LOGFONT), &lf);
    _tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Courier New\0"));

    if (!m_hFont.CreateFontIndirect(&lf))
    {
        _tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Lucida Console\0"));
        m_hFont.CreateFontIndirect(&lf);
    }

    // Подсказки
    m_ToolTip.Create(this, WS_POPUP | TTS_NOPREFIX | TTS_USEVISUALSTYLE | TTS_ALWAYSTIP/* | TTS_BALLOON*/);
    m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 20000);
    // *** you could change the default settings ***
    m_ToolTip.SetMaxTipWidth(200);
    // m_ToolTip.SetTipBkColor(RGB(121, 121, 121));
    // m_ToolTip.SetTipTextColor(RGB(255, 0, 0));
    m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_RDC_CPUFREQ_DEC), ID_CPU_SLOWDOWN);
    m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_RDC_CPUFREQ_INC), ID_CPU_ACCELERATE);
    m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_RDC_CPUFREQ_BASESET), ID_CPU_NORMALSPEED);
    // Подготовим ленту с иконками и имаджлист
    CBitmap m_bmparr;
    m_bmparr.LoadBitmap(IDR_MAINFRAME_256);
    CImageList m_imagelist;
    m_imagelist.Create(16, 15, ILC_COLOR32 | ILC_MASK, 0, 0);
//  m_imagelist.SetBkColor(RGB(200, 200, 200));
    // Наложим ленту на имаджлист, учитывая фон/прозрачность (в нашем случае какой-то серый)
    m_imagelist.Add(&m_bmparr, RGB(192, 192, 192));
    // Вытаскиваем на кнопки по одной иконке из имаджлиста
    (reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_RDC_CPUFREQ_DEC)))->SetIcon(m_imagelist.ExtractIcon(5));
    (reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_RDC_CPUFREQ_INC)))->SetIcon(m_imagelist.ExtractIcon(4));
    (reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_RDC_CPUFREQ_BASESET)))->SetIcon(m_imagelist.ExtractIcon(6));
    // Уничтожим ненужное более
    m_imagelist.DeleteImageList();
    m_bmparr.DeleteObject();
    int nPixelW = GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSX);
    // Создадим список регистров CPU
    // 3 колонки, 1-я - имя регистра, 2-я - его значение, 3-я - кастомное значение
    CString strHeader;
    m_listCPU.SetFont(&m_hFont, FALSE);
    m_listCPU.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER | LVS_EX_SNAPTOGRID);
    m_listCPU.AcceptDigits(false); // принимаем не только цифры
    m_listCPU.InsertColumn(LISTCPU_C::COL_NAME, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH_REG, nPixelW, 96));
    m_listCPU.InsertColumn(LISTCPU_C::COL_VALUE, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH, nPixelW, 96));
    m_listCPU.InsertColumn(LISTCPU_C::COL_CUSTOM, LPSTR_TEXTCALLBACK, LVCFMT_RIGHT, MulDiv(COLUMN_WIDTH_TXT, nPixelW, 96));

    for (int i = LISTCPU_L::LINE_R0; i <= LISTCPU_L::LINE_PSW; ++i)
    {
        strHeader.LoadString(m_pListCpuIDs[i]);
        m_listCPU.InsertItem(i, strHeader);
        m_listCPU.SetItem(i, LISTCPU_C::COL_VALUE, LVIF_TEXT, LPSTR_TEXTCALLBACK, 0, 0, 0, 0);
        m_listCPU.SetItem(i, LISTCPU_C::COL_CUSTOM, LVIF_TEXT, LPSTR_TEXTCALLBACK, 0, 0, 0, 0);
    }

    m_listCPU.SetColumnBkColor(GetSysColor(COLOR_BTNFACE), LISTCPU_C::COL_NAME);
    m_listCPU.EnableColumnEdit(false, LISTCPU_C::COL_NAME);
    // Создадим список портов и системных регистров
    // 4 колонки, 1-я и 3-я - адрес порта/регистра, 2-я и 4-я - их значения
    m_listSys.SetFont(&m_hFont, FALSE);
    m_listSys.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER | LVS_EX_SNAPTOGRID);
    m_listSys.InsertColumn(LISTSYS_C::COL_NAME1, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH_PRT, nPixelW, 96));
    m_listSys.InsertColumn(LISTSYS_C::COL_VALUE1, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH, nPixelW, 96));
    m_listSys.InsertColumn(LISTSYS_C::COL_NAME2, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH, nPixelW, 96));
    m_listSys.InsertColumn(LISTSYS_C::COL_VALUE2, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH, nPixelW, 96));

    for (int i = LISTSYS_L::LINE_REG177660; i <= LISTSYS_L::LINE_REG177716OUT_MEM; ++i)
    {
        strHeader.LoadString(m_pListSysIDs[0][i]);
        m_listSys.InsertItem(i, strHeader);
        m_listSys.SetItem(i, LISTSYS_C::COL_VALUE1, LVIF_TEXT, LPSTR_TEXTCALLBACK, 0, 0, 0, 0);

        if (m_pListSysIDs[1][i])
        {
            strHeader.LoadString(m_pListSysIDs[1][i]);
        }
        else
        {
            strHeader.Empty();
        }

        m_listSys.SetItem(i, LISTSYS_C::COL_NAME2, LVIF_TEXT, strHeader, 0, 0, 0, 0);
        m_listSys.SetItem(i, LISTSYS_C::COL_VALUE2, LVIF_TEXT, LPSTR_TEXTCALLBACK, 0, 0, 0, 0);
    }

    m_listSys.SetColumnBkColor(GetSysColor(COLOR_BTNFACE), LISTSYS_C::COL_NAME1);
    m_listSys.EnableColumnEdit(false, LISTSYS_C::COL_NAME1);
    m_listSys.SetColumnBkColor(GetSysColor(COLOR_BTNFACE), LISTSYS_C::COL_NAME2);
    m_listSys.EnableColumnEdit(false, LISTSYS_C::COL_NAME2);

    // ещё надо запретить редактировать свободные ячейки в 3-м столбце
    for (int i = LISTSYS_L2::LINE_REG177712 + 1; i <= LISTSYS_L::LINE_REG177716OUT_MEM; ++i)
    {
        m_listSys.EnableEdit(false, i, LISTSYS_C::COL_VALUE2);
    }

    // создание списка режимов AltPro
    m_listAltPro.SetFont(&m_hFont, FALSE);
    m_listAltPro.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER | LVS_EX_SNAPTOGRID);
    m_listAltPro.InsertColumn(LISTALTPRO_C::COL_NAMEA, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH_PRT, nPixelW, 96));
    m_listAltPro.InsertColumn(LISTALTPRO_C::COL_VALUEA, LPSTR_TEXTCALLBACK, LVCFMT_LEFT, MulDiv(COLUMN_WIDTH, nPixelW, 96));

    for (int i = LISTALTPRO_L::LINE_MODE; i <= LISTALTPRO_L::LINE_CODE; ++i)
    {
        strHeader.LoadString(m_pListAltProIDs[i]);
        m_listAltPro.InsertItem(i, strHeader);
        m_listAltPro.SetItem(i, LISTALTPRO_C::COL_VALUEA, LVIF_TEXT, nullptr, 0, 0, 0, 0);
    }

    m_listAltPro.SetColumnBkColor(GetSysColor(COLOR_BTNFACE), LISTALTPRO_C::COL_NAMEA);
    m_listAltPro.EnableColumnEdit(false, LISTALTPRO_C::COL_NAMEA);
    ResizeList(&m_listAltPro, LISTALTPRO_L::LINE_CODE + 1);
    int nOffset = ResizeList(&m_listSys, LISTSYS_L::LINE_REG177716OUT_MEM + 1);
    SlideObj(GetDlgItem(IDC_STATIC_RDC_SMKREGTITLE), nOffset);
    SlideObj(GetDlgItem(IDC_LIST_RDC_ALTPRO_DATA), nOffset);
    nOffset = ResizeList(&m_listCPU, LISTCPU_L::LINE_PSW + 1);
    SlideObj(GetDlgItem(IDC_STATIC_RDC_SYSREGTITLE), nOffset);
    SlideObj(GetDlgItem(IDC_LIST_RDC_SYSREGS), nOffset);
    SlideObj(GetDlgItem(IDC_STATIC_RDC_SMKREGTITLE), nOffset);
    SlideObj(GetDlgItem(IDC_LIST_RDC_ALTPRO_DATA), nOffset);
    CString t;
    t.LoadString(m_cvArray[m_nCurrentCVM].strID);
    SetDlgItemText(IDC_BUTTON_RDC_CVMODE, t);
    // Create layout
    m_Resizer.Init(this);
    m_Resizer.AddCtrl(IDC_LIST_RDC_CPUREGS, DT_LEFT | DT_TOP, TRUE, TRUE);
    m_Resizer.AddCtrl(IDC_LIST_RDC_SYSREGS, DT_LEFT | DT_TOP, TRUE, TRUE);
    m_Resizer.AddCtrl(IDC_LIST_RDC_ALTPRO_DATA, DT_LEFT | DT_TOP, TRUE, TRUE);
    m_Resizer.AddCtrl(IDC_BUTTON_RDC_CVMODE, DT_LEFT | DT_TOP, TRUE, TRUE);
    m_ToolTip.Activate(TRUE);
    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
#endif

//void CRegDumpCPUDlg::OnSize(UINT nType, int cx, int cy)
//{
//	CDialogEx::OnSize(nType, cx, cy);
//	// Resize layout to current window size
//	m_Resizer.OnSize(cx, cy);
//}

//CSize CRegDumpCPUDlg::GetMinSize()
//{
//	CSize size;
//	MINMAXINFO mmi;
//	m_Resizer.GetMinMaxInfo(&mmi);
//	size.cx = mmi.ptMinTrackSize.x;
//	size.cy = mmi.ptMinTrackSize.y;
//	return size;
//}


CString CRegDumpCPUDlg::FormatPSW(uint16_t value)
{
    //                   012345678
    CString strPSW = _T("--------");

    if (value & (1 << static_cast<int>(PSW_BIT::HALT)))
    {
        strPSW.SetAt(0, _T('H'));
    }

    if (value & (1 << static_cast<int>(PSW_BIT::MASKI)))
    {
        strPSW.SetAt(1, _T('M'));
    }

    if (value & (1 << static_cast<int>(PSW_BIT::P)))
    {
        strPSW.SetAt(2, _T('P'));
    }

    if (value & (1 << static_cast<int>(PSW_BIT::T)))
    {
        strPSW.SetAt(3, _T('T'));
    }

    if (value & (1 << static_cast<int>(PSW_BIT::N)))
    {
        strPSW.SetAt(4, _T('N'));
    }

    if (value & (1 << static_cast<int>(PSW_BIT::Z)))
    {
        strPSW.SetAt(5, _T('Z'));
    }

    if (value & (1 << static_cast<int>(PSW_BIT::V)))
    {
        strPSW.SetAt(6, _T('V'));
    }

    if (value & (1 << static_cast<int>(PSW_BIT::C)))
    {
        strPSW.SetAt(7, _T('C'));
    }

    return strPSW;
}


uint16_t CRegDumpCPUDlg::GetPSW(CString &strPSW)
{
    register uint16_t psw = 0;

    strPSW.Trim();
    strPSW.MakeUpper();
    register int nStrLen = strPSW.GetLength();
    // все символы могут быть любыми, но воспринимаются только те, что нужны.
    // причём порядок следования не важен, количество символов не важно
    for (int i = 0; i < nStrLen; ++i)
    {
        switch (strPSW.GetAt(i))
        {
            case _T('C'):
                psw |= (1 << static_cast<int>(PSW_BIT::C));
                break;

            case _T('V'):
                psw |= (1 << static_cast<int>(PSW_BIT::V));
                break;

            case _T('Z'):
                psw |= (1 << static_cast<int>(PSW_BIT::Z));
                break;

            case _T('N'):
                psw |= (1 << static_cast<int>(PSW_BIT::N));
                break;

            case _T('T'):
                psw |= (1 << static_cast<int>(PSW_BIT::T));
                break;

            case _T('P'):
                psw |= (1 << static_cast<int>(PSW_BIT::P));
                break;

            case _T('M'):
                psw |= (1 << static_cast<int>(PSW_BIT::MASKI));
                break;

            case _T('H'):
                psw |= (1 << static_cast<int>(PSW_BIT::HALT));
                break;
        }
    }

    return psw;
}

void CRegDumpCPUDlg::DisplayPortRegs()
{
    if (m_pDebugger)
    {
        for (int i = LISTSYS_L::LINE_REG177660; i <= LISTSYS_L::LINE_REG177716OUT_MEM; ++i)
        {
//			m_listSys.SetItemWithModified(m_pDebugger->GetPortValue(m_pListSysRegs[0][i]), i, LISTSYS_C::COL_VALUE1);
        }

        for (int i = LISTSYS_L2::LINE_REG177700; i <= LISTSYS_L2::LINE_REG177712; ++i)
        {
//			m_listSys.SetItemWithModified(m_pDebugger->GetPortValue(m_pListSysRegs[1][i]), i, LISTSYS_C::COL_VALUE2);
        }
    }
}

void CRegDumpCPUDlg::DisplayRegisters()
{
    if (m_pDebugger)
    {
        for (int i = LISTCPU_L::LINE_R0; i <= LISTCPU_L::LINE_PC; ++i)
        {
            uint16_t val = m_pDebugger->GetRegister(i);
            m_listCPU[i]->SetValue(val);
            m_listCPU[i]->repaint();
            // выводим первую колонку
//			m_listCPU.SetItemWithModified(val, i, LISTCPU_C::COL_VALUE);
            // вторую колонку
//			m_listCPU.SetItemWithModifiedASCII(WordToCustom(val), i, LISTCPU_C::COL_CUSTOM);
        }

//		m_listCPU.SetItemWithModified(m_pDebugger->GetRegister(LISTCPU_L::LINE_PSW), LISTCPU_L::LINE_PSW, LISTCPU_C::COL_VALUE);
        SetPSW(m_pDebugger->GetBoard()->GetPSW());
    }
}

void CRegDumpCPUDlg::SetPSW(uint16_t value)
{
//	m_listCPU.SetItemWithModifiedASCII(FormatPSW(value), LISTCPU_L::LINE_PSW, LISTCPU_C::COL_CUSTOM);
    m_listCPU[LISTCPU_L::LINE_PSW]->SetTextValue(FormatPSW(value));
    m_listCPU[LISTCPU_L::LINE_PSW]->repaint();
}

void CRegDumpCPUDlg::DisplayAltProData()
{
    if (m_pDebugger)
    {
//		m_listAltPro.SetItemWithModified(m_pDebugger->GetAltProData(LISTALTPRO_L::LINE_MODE), LISTALTPRO_L::LINE_MODE, LISTALTPRO_C::COL_VALUEA);
//		m_listAltPro.SetItemWithModified(m_pDebugger->GetAltProData(LISTALTPRO_L::LINE_CODE), LISTALTPRO_L::LINE_CODE, LISTALTPRO_C::COL_VALUEA);
    }
}

#if 0
void CRegDumpCPUDlg::OnLvnItemchangedMdCpuRegisters(NMHDR *pNMHDR, LRESULT *pResult)
{
    auto pNMListView = reinterpret_cast<LPNMLISTVIEW >(pNMHDR);
    ASSERT(pNMListView->iItem <= CCPU::R_PSW);
    *pResult = S_OK;
    register int nSubItem = pNMListView->iSubItem;
    register int addr = pNMListView->iItem;
    register uint16_t value;

    if (nSubItem == LISTCPU_C::COL_VALUE)
    {
        register auto pStr = reinterpret_cast<CString *>(pNMListView->lParam);
        value = OctStringToWord(*pStr);
    }
    else if (nSubItem == LISTCPU_C::COL_CUSTOM)
    {
        register auto pStr = reinterpret_cast<CString *>(pNMListView->lParam);
        value = (addr == CCPU::R_PSW) ? GetPSW(*pStr) : CustomToWord(*pStr);
    }
    else
    {
        return; // nSubItem может быть и другое значение
    }

    m_listCPU.SetEndEdit(addr, nSubItem);

    if (m_pDebugger)
    {
        m_pDebugger->OnDebugModify_Regs(addr, value);
    }

    if (addr == CCPU::R_PSW)
    {
        m_listCPU.SetItemWithModified(value, LISTCPU_L::LINE_PSW, LISTCPU_C::COL_VALUE);
        SetPSW(value);
    }
    else
    {
        m_listCPU.SetItemWithModified(value, addr, LISTCPU_C::COL_VALUE);
        m_listCPU.SetItemWithModifiedASCII(WordToCustom(value), addr, LISTCPU_C::COL_CUSTOM);
    }
}
#endif

CString CRegDumpCPUDlg::WordToCustom(uint16_t value)
{
    CString str;

    switch (m_nCurrentCVM)
    {
        case CV_DEC_VIEW:
            str.Format(_T("%05d"), value);
            break;

        case CV_HEX_VIEW:
            str.Format(_T("0x%04x"), value);
            break;
    }

    return str;
}

uint16_t CRegDumpCPUDlg::CustomToWord(const CString &str)
{
    uint16_t res = 0;

    switch (m_nCurrentCVM)
    {
        case CV_DEC_VIEW:
            res = str.toInt(nullptr, 10);
            break;

        case CV_HEX_VIEW:
            res = uint16_t(str.toInt(nullptr, 16));
            break;
    }

    return res;
}

#if 0
void CRegDumpCPUDlg::OnLvnItemchangedMdAltProData(NMHDR *pNMHDR, LRESULT *pResult)
{
    auto pNMListView = reinterpret_cast<LPNMLISTVIEW >(pNMHDR);
    register int nSubItem = pNMListView->iSubItem;

    if (nSubItem == LISTALTPRO_L::LINE_CODE)
    {
        register int addr = pNMListView->iItem;
        register auto pStr = reinterpret_cast<CString *>(pNMListView->lParam);
        register uint16_t value = OctStringToWord(*pStr);
        m_listAltPro.SetEndEdit(addr, nSubItem);

        if (m_pDebugger)
        {
            m_pDebugger->OnDebugModify_AltProData(addr, value);
        }

        m_listAltPro.SetItemWithModified(value, addr, nSubItem);
        // Тут надо принудительно обновить содержимое дампера
        ::SendMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_MEMDUMP_NEED_UPDATE, 0, 0);
    }

    *pResult = S_OK;
}


void CRegDumpCPUDlg::OnLvnItemchangedMdSysRegisters(NMHDR *pNMHDR, LRESULT *pResult)
{
    auto pNMListView = reinterpret_cast<LPNMLISTVIEW >(pNMHDR);
    *pResult = S_OK;
    register int addr = -1;
    register int nSubItem = pNMListView->iSubItem;
    register int nitem = pNMListView->iItem;

    switch (nSubItem)
    {
        case LISTSYS_C::COL_VALUE1:
            ASSERT(nitem <= LISTSYS_L::LINE_REG177716OUT_MEM);
            addr = m_pListSysRegs[0][nitem];
            break;

        case LISTSYS_C::COL_VALUE2:
            ASSERT(nitem <= LISTSYS_L2::LINE_REG177712);
            addr = m_pListSysRegs[1][nitem];
            break;

        default:
            return;
    }

    register auto pStr = reinterpret_cast<CString *>(pNMListView->lParam);
    register uint16_t value = OctStringToWord(*pStr);
    m_listSys.SetEndEdit(nitem, nSubItem);

    if (m_pDebugger)
    {
        m_pDebugger->OnDebugModify_Ports(addr, value);
    }

    m_listSys.SetItemWithModified(value, nitem, nSubItem);
}
#endif


#if 0
// для того, чтобы акселераторы работали, их надо загрузить и в этот диалог
// и передавать главному фрейму.
// способа проще найти не получилось.
BOOL CRegDumpCPUDlg::PreTranslateMessage(MSG *pMsg)
{
    if (m_hAccelTable)
    {
        if (::TranslateAccelerator(AfxGetMainWnd()->GetSafeHwnd(), m_hAccelTable, pMsg))
        {
            return TRUE;
        }
    }

    if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
    {
        m_ToolTip.RelayEvent(pMsg);
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}

#endif

void CRegDumpCPUDlg::OnEnChangeEditDbg1Cpufreq()
{
//    UpdateData(TRUE);
    register CMotherBoard *pBoard = m_pDebugger->GetBoard();
    ASSERT(pBoard);

    if (pBoard)
    {
        pBoard->SetCPUFreq(m_nCurrCPUFreq);
        // UpdateData(FALSE); // вот это не нужно. мешает только
    }
}

void CRegDumpCPUDlg::OnBnClickedButtonDbg1CpufreqDec()
{
    register CMotherBoard *pBoard = m_pDebugger->GetBoard();
    ASSERT(pBoard);

    if (pBoard && pBoard->CanSlowDown())
    {
        pBoard->SlowdownCPU();
        m_nCurrCPUFreq = pBoard->GetCPUFreq();
//        UpdateData(FALSE);
    }
}

void CRegDumpCPUDlg::OnBnClickedButtonDbg1CpufreqInc()
{
    register CMotherBoard *pBoard = m_pDebugger->GetBoard();
    ASSERT(pBoard);

    if (pBoard && pBoard->CanAccelerate())
    {
        pBoard->AccelerateCPU();
        m_nCurrCPUFreq = pBoard->GetCPUFreq();
//        UpdateData(FALSE);
    }
}

void CRegDumpCPUDlg::OnBnClickedButtonDbg1CpufreqBaseset()
{
    register CMotherBoard *pBoard = m_pDebugger->GetBoard();
    ASSERT(pBoard);

    if (pBoard)
    {
        pBoard->NormalizeCPU();
        m_nCurrCPUFreq = pBoard->GetCPUFreq();
//        UpdateData(FALSE);
    }
}

// отобразим текущую частоту
void CRegDumpCPUDlg::DisplayFreqParam()
{
    CMotherBoard *pBoard = m_pDebugger->GetBoard();
    ASSERT(pBoard);
    m_nCurrCPUFreq = pBoard->GetCPUFreq();
//    m_spinCPUFreq.SetRange32(pBoard->GetLowBound(), pBoard->GetHighBound());
//    m_spinCPUFreq.SetPos32(m_nCurrCPUFreq);
//    m_spinCPUFreq.SetBuddy(GetDlgItem(IDC_EDIT_RDC_CPUFREQ));
//    UpdateData(FALSE);
}

void CRegDumpCPUDlg::UpdateFreq()
{
    CMotherBoard *pBoard = m_pDebugger->GetBoard();
    ASSERT(pBoard);
    m_nCurrCPUFreq = pBoard->GetCPUFreq();
//    UpdateData(FALSE);
}


void CRegDumpCPUDlg::OnBnClickedRegdumpCvmode()
{
    if (++m_nCurrentCVM >= CUSTOMVIEW_REGS_NUM)
    {
        m_nCurrentCVM = 0;
    }
    CString t;
    t.LoadString(m_cvArray[m_nCurrentCVM].strID);
//    SetDlgItemText(IDC_BUTTON_RDC_CVMODE, t);
    register CMotherBoard *pBoard = m_pDebugger->GetBoard();
    ASSERT(pBoard);

    // если мы в отладочном останове, то
    if (pBoard && pBoard->IsCPUBreaked())
    {
        DisplayRegisters(); // надо вручную обновить регистры
    }
}

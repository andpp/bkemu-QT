#pragma once

#include <QWidget>
#include "Debugger.h"
#include "RegDumpCPUCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CRegDumpDlg dialog

class CDebugger;

constexpr auto CUSTOMVIEW_REGS_NUM = 2;
enum CUSTOMVIEW_MODE : int
{
    CV_DEC_VIEW = 0,
    CV_HEX_VIEW
};

struct CUSTOMVIEW_MODE_PARAM
{
    CUSTOMVIEW_MODE mode;
    UINT strID;
};

class CRegDumpCPUDlg : public QWidget
{
    Q_OBJECT

public:
    explicit CRegDumpCPUDlg(QWidget *parent = nullptr);
    ~CRegDumpCPUDlg();


//    GFResizeDialogHelper m_Resizer;
    CDebugger          *m_pDebugger;
    CRegDumpCPUCtrl    *m_listCPU[9];
    QWidget            *m_pListCPUWidget;
    CRegDumpCPUCtrl    *m_listSys[2][9];
    CRegDumpCPUCtrl    *m_listAltPro[2];
    QWidget            *m_pListSysWidget;
//    CFont               m_hFont;
//    HACCEL              m_hAccelTable;
//    CToolTipCtrl		m_ToolTip;

    static const UINT	m_pListCpuIDs[9];
    static const CCPU::REGISTER	m_pListCpuRegs[9];
    static const UINT	m_pListSysIDs[2][9];
    static const UINT	m_pListSysRegs[2][9];
    static const UINT	m_pListAltProIDs[2];

    int                 m_nCurrentCVM; // индекс текущего режима
    static CUSTOMVIEW_MODE_PARAM m_cvArray[CUSTOMVIEW_REGS_NUM]; // в этом массиве

    int					m_nCurrCPUFreq;
//    CSpinButtonCtrl		m_spinCPUFreq;

    // номера колонок для listCPU
    enum LISTCPU_C : int
    {
        COL_NAME = 0,
        COL_VALUE,
        COL_CUSTOM
    };
    // номера строк для listCPU
    enum LISTCPU_L : int
    {
        LINE_R0 = 0,
        LINE_R1,
        LINE_R2,
        LINE_R3,
        LINE_R4,
        LINE_R5,
        LINE_SP,
        LINE_PC,
        LINE_PSW
    };

    // номера колонок для listSys
    enum LISTSYS_C : int
    {
        COL_NAME1 = 0,
        COL_VALUE1,
        COL_NAME2,
        COL_VALUE2
    };
    // номера строк для listSys, колонки 0,1
    enum LISTSYS_L: int
    {
        LINE_REG177660 = 0,
        LINE_REG177662IN,
        LINE_REG177662OUT,
        LINE_REG177664,
        LINE_REG177714IN,
        LINE_REG177714OUT,
        LINE_REG177716IN,
        LINE_REG177716OUT_TAP,
        LINE_REG177716OUT_MEM
    };
    // номера строк для listSys, колонки 2,3
    enum LISTSYS_L2 : int
    {
        LINE_REG177700 = 0,
        LINE_REG177702,
        LINE_REG177704,
        LINE_REG177706,
        LINE_REG177710,
        LINE_REG177712
    };

    // номера колонок для listAltPro
    enum LISTALTPRO_C : int
    {
        COL_NAMEA = 0,
        COL_VALUEA
    };
    // номера строк для listAltPro
    enum LISTALTPRO_L : int
    {
        LINE_MODE = 0,
        LINE_CODE
    };


//    enum { IDD = IDD_REGDUMP_CPU_DLG };
    enum { IDD = 0 };
//    CSize               GetMinSize();
    void                AttachDebugger(CDebugger *pDbgr);
    void                DisplayFreqParam();
    void                UpdateFreq();
    void                SetPSW(uint16_t value);
    void                DisplayPortRegs();
    void                DisplayRegisters();
    void                DisplayAltProData();
    uint16_t            GetPSW(CString &strPSW);

protected:
    CString             FormatPSW(uint16_t value);
    CString             WordToCustom(uint16_t value);
    uint16_t            CustomToWord(const CString &str);

protected:
    virtual void resizeEvent(QResizeEvent *event);

public slots:
//     virtual BOOL OnInitDialog() override;
//        virtual void OnOK() override;
//        virtual void OnCancel() override;
//    void OnSize(UINT nType, int cx, int cy);
//    void OnLvnItemchangedMdCpuRegisters(NMHDR *pNMHDR, LRESULT *pResult);
//    void OnLvnItemchangedMdAltProData(NMHDR *pNMHDR, LRESULT *pResult);
//    void OnLvnItemchangedMdSysRegisters(NMHDR *pNMHDR, LRESULT *pResult);
    void OnEnChangeEditDbg1Cpufreq();
    void OnBnClickedButtonDbg1CpufreqDec();
    void OnBnClickedButtonDbg1CpufreqInc();
    void OnBnClickedButtonDbg1CpufreqBaseset();
    void OnBnClickedRegdumpCvmode();
};


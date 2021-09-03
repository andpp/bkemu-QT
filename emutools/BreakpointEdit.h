#ifndef CBREAKPOINTEDIT_H
#define CBREAKPOINTEDIT_H

#include <QDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QFrame>
#include <QLineEdit>
#include "BreakPoint.h"
#include "Debugger.h"

class CBreakPointEdit : public QDialog
{
    Q_OBJECT

    UINT               m_type;
    QGroupBox         *m_pTypeButtons;
    QRadioButton      *m_pBtnAddressBP;
    QRadioButton      *m_pBtnMemoryBP;

    QWidget           *m_pWidgetBP;
    QFrame            *m_pFrameAddressBP;
    QFrame            *m_pFrameMemoryBP;

    QLineEdit         *m_pAddrEdit;      // Address edit for Address BP
    QLineEdit         *m_pCondEdit;      // Condition edit for address

    QLineEdit         *m_pAddrEditStart; // Start Address edit for Memory BP
    QLineEdit         *m_pAddrEditEnd;   // Start Address edit for Memory BP
    QCheckBox         *m_pAccessRead;    // Checkbox Read Access for Memory BP
    QCheckBox         *m_pAccessWrite;   // Checkbox Write Access for Memory BP


    uint16_t           m_nAddrStart;
    uint16_t           m_nAddrEnd;
    CString            m_sCond;
    uint16_t           m_nMemAccessFlags;

    CBreakPoint      **m_ppBreakPoint;
    CDebugger         *m_pDebuger;

public:
    CBreakPointEdit(CBreakPoint **bp, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void      SetType(UINT type);
    void      SetAddress(u_int32_t addr)
    {
        m_nAddrStart = addr & 0xFFFF;
        m_nAddrEnd   = (addr >> 16) & 0xFFFF;
    }
    uint32_t  GetAddress() { return m_nAddrStart + ((uint32_t)m_nAddrEnd << 16); }
    void      SetCond(CString &cond) { m_sCond = cond; }
    CString&  GetCond() { return m_sCond; }
    void      SetMemAccessFlags(u_int16_t flags) { m_nMemAccessFlags = flags; }
    u_int16_t GetMemAccessFlags()  { return m_nMemAccessFlags; }

    void      AttachDebugger(CDebugger *dbg) { m_pDebuger = dbg; }

    bool      StrToAddr(CString &str, uint16_t *addr);  // Convert string to addr. String might be ether symbol name or number

public slots:
    void OnAccepted();
    void OnRejected();
};

#endif // CBREAKPOINTEDIT_H

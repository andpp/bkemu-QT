#include "BreakpointEdit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

CBreakPointEdit::CBreakPointEdit(CBreakPoint **bp, QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , m_ppBreakPoint(bp)
  , m_pDebuger(nullptr)
{
    resize(400, 300);

    QVBoxLayout * vLayout = new QVBoxLayout(this);
    this->setLayout(vLayout);

    m_pTypeButtons = new QGroupBox();
    m_pTypeButtons->setFlat(true);
    m_pTypeButtons->setStyleSheet("border:none");
    m_pWidgetBP = new QWidget(this);
    m_pBtnAddressBP = new QRadioButton(tr("Address"));
    m_pBtnMemoryBP  = new QRadioButton(tr("Memory access"));

    QHBoxLayout *hBox = new QHBoxLayout;
    hBox->addWidget(m_pBtnAddressBP);
    hBox->addWidget(m_pBtnMemoryBP);
    m_pTypeButtons->setLayout(hBox);
    m_pTypeButtons->setMaximumHeight(100);

    connect(m_pBtnAddressBP, &QRadioButton::clicked, this, [=]() {SetType(BREAKPOINT_ADDRESS);});
    connect(m_pBtnMemoryBP, &QRadioButton::clicked, this, [=]() {SetType(BREAKPOINT_MEMORY_ACCESS);});

    vLayout->addWidget(m_pTypeButtons);
    vLayout->addWidget(m_pWidgetBP);
    QDialogButtonBox *dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vLayout->addWidget(dialogButtons);

    connect(dialogButtons, &QDialogButtonBox::accepted, this, &CBreakPointEdit::OnAccepted);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &CBreakPointEdit::OnRejected);

    m_pFrameAddressBP = new QFrame(m_pWidgetBP);
    m_pFrameMemoryBP = new QFrame(m_pWidgetBP);

    m_pFrameAddressBP->move(0,0);
    m_pFrameAddressBP->resize(400,200);
    m_pFrameMemoryBP->move(0,0);
    m_pFrameMemoryBP->resize(400,200);

    // Address breakpoint
    QLabel *header = new QLabel("Address", m_pFrameAddressBP);
    header->move(10, 5);
    header->resize(400, 18);
    header = new QLabel("Condition", m_pFrameAddressBP);
    header->move(160, 5);
    header->resize(400, 18);

    m_pAddrEdit = new QLineEdit(m_pFrameAddressBP);
    m_pAddrEdit->move(10, 25);
    m_pAddrEdit->resize(150, 18);

    m_pCondEdit = new QLineEdit(m_pFrameAddressBP);
    m_pCondEdit->move(160, 25);
    m_pCondEdit->resize(190, 18);

    // Address breakpoint
    header = new QLabel("First Address", m_pFrameMemoryBP);
    header->move(10, 5);
    header->resize(400, 18);
    header = new QLabel("Last Address", m_pFrameMemoryBP);
    header->move(160, 5);
    header->resize(400, 18);
    header = new QLabel("R  W", m_pFrameMemoryBP);
    header->move(320, 5);
    header->resize(400, 18);

    m_pAddrEditStart = new QLineEdit(m_pFrameMemoryBP);
    m_pAddrEditStart->move(10, 25);
    m_pAddrEditStart->resize(150, 18);

    m_pAddrEditEnd = new QLineEdit(m_pFrameMemoryBP);
    m_pAddrEditEnd->move(160, 25);
    m_pAddrEditEnd->resize(150, 18);

    m_pAccessRead = new QCheckBox(m_pFrameMemoryBP);
    m_pAccessRead->move(320, 27);

    m_pAccessWrite = new QCheckBox(m_pFrameMemoryBP);
    m_pAccessWrite->move(340, 27);

    if(m_ppBreakPoint && *m_ppBreakPoint) {
        SetType((*m_ppBreakPoint)->GetType());
        switch (m_type) {
            case BREAKPOINT_ADDRESS_COND:
                m_pCondEdit->setText((*(CCondBreakPoint **)m_ppBreakPoint)->GetCond());
                // We don't need 'break' here
            case BREAKPOINT_ADDRESS:
                m_pAddrEdit->setText(::WordToOctString((*m_ppBreakPoint)->GetAddress()));

                break;
            case BREAKPOINT_MEMORY_ACCESS: {
                uint32_t addr = (*m_ppBreakPoint)->GetAddress();
                uint accessType = (*(CMemBreakPoint **)m_ppBreakPoint)->GetAccessType();
                m_pAddrEditStart->setText(::WordToOctString(addr >> 16));
                m_pAddrEditEnd->setText(::WordToOctString(addr & 0xFFFF));
                m_pAccessRead->setChecked(accessType & BREAKPOINT_MEMACCESS_READ);
                m_pAccessWrite->setChecked(accessType & BREAKPOINT_MEMACCESS_WRITE);
                break;
            }
        }
    } else {
        SetType(BREAKPOINT_ADDRESS);
    }

 }

void CBreakPointEdit::SetType(UINT type)
{
    if(type == BREAKPOINT_ADDRESS || type == BREAKPOINT_ADDRESS_COND) {
        m_type = type;
        m_pBtnAddressBP->setChecked(true);
        m_pFrameMemoryBP->hide();
        m_pFrameAddressBP->show();
    } else {
        m_type = BREAKPOINT_MEMORY_ACCESS;
        m_pBtnMemoryBP->setChecked(true);
        m_pFrameAddressBP->hide();
        m_pFrameMemoryBP->show();
    }
}

void CBreakPointEdit::OnAccepted()
{
    if(m_ppBreakPoint) {
        if(m_pBtnAddressBP->isChecked()) {
            uint16_t addr;
            CString sAddr = m_pAddrEdit->text();
            sAddr.Trim();
            if(!StrToAddr(sAddr, &addr)) {
                return;
            }
            CString sCond = m_pCondEdit->text();
            sCond.Trim();
            if (sCond.length() == 0) {
                *m_ppBreakPoint = new CBreakPoint(addr);
            } else {
                CCondBreakPoint *bp = new CCondBreakPoint(m_pDebuger->L, addr);
                bp->SetCond(sCond);
                *m_ppBreakPoint = bp;
            }
        } else {
            uint16_t addrStart;
            CString sAddrStart = m_pAddrEditStart->text();
            sAddrStart.Trim();
            if(!StrToAddr(sAddrStart, &addrStart)) {
                return;
            }
            uint16_t addrEnd;
            CString sAddrEnd = m_pAddrEditEnd->text();
            sAddrEnd.Trim();
            if(!StrToAddr(sAddrEnd, &addrEnd)) {
                return;
            }
            uint accessType = 0;
            if(m_pAccessRead->isChecked())
                accessType |= BREAKPOINT_MEMACCESS_READ;
            if(m_pAccessWrite->isChecked())
                accessType |= BREAKPOINT_MEMACCESS_WRITE;

            CMemBreakPoint *bp = new CMemBreakPoint(addrStart, addrEnd, accessType);
            *m_ppBreakPoint = bp;
        }
    }
    done(1);
}

void CBreakPointEdit::OnRejected()
{
    done(0);
}

bool CBreakPointEdit::StrToAddr(CString &str, uint16_t *addr)
{
    if (m_pDebuger == nullptr)
        return false;

    bool ok;
    uint16_t nAddr = m_pDebuger->m_SymTable.GetAddrForSymbol(str);

    if(nAddr != 0xFFFF) {
        *addr = nAddr;
        return true;
    }
    *addr = str.toUShort(&ok, 0);
    return ok;

}

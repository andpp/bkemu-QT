#include "WatchpointEdit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include "Config.h"
#include "Debugger.h"

CWatchpointEdit::CWatchpointEdit(CWatchPoint *wp, CDebugger *debugger, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , m_pWP(wp)
    , m_pDebugger(debugger)
{

    resize(400, 200);
    QVBoxLayout * vLayout = new QVBoxLayout(this);
    this->setLayout(vLayout);

    QFrame *m_pFrame = new QFrame(this);
    vLayout->addWidget(m_pFrame);
    m_pFrame->move(0,0);
    m_pFrame->resize(400,200);

    QDialogButtonBox *dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vLayout->addWidget(dialogButtons);

    QPushButton* okBtn = dialogButtons->button(QDialogButtonBox::Ok);
    okBtn->setAutoDefault(true);
    okBtn->setDefault(true);

    QPushButton* caBtn = dialogButtons->button(QDialogButtonBox::Cancel);
    caBtn->setAutoDefault(false);
    caBtn->setDefault(false);

    connect(dialogButtons, &QDialogButtonBox::accepted, this, &CWatchpointEdit::OnAccepted);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &CWatchpointEdit::OnRejected);

    // Address breakpoint
    QLabel *header = new QLabel("Address/Symbol", m_pFrame);
    header->move(10, 5);
    header->resize(400, 18);
    header = new QLabel("Length", m_pFrame);
    header->move(190, 5);
    header->resize(400, 18);

    header = new QLabel("Type", m_pFrame);
    header->move(260, 5);
    header->resize(400, 18);


    m_pAddrEdit = new CNumberEdit(CNumberEdit::STRING_EDIT + 24, m_pFrame);
    m_pAddrEdit->setMaximumWidth(160);
    m_pAddrEdit->resize(160, 18);
    m_pAddrEdit->move(10, 25);
    if(m_pWP) {
        if(m_pWP->m_bShowSymbolName && m_pDebugger->m_SymTable.Contains(m_pWP->m_nAddr))
            m_pAddrEdit->setText(m_pDebugger->m_SymTable.GetSymbolForAddr(m_pWP->m_nAddr));
        else
            m_pAddrEdit->setText(::WordToOctString(m_pWP->m_nAddr));
    }

    m_pAddrEdit->setHideOnFocusLost(false);

    m_pLengthEdit = new CNumberEdit(10, m_pFrame);
    m_pLengthEdit->setMinimumWidth(60);
    m_pLengthEdit->resize(60, 18);
    m_pLengthEdit->move(190, 25);
    if(m_pWP)
        m_pLengthEdit->setText(::IntToString(m_pWP->m_nSize));

    m_pLengthEdit->setHideOnFocusLost(false);

    m_pAddrEdit->setFocus();
    m_pAddrEdit->selectAll();

    m_pTypeBox = new QComboBox(this);
    m_pTypeBox->addItem("Words");
    m_pTypeBox->addItem("Bytes");
    m_pTypeBox->addItem("String");
    m_pTypeBox->addItem("Pointer");
    m_pTypeBox->setEditable(false);
    m_pTypeBox->setCurrentIndex(0);

    m_pTypeBox->move(270, 35);

}

void CWatchpointEdit::OnAccepted()
{
    bool ok = false;

    if(m_pWP) {
        uint16_t addr;
        if(m_pDebugger->m_SymTable.Contains(m_pAddrEdit->text())) {
            addr = m_pDebugger->m_SymTable.GetAddrForSymbol(m_pAddrEdit->text());
            ok = true;
        } else {
            addr = m_pAddrEdit->text().toUShort(&ok, 0);
        }
        if(!ok)
            return;
        m_pWP->m_nAddr = addr;

        m_pWP->m_nSize = m_pLengthEdit->text().toUShort(&ok, 0);
        if (m_pWP->m_nSize == 0)
            return;
        m_pWP->m_nType = m_pTypeBox->currentIndex();
    }
    done(1);
}

void CWatchpointEdit::OnRejected()
{
    done(0);
}


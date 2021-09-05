#include "SymbolTableEdit.h"
#include "Config.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>


CSymbolTableEdit::CSymbolTableEdit(uint16_t *addr, CString *name, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , m_pAddr(addr)
    , m_pName(name)
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

    connect(dialogButtons, &QDialogButtonBox::accepted, this, &CSymbolTableEdit::OnAccepted);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &CSymbolTableEdit::OnRejected);


    // Address breakpoint
    QLabel *header = new QLabel("Address", m_pFrame);
    header->move(10, 5);
    header->resize(400, 18);
    header = new QLabel("Name", m_pFrame);
    header->move(160, 5);
    header->resize(400, 18);

    m_pAddrEdit = new CNumberEdit(8, m_pFrame);
    m_pAddrEdit->move(10, 25);
    m_pAddrEdit->resize(100, 18);
    if(*addr != 0xFFFF)
        m_pAddrEdit->setText(::WordToOctString(*addr));

    m_pAddrEdit->setHideOnFocusLost(false);

    m_pNameEdit = new CNumberEdit(CNumberEdit::STRING_EDIT + 24, m_pFrame);
    m_pNameEdit->move(160, 25);
    m_pNameEdit->resize(190, 18);
    m_pNameEdit->setText(*name);
    m_pNameEdit->setHideOnFocusLost(false);

    m_pAddrEdit->setFocus();
}

void CSymbolTableEdit::OnAccepted()
{
    bool ok;

    *m_pAddr = m_pAddrEdit->text().toUShort(&ok, 0);
    if (*m_pAddr == 0xFFFF)
        return;
    *m_pName = m_pNameEdit->text();
    (*m_pName).Trim();
    if ((*m_pName).size() == 0)
        return;
    done(1);
}

void CSymbolTableEdit::OnRejected()
{
    done(0);
}

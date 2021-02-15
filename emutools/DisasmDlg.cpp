//#include <QVBoxLayout>
//#include <QGridLayout>
//#include <QToolBar>
#include <QResizeEvent>

#include "DisasmDlg.h"
#include "Debugger.h"
#include "MainWindow.h"

CDisasmDlg::CDisasmDlg(QWidget *parent) :
    QWidget(parent)
{
    m_ListDisasm = new CDisasmCtrl();
    m_ListDisasm->setParent(this);
    m_ListDisasm->move(0, 7);

    m_EditAddr = new CNumberEdit(8, this);

    m_EditAddr->move(DBG_LINE_ADR_START-9, 0);
    setMinimumWidth(m_ListDisasm->minimumWidth());
    m_EditAddr->hide();

#if 0
     QAction *act;

     QPixmap tbDbgImg(":toolBar/dbg");
     QToolBar *tb = new QToolBar();

     hLayout->setMenuBar(tb);

     act = new QAction(makeIcon(0, tbDbgImg), QString("Стоп"), this);
//     connect(act, &QAction::triggered, this, );
     tb->addAction(act);

     act = new QAction(makeIcon(1, tbDbgImg), QString("Step Down"), this);
     connect(act, &QAction::triggered, this, &CDisasmDlg::OnDisasmStepDn);
     tb->addAction(act);

     act = new QAction(makeIcon(2, tbDbgImg), QString("Step Up"), this);
     connect(act, &QAction::triggered, this, &CDisasmDlg::OnDisasmStepUp);
     tb->addAction(act);

     act = new QAction(makeIcon(3, tbDbgImg), QString("Pg Down"), this);
     connect(act, &QAction::triggered, this, &CDisasmDlg::OnDisasmPgDn);
     tb->addAction(act);

     act = new QAction(makeIcon(4, tbDbgImg), QString("Pg Up"), this);
     connect(act, &QAction::triggered, this, &CDisasmDlg::OnDisasmPgUp);
     tb->addAction(act);

     act = new QAction(makeIcon(5, tbDbgImg), QString("Set Breakpoint"), this);
     connect(act, &QAction::triggered, this, &CDisasmDlg::OnDisasmCheckBp);
     tb->addAction(act);

     tb->addSeparator();
     tb->addWidget(m_EditAddr);
#endif
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmStepDn, this, &CDisasmDlg::OnDisasmStepDn);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmStepUp, this, &CDisasmDlg::OnDisasmStepUp);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmCheckBp, this, &CDisasmDlg::OnDisasmCheckBp);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::ShowAddrEdit, this, &CDisasmDlg::OnShowAddrEdit);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::HideAddrEdit, this, &CDisasmDlg::OnHideAddrEdit);
     QObject::connect(m_EditAddr, &CNumberEdit::AddressUpdated, this, &CDisasmDlg::OnDisasmTopAddressUpdate);

}

CDisasmDlg::~CDisasmDlg()
{
}

void CDisasmDlg::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    size.setHeight(size.height() - 16);
    m_ListDisasm->resize(event->size());

}

void CDisasmDlg::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
    m_ListDisasm->AttachDebugger(pDebugger);
    pDebugger->AttachWnd(this);
    m_EditAddr->setText(::WordToOctString(m_pDebugger->GetCurrentAddress()));
}

//void CDisasmDlg::OnSetFocus()
//{
////	CDialogEx::OnSetFocus(pOldWnd);
////	m_ListDisasm.SetFocus();
//}


void CDisasmDlg::OnDisasmTopAddressUpdate()
{
    CString strBuf = m_EditAddr->text();
    uint16_t nAddr = ::OctStringToWord(strBuf);
    // обновим значение, чтобы оно было всегда 6 значным
    m_EditAddr->setText(::WordToOctString(nAddr));
    m_pDebugger->SetCurrentAddress(nAddr);
}

void CDisasmDlg::OnDisasmCurrentAddressChange(int wp)
{
    m_EditAddr->setText(::WordToOctString(uint16_t(wp)));
    m_ListDisasm->repaint();
}

void CDisasmDlg::OnShowAddrEdit(QPoint &pnt)
{
    m_EditAddr->setText(::WordToOctString(uint16_t(m_pDebugger->GetLineAddress(0))));
    m_EditAddr->show();
    m_EditAddr->selectAll();
    m_EditAddr->setFocus();
}

void CDisasmDlg::OnHideAddrEdit()
{
    m_EditAddr->hide();
}

void CDisasmDlg::OnDisasmStepUp()
{
    uint16_t addr = 0;

    if (m_pDebugger) // сдвигаемся вверх
    {
        m_pDebugger->StepBackward();
        addr = m_pDebugger->GetCurrentAddress();
    }

    m_EditAddr->setText(::WordToOctString(addr));
    m_ListDisasm->repaint();
}

void CDisasmDlg::OnDisasmStepDn()
{
    uint16_t addr = 0;

    if (m_pDebugger) // сдвигаемся вниз
    {
        m_pDebugger->StepForward();
        addr = m_pDebugger->GetCurrentAddress();
    }

    m_EditAddr->setText(::WordToOctString(addr));
    m_ListDisasm->repaint();
}

void CDisasmDlg::OnDisasmPgUp(const int wp)
{
    uint16_t addr = 0;
    auto nCount = static_cast<int>(wp);

    if (m_pDebugger) // сдвигаемся вверх
    {
        while (--nCount > 0)
        {
            m_pDebugger->StepBackward();
        }

        addr = m_pDebugger->GetCurrentAddress();
    }

    m_EditAddr->setText(::WordToOctString(addr));
    m_ListDisasm->repaint();
}

void CDisasmDlg::OnDisasmPgDn(const int wp)
{
    uint16_t addr = 0;
    auto nCount = static_cast<int>(wp);

    if (m_pDebugger) // сдвигаемся вниз
    {
        while (--nCount > 0)
        {
            m_pDebugger->StepForward();
        }

        addr = m_pDebugger->GetCurrentAddress();
    }

    m_EditAddr->setText(::WordToOctString(addr));
    m_ListDisasm->repaint();
}

void CDisasmDlg::OnDisasmCheckBp(const int wp)
{
    if (m_pDebugger)
    {
        uint16_t addr = m_pDebugger->GetLineAddress(static_cast<int>(wp));

        if (!m_pDebugger->SetSimpleBreakpoint(addr))
        {
            m_pDebugger->RemoveBreakpoint(addr);
        }
    }
    m_ListDisasm->repaint();
}


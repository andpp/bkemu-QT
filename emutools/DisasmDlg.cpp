#include "DisasmDlg.h"
#include "Debugger.h"
#include <QVBoxLayout>
#include <QToolBar>
#include "MainWindow.h"

inline static QIcon makeIcon(int i, QPixmap &pm)
{
    return QIcon(pm.copy(i * 16, 0, 16, 16));
}


CDisasmDlg::CDisasmDlg(QWidget *parent) :
    QWidget(parent)
{
    m_ListDisasm = new CDisasmCtrl();

    m_EditAddr = new CEnterEdit(CString("Address"));
    m_EditAddr->setMaximumSize(120,24);
    m_EditAddr->setMinimumSize(120,24);
    m_EditAddr->setAlignment(Qt::AlignRight);
    m_EditAddr->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *hLayout = new QVBoxLayout;
    hLayout->addWidget(m_ListDisasm);
    setLayout(hLayout);


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

}

CDisasmDlg::~CDisasmDlg()
{
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
    CString strBuf = m_EditAddr->toPlainText();
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
            m_pDebugger->RemoveBreakpoint();
        }
    }

}


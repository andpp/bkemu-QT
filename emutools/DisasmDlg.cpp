//#include <QVBoxLayout>
//#include <QGridLayout>
//#include <QToolBar>
#include <QResizeEvent>
#include <QInputDialog>

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

    m_EditAddr->move(m_ListDisasm->m_LineLayout.DBG_LINE_ADR_START-9, 0);
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
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmStepDn,   this, &CDisasmDlg::OnDisasmStepDn);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmStepUp,   this, &CDisasmDlg::OnDisasmStepUp);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmPgDn,     this, &CDisasmDlg::OnDisasmPgDn);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmPgUp,     this, &CDisasmDlg::OnDisasmPgUp);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmDelBP,    this, &CDisasmDlg::OnDisasmDelBp);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::DisasmCheckBp,  this, &CDisasmDlg::OnDisasmCheckBp);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::ShowAddrEdit,   this, &CDisasmDlg::OnShowAddrEdit);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::HideAddrEdit,   this, &CDisasmDlg::OnHideAddrEdit);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::ShowLabelEdit,  this, &CDisasmDlg::OnShowLabelEdit);
     QObject::connect(m_ListDisasm, &CDisasmCtrl::HideLabelEdit,  this, &CDisasmDlg::OnHideLabelEdit);
     QObject::connect(m_EditAddr,   &CNumberEdit::AddressUpdated, this, &CDisasmDlg::OnDisasmTopAddressUpdate);
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
    m_EditAddr->setText(::WordToOctString(g_Config.m_nDisasmAddr));
}

//void CDisasmDlg::OnSetFocus()
//{
////	CDialogEx::OnSetFocus(pOldWnd);
////	m_ListDisasm.SetFocus();
//}


void CDisasmDlg::OnDisasmTopAddressUpdate()
{
    if(m_EditAddr->getBase() & CNumberEdit::STRING_EDIT) {
        // Process label
        CString strName = m_EditAddr->text();
        uint16_t usAddr = m_EditAddr->getMisc();
        if(strName.Trim() == "") {
            // Remove Label
            m_pDebugger->m_SymTable.RemoveSymbol(usAddr);
        } else {
            // Add Label
            m_pDebugger->m_SymTable.AddSymbol(usAddr, strName);
        }
        m_ListDisasm->repaint();
        emit UpdateSymbolTableView();
    } else {
        CString strBuf = m_EditAddr->text();
        uint16_t nAddr = ::OctStringToWord(strBuf);
        // обновим значение, чтобы оно было всегда 6 значным
        m_EditAddr->setText(::WordToOctString(nAddr));
        m_pDebugger->SetCurrentAddress(nAddr);
    }
    m_ListDisasm->setFocus();
}

void CDisasmDlg::OnDisasmCurrentAddressChange(int wp)
{
    m_EditAddr->setText(::WordToOctString(uint16_t(wp)));
    m_ListDisasm->repaint();
}

void CDisasmDlg::OnShowAddrEdit()
{
    m_EditAddr->setBase(8);
    m_EditAddr->setWidth(m_ListDisasm->m_LineLayout.DBG_LINE_ADR_WIDTH + 5);
    m_EditAddr->move(m_ListDisasm->m_LineLayout.DBG_LINE_ADR_START-9, 0);
    m_EditAddr->setAlignment(Qt::AlignRight);
    m_EditAddr->setText(::WordToOctString(uint16_t(m_pDebugger->GetLineAddress(0))));
    m_EditAddr->show();
    m_EditAddr->selectAll();
    m_EditAddr->setFocus();
}

void CDisasmDlg::OnHideAddrEdit()
{
    m_EditAddr->hide();
    m_ListDisasm->setFocus();
}

void CDisasmDlg::OnShowLabelEdit(int nLine)
{
    uint16_t usAddr = m_pDebugger->GetLineAddress(nLine);
    m_EditAddr->setBase(CNumberEdit::STRING_EDIT + 24);
    m_EditAddr->setMisc(usAddr);
    if(m_ListDisasm->m_LineLayout.DBG_LINE_LBL_WIDTH != 0) {
        m_EditAddr->setAlignment(Qt::AlignLeft);
        m_EditAddr->setWidth(m_ListDisasm->m_LineLayout.DBG_LINE_LBL_WIDTH + 5);
        m_EditAddr->move(m_ListDisasm->m_LineLayout.DBG_LINE_LBL_START-3, m_ListDisasm->lineStartPos(nLine)+3);
    } else {
        m_EditAddr->setAlignment(Qt::AlignLeft);
        m_EditAddr->setWidth(m_ListDisasm->m_LineLayout.DBG_LINE_ADR_WIDTH + 5);
        m_EditAddr->move(m_ListDisasm->m_LineLayout.DBG_LINE_ADR_START-9, m_ListDisasm->lineStartPos(nLine)+3);
    }
    m_EditAddr->setText(m_pDebugger->m_SymTable.GetSymbolForAddr(usAddr));
    m_EditAddr->show();
    m_EditAddr->selectAll();
    m_EditAddr->setFocus();
}

void CDisasmDlg::OnHideLabelEdit()
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
    if (nCount == 0)
        nCount = m_ListDisasm->numRowsVisible();

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
    if (nCount == 0)
        nCount = m_ListDisasm->numRowsVisible();

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

void CDisasmDlg::OnDisasmDelBp(const int wp)
{
    if (m_pDebugger)
    {
        uint16_t addr = m_pDebugger->GetLineAddress(static_cast<int>(wp));

        CBreakPoint *bp = nullptr;
        if(m_pDebugger->IsBpeakpointAtAddress(addr, &bp)) {
            m_pDebugger->RemoveBreakpoint(addr);
//            m_ListDisasm->repaint();
        }
    }
}


void CDisasmDlg::OnDisasmCheckBp(const int wp, const bool cond)
{
    if (m_pDebugger)
    {
        uint16_t addr = m_pDebugger->GetLineAddress(static_cast<int>(wp));

        CBreakPoint *bp = nullptr;
        if(m_pDebugger->IsBpeakpointAtAddress(addr, &bp) && !cond) {
            // Remove BP unly by LeftClick
            bp->SetActive(!bp->IsActive());
        } else {
            if(cond) {
                QInputDialog *dialog =new QInputDialog(this, Qt::WindowFlags());
                dialog->setWindowTitle("Breakpoint Condition");
                dialog->setLabelText(nullptr);
                dialog->setTextEchoMode(QLineEdit::Normal);
                dialog->setInputMethodHints(Qt::ImhNone);
                dialog->setMinimumWidth(280);

                if(bp && bp->GetType() == BREAKPOINT_ADDRESS_COND) {
                    // Breakpoind already exist
                    dialog->setTextValue(static_cast<CCondBreakPoint *>(bp)->GetCond());
                } else {
                    dialog->setTextValue("");
                }

                const int ret = dialog->exec();
                if (ret) {
                    if(bp) {
                        // Remove old breakpoint
                        m_pDebugger->RemoveBreakpoint(addr);
                    }
                    QString sCond = dialog->textValue();
                    if(!sCond.isEmpty()) {
                        m_pDebugger->SetConditionalBreakpoint(addr, sCond);
                    } else {
                        m_pDebugger->SetSimpleBreakpoint(addr);
                    }
                } else {
                    return;
                }
            } else {
                m_pDebugger->SetSimpleBreakpoint(addr);
            }
        }
    }
    m_ListDisasm->repaint();
    emit UpdateBreakPointView();
}


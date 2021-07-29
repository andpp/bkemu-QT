#include <QKeyEvent>

#include "DisasmView.h"
#include "MainWindow.h"

extern CMainFrame *g_pMainFrame;

CDisasmView::CDisasmView(QWidget *parent) : QDockWidget(parent)
{
    m_pDisasmDlg = new CDisasmDlg(this);
    setWidget(m_pDisasmDlg);

    QObject::connect(this, &CDisasmView::DebugBreak, g_pMainFrame, &CMainFrame::OnDebugBreak);
    QObject::connect(this, &CDisasmView::DebugStepinto, g_pMainFrame, &CMainFrame::OnDebugStepinto);
    QObject::connect(this, &CDisasmView::DebugStepover, g_pMainFrame, &CMainFrame::OnDebugStepover);
    QObject::connect(this, &CDisasmView::DebugStepout, g_pMainFrame, &CMainFrame::OnDebugStepout);
#ifdef ENABLE_BACKTRACE
    QObject::connect(this, &CDisasmView::DebugStepback, g_pMainFrame, &CMainFrame::OnDebugStepBack);
#endif
}

void CDisasmView::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
    m_pDisasmDlg->AttachDebugger(pDebugger);

}

void CDisasmView::keyPressEvent(QKeyEvent *event)
{

    switch(event->key()) {
        case Qt::Key::Key_F5:
            event->ignore();
            emit DebugBreak();
            break;
#ifdef ENABLE_BACKTRACE
        case Qt::Key::Key_F9:
            event->ignore();
            emit DebugStepback();
            break;
#endif
            case Qt::Key::Key_F10:
            event->ignore();
            emit DebugStepover();
            break;
        case Qt::Key::Key_F11:
            event->ignore();
            emit DebugStepinto();
            break;
        case Qt::Key::Key_F12:
            event->ignore();
            emit DebugStepout();
            break;
        case Qt::Key::Key_PageUp:
            event->ignore();
            m_pDisasmDlg->OnDisasmPgUp();
            break;
        case Qt::Key::Key_PageDown:
            event->ignore();
            m_pDisasmDlg->OnDisasmPgDn();
            break;
        case Qt::Key::Key_Up:
            event->ignore();
            m_pDisasmDlg->OnDisasmStepUp();
            break;
        case Qt::Key::Key_Down:
            event->ignore();
            m_pDisasmDlg->OnDisasmStepDn();
            break;


    }
};



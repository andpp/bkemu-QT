#include "SymbolTableView.h"
#include "Debugger.h"
#include <QWheelEvent>
#include <QMenu>

#include "SymbolTableEdit.h"
#include "BKMessageBox.h"

// массив цветов для подсветки синтаксиса
enum : int
{
    BPCOLOR_TITLE = 0,
    BPCOLOR_ADDR,
    BPCOLOR_COND,
    BPCOLOR_ADR_DISABLED,
    BPCOLOR_COND_DISABLED,
    BPCOLOR_NUM_COLS    // количество цветов
};
static const COLORREF g_crBPColorHighLighting[] =
{
    RGB(0, 0, 0),          // BPCOLOR_TITLE
    RGB(0xff, 0x66, 0),    // BPCOLOR_ADDR
    RGB(0xff, 0x66, 0xcc), // BPCOLOR_COND
    RGB(0x55, 0x55, 0x55), // BPCOLOR_ADR_DISABLED
    RGB(0x55, 0x55, 0xaa), // BPCOLOR_COND_DISABLED
};

CSymbolTableView::CSymbolTableView(QWidget *parent)
    : QDockWidget(parent)
    , m_pDebugger(nullptr)
    , m_pSymTable(nullptr)
    , m_Font(g_FontMonospace)
    , m_nStartIndex(0)
    , m_bSortByAddr(true)
{
    m_Font.setStyleHint(QFont::TypeWriter);
    m_Font.setPixelSize(14);

    QFontMetricsF fm(m_Font);
    m_nlineHeight = fm.height();  // font height

    m_nAddrStart = 10;
    m_nNameStart = m_nAddrStart + int(fm.horizontalAdvance("0000000  "));

    setMinimumSize(m_nNameStart + 150, m_nlineHeight * 10 + winHeaderHight + 5);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(OnShowContextMenu(const QPoint &)));
}

void CSymbolTableView::AttachDebugger( CDebugger *dbg)
{
    m_pDebugger = dbg;
    m_pSymTable = &dbg->m_SymTable;
}


void CSymbolTableView::resizeEvent(QResizeEvent* event)
{
    QDockWidget::resizeEvent(event);
}

void CSymbolTableView::paintEvent(QPaintEvent *event)
{
    QDockWidget::paintEvent(event);

    QPainter  pnt(this);
    int nIndex = -1;

    if(!m_pDebugger)
        return;

    int pos_y = winHeaderHight + m_nlineHeight;
    CString strTxt;
    int nLines = numRowsVisible() - 1;

    pnt.setPen(g_crBPColorHighLighting[BPCOLOR_TITLE]);

    m_Font.setBold(m_bSortByAddr);
    pnt.setFont(m_Font);
    pnt.drawText(m_nAddrStart, winHeaderHight, "Address");

    m_Font.setBold(!m_bSortByAddr);
    pnt.setFont(m_Font);
    pnt.drawText(m_nNameStart, winHeaderHight, "Name");

    m_Font.setBold(false);
    pnt.setFont(m_Font);

    if(m_bSortByAddr) {
        SymTableAddr_t *addrs = m_pSymTable->GetAllAddresses();
        if (m_nStartIndex > (m_pSymTable->GetAllSymbols()->count() - numRowsVisible())) {
            m_nStartIndex = (m_pSymTable->GetAllSymbols()->count() - numRowsVisible());
            if (m_nStartIndex < 0)
                m_nStartIndex = 0;
        }

        for (auto i = addrs->begin(); i != addrs->end(); i++) {
            nIndex++;
            if(nIndex > (nLines + m_nStartIndex))
                break;
            if(nIndex < m_nStartIndex)
                continue;

            pnt.setPen(g_crBPColorHighLighting[BPCOLOR_ADDR]);
            strTxt = ::WordToOctString(i.key());
            pnt.drawText(m_nAddrStart, pos_y, strTxt);

            pnt.setPen(g_crBPColorHighLighting[BPCOLOR_COND]);
            pnt.drawText(m_nNameStart, pos_y, i.value());

            pos_y += m_nlineHeight;
        }
    } else {  // Sort by Symbol names
        SymTable_t *symbols = m_pSymTable->GetAllSymbols();
        if (m_nStartIndex > (m_pSymTable->GetAllSymbols()->count() - numRowsVisible())) {
            m_nStartIndex = (m_pSymTable->GetAllSymbols()->count() - numRowsVisible());
            if (m_nStartIndex < 0)
                m_nStartIndex = 0;
        }

        for (auto i = symbols->begin(); i != symbols->end(); i++) {
            nIndex++;
            if(nIndex > (nLines + m_nStartIndex))
                break;
            if(nIndex < m_nStartIndex)
                continue;

            pnt.setPen(g_crBPColorHighLighting[BPCOLOR_ADDR]);
            strTxt = ::WordToOctString(i.value());
            pnt.drawText(m_nAddrStart, pos_y, strTxt);

            pnt.setPen(g_crBPColorHighLighting[BPCOLOR_COND]);
            pnt.drawText(m_nNameStart, pos_y, i.key());

            pos_y += m_nlineHeight;
        }
    }
}

uint16_t CSymbolTableView::GetAddrByPos(const QPoint &pos)
{
    int idx = (pos.y() - winHeaderHight) / m_nlineHeight + m_nStartIndex;

    if(m_bSortByAddr) {
        SymTableAddr_t *addrs = m_pSymTable->GetAllAddresses();
        auto keys = addrs->keys();

        if (idx < keys.size())
            return keys[idx];
        else
            return (uint16_t)-1;
    } else {
        SymTable_t *symbols = m_pSymTable->GetAllSymbols();
        auto keys = symbols->keys();

        if (idx < keys.size())
            return m_pSymTable->GetAddrForSymbol(keys[idx]);
        else
            return (uint16_t)-1;
    }
}

CString CSymbolTableView::GetNameByPos(const QPoint &pos)
{
    int idx = (pos.y() - winHeaderHight) / m_nlineHeight + m_nStartIndex;

    if(m_bSortByAddr) {
        SymTableAddr_t *addrs = m_pSymTable->GetAllAddresses();
        auto values = addrs->values();

        if (idx < values.size())
            return values[idx];
    } else {
        SymTable_t *symbols = m_pSymTable->GetAllSymbols();
        auto keys = symbols->keys();

        if (idx < keys.size())
            return keys[idx];
    }

    return "";
}


void CSymbolTableView::OnShowContextMenu(const QPoint &pos)
{
   QMenu contextMenu(tr("Breakpoint View menu"), this);

   uint16_t nAddr = GetAddrByPos(pos);
   CString name = GetNameByPos(pos);
   bool isNotEmpty = m_pSymTable->GetAllSymbols()->count() > 0;

   QAction actDel("Remove", this);
   if(nAddr != (uint16_t)-1) {
       connect(&actDel, &QAction::triggered, this, [=](){ OnDeleteSymbol(name); });
       contextMenu.addAction(&actDel);
   }
   QAction actEdit("Edit", this);
   if(nAddr != (uint16_t)-1) {
       connect(&actEdit, &QAction::triggered, this, [=](){ OnEditSymbol(name); });
       contextMenu.addAction(&actEdit);
   }
   QAction actAdd("Add", this);
   connect(&actAdd, &QAction::triggered, this, &CSymbolTableView::OnAddSymbol);
   contextMenu.addAction(&actAdd);
   contextMenu.addSeparator();

   QAction actRemoveAll("Remove All", this);
   connect(&actRemoveAll, &QAction::triggered, this, &CSymbolTableView::OnRemoveAllSymbols);
   actRemoveAll.setEnabled(isNotEmpty);
   contextMenu.addAction(&actRemoveAll);


   contextMenu.exec(mapToGlobal(pos));
}

void CSymbolTableView::OnDeleteSymbol(const CString &name)
{
    m_pSymTable->RemoveSymbol(name);
    emit UpdateDisasmView();
    Update();
}

void CSymbolTableView::OnEditSymbol(const CString &name)
{
    CString sName = name;
    uint16_t nAddr = m_pSymTable->GetAddrForSymbol(sName);

    CSymbolTableEdit m_Dialog(&nAddr, &sName, this);

    if(m_Dialog.exec()) {
        if(sName.size() > 0) {
            m_pSymTable->RemoveSymbol(name);
            m_pSymTable->AddSymbol(nAddr, sName);
        }
    }

    emit UpdateDisasmView();
    update();
}

void CSymbolTableView::OnAddSymbol()
{
    CString sName;
    uint16_t nAddr = 0xFFFF;

    CSymbolTableEdit m_Dialog(&nAddr, &sName, this);

    while(true) {
        if(m_Dialog.exec()) {
            if(sName.size() > 0) {
                if(m_pSymTable->Contains(sName)) {
                    int result = g_BKMsgBox.Show("Symbol '" + sName + "'already exist!\nUpdate symbol?", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
                    if (result == IDYES) {
                        m_pSymTable->RemoveSymbol(sName);
                        m_pSymTable->AddSymbol(nAddr, sName);
                        break;
                    }
                } else {
                    m_pSymTable->AddSymbol(nAddr, sName);
                    break;
                }
            }
        } else {
            break;
        }
    }

    emit UpdateDisasmView();
    update();
}

void CSymbolTableView::OnRemoveAllSymbols()
{
    m_pSymTable->RemoveAllSymbols();
    emit UpdateDisasmView();
    update();
}

void CSymbolTableView::keyPressEvent(QKeyEvent *event)
{
    QDockWidget::keyPressEvent(event);
}

void CSymbolTableView::mousePressEvent(QMouseEvent *event)
{
    Qt::MouseButtons m_Buttons = event->buttons();
    if (m_Buttons & Qt::MouseButton::LeftButton) {
        if(event->pos().y() <= winHeaderHight) {
            if (event->pos().x() > m_nNameStart)
                m_bSortByAddr = false;
            else
                m_bSortByAddr = true;
            update();
        }
    }
}

void CSymbolTableView::mouseDoubleClickEvent(QMouseEvent *event)
{
    (void)event;
}

void CSymbolTableView::wheelEvent(QWheelEvent *event)
{
    QPoint degrees = event->angleDelta() / 8;

    if(degrees.y() == 0) return;

    if(degrees.y() > 0) {
        if (m_nStartIndex) {
            m_nStartIndex--;
        }
    } else {
        if (m_nStartIndex < (m_pSymTable->GetAllSymbols()->count() - numRowsVisible())) {
            m_nStartIndex++;
        }
    }

    repaint();
}

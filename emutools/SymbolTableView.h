#ifndef CSYMBOLTABLEVIEW_H
#define CSYMBOLTABLEVIEW_H

#include <QDockWidget>
#include <QScrollBar>
#include <QScrollEvent>


class CDebugger;
class CSymTable;
class CString;

class CSymbolTableView : public QDockWidget
{
    Q_OBJECT

    CDebugger          * m_pDebugger;
    CSymTable          * m_pSymTable;

    const int winHeaderHight = 35;  // Ofset of first line from window top

    int         numRowsVisible() const {return (height()-winHeaderHight)/m_nlineHeight; }
    int         lineStartPos(const int line) {return line * m_nlineHeight; }

    uint        m_nlineHeight;
    QFont       m_Font;
    QScrollBar *m_pScrollBar;

    int         m_nStartIndex;
    int         m_nAddrStart;
    int         m_nNameStart;

    bool        m_bSortByAddr;

    uint16_t    GetAddrByPos(const QPoint &pos);
    CString     GetNameByPos(const QPoint &pos);

public:
    CSymbolTableView(QWidget *parent = nullptr);

    void AttachDebugger( CDebugger *dbg);
    void Update() { if(isVisible()) update(); }

    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private slots:
    void OnShowContextMenu(const QPoint &pos);
    void OnDeleteSymbol(const CString &name);
    void OnAddSymbol();
    void OnEditSymbol(const CString &name);
    void OnRemoveAllSymbols();

signals:
     void UpdateDisasmView();

};

#endif // CSYMBOLTABLEVIEW_H

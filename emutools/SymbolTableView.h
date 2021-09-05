#ifndef CSYMBOLTABLEVIEW_H
#define CSYMBOLTABLEVIEW_H

#include <QDockWidget>

class CDebugger;
class CSymTable;

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

    int         m_nStartIndex;
    int         m_nAddrStart;
    int         m_nNameStart;

    bool        m_bSortByAddr;

    uint16_t    GetAddrByPos(const QPoint &pos);

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
    void OnDeleteSymbol(uint16_t addr);
    void OnAddSymbol();
    void OnEditSymbol(uint16_t addr);
    void OnRemoveAllSymbols();

signals:
     void UpdateDisasmView();

};

#endif // CSYMBOLTABLEVIEW_H

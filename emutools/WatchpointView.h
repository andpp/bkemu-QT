#ifndef CWATCHPOINTVIEW_H
#define CWATCHPOINTVIEW_H

#include <QDockWidget>
#include <QScrollBar>
#include <QMap>
#include "WatchPoint.h"
#include "NumberEditCtrl.h"


class CDebugger;
class CString;

class CWatchpointView : public QDockWidget
{
    Q_OBJECT
    CDebugger          *m_pDebugger;
    CWatchPointList     m_pWatchpointTable;
    CNumberEdit        *m_pNumberEdit;

    const int winHeaderHight = 35;  // Ofset of first line from window top

    const uint32_t WP_MAGIC = 0xAC3F;

    int           numRowsVisible() const {return (height()-winHeaderHight)/m_nlineHeight; }
    int           lineStartPos(const int line) {return line * m_nlineHeight; }

    uint          m_nlineHeight;
    QFont         m_Font;
    QScrollBar   *m_pScrollBar;

    int           m_nStartIndex;
    int           m_nAddrStart;
    int           m_nValuesStart;

    int           m_nWordWidth;

    int           m_nEditingMode;

    CWatchPoint  *m_pEditingWP;

    void          DrawWatchpointLine(int nLine, CWatchPoint *wp, QPainter& pnt);
    CWatchPoint  *GetWatchpointByPos(const QPoint &pos);

    void          RemoveAllWatchpoints();

public:
    CWatchpointView(QWidget *parent = nullptr);

    void AttachDebugger( CDebugger *dbg);
    void Update() { if(isVisible()) update(); }

    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    bool AddWatchpoint(u_int16_t startAddr, uint16_t size = 2, int type = WPTYPE_WORD);
    bool RemoveWatchpoint(u_int16_t startAddr, uint16_t size = 0, int type = WPTYPE_ANY);
    bool SaveWatchpoints(CString &fname);
    bool LoadWatchpoints(CString &fname);

    void Toggle()
    {
        if(isHidden())
            show();
        else
            hide();
    }



private slots:
    void OnShowContextMenu(const QPoint &pos);
    void OnDeleteWatchpoint(CWatchPoint *wp);
    void OnAddWatchpoint();
    void OnEditWatchpoint(CWatchPoint *wp);
    void OnRemoveAllWatchpoints();
    void OnEditFinished();

signals:
     void UpdateDisasmView();
     void UpdateMemoryView();
};

#endif // CWATCHPOINTVIEW_H

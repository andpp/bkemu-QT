#ifndef CSTACKVIEW_H
#define CSTACKVIEW_H

#include <QDockWidget>

class CDebugger;

class CStackView : public QDockWidget
{
    Q_OBJECT
    CDebugger     *m_pDebugger;
    QFont          m_Font;

    uint32_t m_nlineHeight;  // font height

    uint32_t m_nOctWidth;
    uint32_t m_nASCIIWidth;

    uint32_t m_nAddrStart;
    uint32_t m_nDumpStart;
    uint32_t m_nASCIIStart;


public:
    CStackView(QWidget *pParent);

public:
    void            AttachDebugger(CDebugger *pDebugger)
    {
        m_pDebugger = pDebugger;
    }

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
};

#endif // CSTACKVIEW_H

#pragma once

#include <QPainter>
#include <QTimer>

#include "Debugger.h"
#include "NumberEditCtrl.h"

class CDebugger;

class CRegDumpCPUCtrl : public QWidget
{
    Q_OBJECT

public:
    CRegDumpCPUCtrl(uint, CString &regName, QWidget *parent = nullptr);
    virtual ~CRegDumpCPUCtrl() {};

    void        AttachDebugger(CDebugger *pDebugger);
    uint16_t    GetValue() const { return m_nValue; }
    void        SetValue(uint16_t val) { m_nValue = val; }
    void        SetTextValue(const CString &val) { m_bIsTextValue = true; m_sTextValue = val; }

private:

    CDebugger *m_pDebugger;
    uint       m_nlineHeight;
    uint16_t   m_nValue;
    uint       m_nRegCode;
    CString    m_sRegName;
    CString    m_sTextValue;
    bool       m_bReadOnly;
    CNumberEdit *m_pNumberEdit;
    bool       m_bIsTextValue;
    uint       m_nBase;

    QFont m_Font;
    QTimer   m_nTimer;

    uint m_nREG_WIDTH;
    uint m_nOCT_WIDTH;
    uint m_nMIST_WIDTH;


    void changeBase() {
        // Change base
        m_nBase = (m_nBase != 10) ? 10 : 16;
        repaint();
    }

//    inline void         StartTimer()
//    {
//        m_nTimer = startTimer(20);  // запустить таймер для DoubleClick
//    }
//    inline void         StopTimer()
//    {
//        killTimer(m_nTimer);         // остановить таймер для DoubleClick
//    }

//    inline void timerEvent(QTimerEvent *event) override {
//        if (event->timerId() == m_nTimer)
//            ;
//    }

signals:

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void onEditFinished();

};

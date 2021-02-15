#pragma once

#include <QPainter>
#include <QTimer>

#include "Debugger.h"
#include "NumberEditCtrl.h"

class CDebugger;

enum : int {
    REG_TYPE_RON = 0,
    REG_TYPE_SYS,
    REG_TYPE_ALTPRO
};

class CRegDumpCPUCtrl : public QWidget
{
    Q_OBJECT

public:
    CRegDumpCPUCtrl(uint regID, CString &regName, uint regNameWidth = 0, QWidget *parent = nullptr);
    virtual ~CRegDumpCPUCtrl() {};

    void        AttachDebugger(CDebugger *pDebugger);
    uint16_t    GetValue() const { return m_nValue; }
    void        SetValue(uint16_t val) { m_nValue = val; }
    void        SetTextValue(const CString &val) { m_bIsTextValue = true; m_sTextValue = val; }
    void        SetReadOnly(bool ro) { m_bReadOnly = ro; }
    void        SetRegType(uint regType) { m_nRegType = regType; }

private:

    CDebugger *m_pDebugger;
    uint       m_nlineHeight;
    uint16_t   m_nValue;
    uint       m_nRegCode;
    CString    m_sRegName;
    uint       m_nRegType;
    CString    m_sTextValue;
    bool       m_bReadOnly;
    CNumberEdit *m_pNumberEdit;
    bool       m_bIsTextValue;
    int        m_nBase;

    QFont m_Font;
    QTimer   m_nTimer;

    int m_nNameStart;
    int m_nNameWidth;
    int m_nOctStart;
    int m_nOctWidth;
    int m_nMiscStart;
    int m_nMiscWidth;



    void changeBase() {
        // Change base
        switch(m_nBase) {
            case 10:
                m_nBase = -10;
                break;
            case -10:
                m_nBase = 16;
                break;
            case 16:
            default:
                m_nBase = 10;
                break;
        }
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

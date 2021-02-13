#pragma once

#include <QPainter>

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


signals:

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

};

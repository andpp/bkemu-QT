#ifndef CWATCHPOINTEDIT_H
#define CWATCHPOINTEDIT_H

#include <QDialog>
#include <QComboBox>
#include "NumberEditCtrl.h"
#include "WatchPoint.h"

class CDebugger;

class CWatchpointEdit : public QDialog
{
    Q_OBJECT

    CNumberEdit     *m_pAddrEdit;      // Address edit for Address BP
    CNumberEdit     *m_pLengthEdit;
    QComboBox       *m_pTypeBox;
    CWatchPoint     *m_pWP;
    CDebugger       *m_pDebugger;

public:
    CWatchpointEdit(CWatchPoint *wp, CDebugger *debugger, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

public slots:
    void OnAccepted();
    void OnRejected();
};

#endif // CWATCHPOINTEDIT_H

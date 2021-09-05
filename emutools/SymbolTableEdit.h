#ifndef CSYMBOLTABLEEDIT_H
#define CSYMBOLTABLEEDIT_H

#include <QDialog>
#include "NumberEditCtrl.h"

class CSymbolTableEdit : public QDialog
{
    Q_OBJECT

    CNumberEdit     *m_pAddrEdit;      // Address edit for Address BP
    CNumberEdit     *m_pNameEdit;      // Condition edit for address

    uint16_t        *m_pAddr;
    CString         *m_pName;

public:
    CSymbolTableEdit(uint16_t *addr, CString *name, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

public slots:
    void OnAccepted();
    void OnRejected();

};

#endif // CSYMBOLTABLEEDIT_H

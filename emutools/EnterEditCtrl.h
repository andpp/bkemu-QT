#ifndef CENTEREDIT_H
#define CENTEREDIT_H

#pragma once

#include "pch.h"
#include <QLineEdit>

class CEnterEdit : public QLineEdit
{
    Q_OBJECT

public:
    CEnterEdit(const CString str, QWidget *parent = nullptr);
    virtual ~CEnterEdit() {};

protected:
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

signals:
    void AddressUpdated();


};

#endif // CENTEREDIT_H

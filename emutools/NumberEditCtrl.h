#ifndef CENTEREDIT_H
#define CENTEREDIT_H

#pragma once

#include "pch.h"
#include <QLineEdit>

class CNumberEdit : public QLineEdit
{
    Q_OBJECT

public:
    CNumberEdit(const int base, QWidget *parent = nullptr);
    virtual ~CNumberEdit() {};
    void setBase(int base);
    int getValue() {return text().toInt(nullptr, m_nBase); }
    void setSize(const int w, const int h) {
        setMaximumSize(w, h);
        setMinimumSize(w, h);
    }

protected:
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void focusOutEvent(QFocusEvent* event) Q_DECL_OVERRIDE;

private:
    int m_nBase;

signals:
    void AddressUpdated();


};

#endif // CENTEREDIT_H

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
    void    setBase(int base);
    int     getBase() {return m_nBase; }
    void    setMisc(int val) {m_nMisc = val; }
    int     getMisc()  { return m_nMisc; }
    int     getValue() {return text().toInt(nullptr, abs(m_nBase)); }
    void    setSize(const int w, const int h) {
        setMaximumSize(w, h);
        setMinimumSize(w, h);
    }
    void    setWidth(const int w) {
        setMaximumWidth(w);
        setMinimumWidth(w);
    }

    static constexpr uint STRING_LEN_MASK = 0x3f;
    static constexpr uint STRING_EDIT = 0x1000;

protected:
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void focusOutEvent(QFocusEvent* event) Q_DECL_OVERRIDE;

private:
    int m_nBase;
    int m_nMisc;

signals:
    void AddressUpdated();


};

#endif // CENTEREDIT_H

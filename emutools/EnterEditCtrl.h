#ifndef CENTEREDIT_H
#define CENTEREDIT_H

#include "pch.h"
#include <QTextEdit>

class CEnterEdit : public QTextEdit
{
    Q_OBJECT

public:
    CEnterEdit(const CString str);
    virtual ~CEnterEdit() {};
};

#endif // CENTEREDIT_H

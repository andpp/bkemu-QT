#include "NumberEditCtrl.h"
#include <QKeyEvent>
#include <QRegExpValidator>
#include "Debugger.h"

CNumberEdit::CNumberEdit(const int base, QWidget *parent) : QLineEdit(parent)
{
   QValidator *ipValidator = nullptr;
   setStyleSheet("border-width: 2px;");
   setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
   switch (base) {
       case 8:
           setMaxLength(7);
           ipValidator = new QRegExpValidator(QRegExp("0?[0-1]?[0-7]?[0-7]?[0-7]?[0-7]?[0-7]?"), this);
           break;
       case 10:
           setMaxLength(5);
           ipValidator = new QIntValidator(0, USHRT_MAX, this);
           break;
       case -10:
           setMaxLength(6);
           ipValidator = new QIntValidator(SHRT_MIN, SHRT_MAX, this);
           break;
       case 16:
           setMaxLength(4);
           ipValidator = new QRegExpValidator(QRegExp("[0-9A-Fa-f]?[0-9A-Fa-f]?[0-9A-Fa-f]?[0-9A-Fa-f]?"), this);
           break;
   }

   setValidator(ipValidator);
   setMaximumSize(DBG_LINE_INS_START - DBG_LINE_ADR_START + 5, 20);
   setMinimumSize(DBG_LINE_INS_START - DBG_LINE_ADR_START + 5, 20);
   setAlignment(Qt::AlignRight);
}

void CNumberEdit::setBase(int base)
{
    delete validator();

    QValidator *ipValidator = nullptr;
    switch (base) {
        case 8:
            setMaxLength(7);
            ipValidator = new QRegExpValidator(QRegExp("0?[0-1]?[0-7]?[0-7]?[0-7]?[0-7]?[0-7]?"), this);
            break;
        case 10:
            setMaxLength(5);
            ipValidator = new QIntValidator(0, USHRT_MAX, this);
            break;
        case -10:
            setMaxLength(6);
            ipValidator = new QIntValidator(SHRT_MIN, SHRT_MAX, this);
            break;
        case 16:
            setMaxLength(4);
            ipValidator = new QRegExpValidator(QRegExp("[0-9A-Fa-f]?[0-9A-Fa-f]?[0-9A-Fa-f]?[0-9A-Fa-f]?"), this);
            break;
    }

    setValidator(ipValidator);
}

void CNumberEdit::focusOutEvent(QFocusEvent* event)
{
    (void)event;
    hide();
}


void CNumberEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Return)
    {
        hide();
        event->ignore();
        emit AddressUpdated();
        return;
    }

    if (event->key() == Qt::Key::Key_Escape)
    {
        hide();
        event->ignore();
        return;
    }


    if (event->key() == Qt::Key::Key_8 || event->key() == Qt::Key::Key_9) {
        event->ignore();
        return;
    }

    QLineEdit::keyPressEvent(event);
}

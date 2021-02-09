#include "EnterEditCtrl.h"
#include <QKeyEvent>
#include <QRegExpValidator>
#include "Debugger.h"

CEnterEdit::CEnterEdit(const CString str, QWidget *parent) : QLineEdit(parent)
{
   setMaxLength(7);
   setStyleSheet("border-width: 2px;");
   setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
   QRegExpValidator *ipValidator = new QRegExpValidator(QRegExp("0?[0-1]?[0-7]?[0-7]?[0-7]?[0-7]?[0-7]?"), this);
   setValidator(ipValidator);
   setMaximumSize(DBG_LINE_INS_START - DBG_LINE_ADR_START + 5, 20);
   setMinimumSize(DBG_LINE_INS_START - DBG_LINE_ADR_START + 5, 20);
   setAlignment(Qt::AlignRight);

}

void CEnterEdit::keyPressEvent(QKeyEvent *event)
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

#include "NumberEditCtrl.h"
#include <QKeyEvent>
#include <QRegExpValidator>
#include "Debugger.h"

CNumberEdit::CNumberEdit(const int base, QWidget *parent)
    : QLineEdit(parent)
    , m_bHideOnFocusLost(true)
{
   setStyleSheet("border-width: 2px;");
   setMaximumSize(68 + 5, 20);
   setMinimumSize(68 + 5, 20);
   setAlignment(Qt::AlignRight);
   setBase(base);
}

void CNumberEdit::setBase(int base)
{
    QValidator * m_pValidator;

    if (validator())
        delete validator();

    m_nBase = base;
    switch (base) {
        case 8:
            setMaxLength(7);
            m_pValidator = new QRegExpValidator(QRegExp("0?[0-1]?[0-7]?[0-7]?[0-7]?[0-7]?[0-7]?"), this);
            break;
        case -8:
            setMaxLength(3);
            m_pValidator = new QRegExpValidator(QRegExp("[0-3]?[0-7]?[0-7]"), this);
            break;
        case 10:
            setMaxLength(5);
            m_pValidator = new QIntValidator(0, USHRT_MAX, this);
            break;
        case -10:
            setMaxLength(6);
            m_pValidator = new QIntValidator(SHRT_MIN, SHRT_MAX, this);
            break;
        case 16:
            setMaxLength(4);
            m_pValidator = new QRegExpValidator(QRegExp("[0-9A-Fa-f]?[0-9A-Fa-f]?[0-9A-Fa-f]?[0-9A-Fa-f]?"), this);
            break;
        default:
            if(base < 0) {                                // No restrictions for negative base
                setMaxLength(-base & STRING_LEN_MASK);
                m_pValidator = new QRegExpValidator(QRegExp(".*"), this);
            } else {                                      // Set symbol name restrictions
                setMaxLength(base & STRING_LEN_MASK);
                m_pValidator = new QRegExpValidator(QRegExp("[0-9_A-Za-z$]*"), this);
            }
    }

    setValidator(m_pValidator);
}

void CNumberEdit::focusOutEvent(QFocusEvent* event)
{
    (void)event;
    if (m_bHideOnFocusLost)
        hide();
}


void CNumberEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Return)
    {
        if (m_bHideOnFocusLost)
            hide();
        event->ignore();
        emit AddressUpdated();
        return;
    }

    if (event->key() == Qt::Key::Key_Escape)
    {
        if (m_bHideOnFocusLost)
            hide();
        event->ignore();
        return;
    }

    QLineEdit::keyPressEvent(event);
}

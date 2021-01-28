#ifndef DISASMDLG_H
#define DISASMDLG_H

#include <QDockWidget>

//namespace Ui {
//class CDisasmDlg;
//}

class CDisasmDlg : public QWidget
{
    Q_OBJECT

public:
    explicit CDisasmDlg(QWidget *parent = nullptr);
    ~CDisasmDlg();

//private:
//    Ui::CDisasmDlg *ui;
};

#endif // DISASMDLG_H

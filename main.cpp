#include "MainWindow.h"

#include <QApplication>

CMainFrame *g_pMainFrame;

int main(int argc, char *argv[])
{
    bool bRes;

    QApplication a(argc, argv);
    g_pMainFrame = new CMainFrame();
    g_pMainFrame->show();
    bRes = a.exec();
    delete g_pMainFrame;
    return bRes;
}

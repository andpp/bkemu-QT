#include "MainWindow.h"
#include "ui_mainwindow.h"

CMainFrame::CMainFrame(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

CMainFrame::~CMainFrame()
{
    delete ui;
}


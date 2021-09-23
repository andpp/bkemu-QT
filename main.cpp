#include "MainWindow.h"

#include <QApplication>
#include <QProxyStyle>
#include <QFontDatabase>
#include "res.h"

CMainFrame *g_pMainFrame;
QString g_FontMonospace;
CResourceStrings g_ResourceStrings;

class MenuStyle : public QProxyStyle
{
public:
    int styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget, QStyleHintReturn *returnData) const
    {
        if (stylehint == QStyle::SH_MenuBar_AltKeyNavigation)
            return 0;
        if (stylehint == QStyle::SH_UnderlineShortcut)
            return 0;

        return QProxyStyle::styleHint(stylehint, opt, widget, returnData);
    }
};

int main(int argc, char *argv[])
{
    bool bRes;

    QApplication a(argc, argv);
    a.setStyle(new MenuStyle()); // Prevent menubar from grabbing focus after Alt pressed
    QApplication::setDoubleClickInterval(180);
    int id = QFontDatabase::addApplicationFont(":fonts/mono-regular");
    g_FontMonospace = QFontDatabase::applicationFontFamilies(id).at(0);

    // Uncomment this to use system monospace font
    // g_FontMonospace = "Monospace";

    g_pMainFrame = new CMainFrame();
    g_pMainFrame->show();
    bRes = a.exec();
    delete g_pMainFrame;
    return bRes;
}

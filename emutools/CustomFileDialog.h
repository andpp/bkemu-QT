#ifndef CBINFILEDIALOG_H
#define CBINFILEDIALOG_H

#include <QFileDialog>

QString getSaveFileName(QWidget *parent,
                                 const QString &caption,
                                 const QString &dir,
                                 const QString &filter,
                                 const QString &suffix,
                                 QString *selectedFilter,
                                 QFileDialog::Options options);

class CNumberEdit;

class CBinFileDialog : public QFileDialog
{
    Q_OBJECT

    CNumberEdit *m_EditStartAddr;
    CNumberEdit *m_EditLength;
    CNumberEdit *m_EditOffset;

    uint        m_nBase;

public:
    CBinFileDialog(QWidget *parent, const QString &caption,
                                    const QString &directory,
                                    const QString &filter,
                                    const bool modeOpen = true);
    ~CBinFileDialog();

    uint16_t GetStartAddr();
    uint16_t GetLength();
    uint16_t GetOffset();
    inline QString GetFileName() { return selectedUrls().value(0).toLocalFile(); }

private slots:
    void OnChange(const QString &file);
};

#endif // CBINFILEDIALOG_H

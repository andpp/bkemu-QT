#include "CustomFileDialog.h"
#include "Config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include "NumberEditCtrl.h"
#include "pch.h"

QString getSaveFileName(QWidget *parent,
                                 const QString &caption,
                                 const QString &dir,
                                 const QString &filter,
                                 const QString &suffix,
                                 QString *selectedFilter,
                                 QFileDialog::Options options)
{
    const QStringList schemes = QStringList(QStringLiteral("file"));

     QFileDialog dialog(parent, caption, dir, filter);
     dialog.setDefaultSuffix(suffix);
     dialog.setOptions(options);
     dialog.setSupportedSchemes(schemes);
     dialog.setAcceptMode(QFileDialog::AcceptSave);
     if (selectedFilter && !selectedFilter->isEmpty())
         dialog.selectNameFilter(*selectedFilter);
     if (dialog.exec() == QDialog::Accepted) {
         if (selectedFilter)
             *selectedFilter = dialog.selectedNameFilter();
         return dialog.selectedUrls().value(0).toLocalFile();
     }
     return "";
}

CBinFileDialog::CBinFileDialog(QWidget *parent, const QString &caption,
                               const QString &directory,
                               const QString &filter,
                               const bool modeOpen)
  : QFileDialog(parent, caption, directory, filter)
  , m_nBase(8)
{
    setOption(QFileDialog::DontUseNativeDialog, true);
    setSupportedSchemes(QStringList("file"));
    if(modeOpen)
        setFileMode(QFileDialog::ExistingFiles);
    else
        setFileMode(QFileDialog::AnyFile);

    QVBoxLayout *box = new QVBoxLayout;

    QWidget *editWidget = new QWidget(this);
    QGridLayout *editLayout = new QGridLayout();
    editLayout->setHorizontalSpacing(15);
    editLayout->setVerticalSpacing(1);


    editWidget->setLayout(editLayout);

    QLabel *labelStartAddr = new QLabel("Start");
    m_EditStartAddr = new CNumberEdit(8, editWidget);
    m_EditStartAddr->setWidth(80);
    m_EditStartAddr->setHideOnFocusLost(false);
    editLayout->addWidget(labelStartAddr,0,1, Qt::AlignmentFlag::AlignLeft);
    editLayout->addWidget(m_EditStartAddr,1,1, Qt::AlignmentFlag::AlignLeft);

    QLabel *labelLength = new QLabel("Length");
    m_EditLength = new CNumberEdit(8, editWidget);
    m_EditLength->setWidth(80);
    m_EditLength->setHideOnFocusLost(false);
    editLayout->addWidget(labelLength,0,2, Qt::AlignmentFlag::AlignLeft);
    editLayout->addWidget(m_EditLength,1,2, Qt::AlignmentFlag::AlignLeft);

    QLabel *labelOffset = new QLabel("Offset");
    m_EditOffset = new CNumberEdit(8, editWidget);
    m_EditOffset->setWidth(80);
    m_EditOffset->setHideOnFocusLost(false);
    editLayout->addWidget(labelOffset,0,3, Qt::AlignmentFlag::AlignLeft);
    editLayout->addWidget(m_EditOffset,1,3, Qt::AlignmentFlag::AlignLeft);

    if(!modeOpen) {
        labelOffset->hide();
        m_EditOffset->hide();
    }

    editLayout->setColumnStretch(4,100);

    box->addWidget(editWidget);

    QWidget *fileDialog = new QWidget(this);
    fileDialog->setLayout(layout());
    fileDialog->resize(700,500);
    box->addWidget(fileDialog);

    setLayout(box);

    connect(this, &QFileDialog::currentChanged, this, &CBinFileDialog::OnChange);
}

CBinFileDialog::~CBinFileDialog()
  = default;

void CBinFileDialog::OnChange(const QString &file)
{
    QFileInfo fInfo(file);

    if(fInfo.isDir() || !fInfo.isReadable()) {
        m_EditStartAddr->setText("");
        m_EditLength->setText("");
        return;
    }

    CFile f;
    if (f.Open(file, CFile::modeRead)) {

        struct {
            uint16_t startAddr;
            uint16_t length;
        } header;

        f.Read(&header, 4);

        if(f.GetLength() == header.length + 4) {
            m_EditStartAddr->setText(::WordToOctString(header.startAddr));
            m_EditLength->setText(::WordToOctString(header.length));
            m_EditOffset->setText("4");
        } else {
            // Not standard binary file
            m_EditStartAddr->setText("");
            m_EditLength->setText(::WordToOctString(f.GetLength()));
            m_EditOffset->setText("0");
        }
    }

    f.Close();
}

uint16_t CBinFileDialog::GetStartAddr()
{
    bool ok;
    return m_EditStartAddr->text().toInt(&ok, m_nBase);
}

uint16_t CBinFileDialog::GetLength()
{
    bool ok;
    return m_EditLength->text().toInt(&ok, m_nBase);

}

uint16_t CBinFileDialog::GetOffset()
{
    bool ok;
    return m_EditOffset->text().toInt(&ok, m_nBase);
}


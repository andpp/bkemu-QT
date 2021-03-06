// AYVolPan.cpp: файл реализации
//

#include "pch.h"
//#include "BK.h"
#include "BKAYVolPan.h"
//#include "afxdialogex.h"

#include <QSignalMapper>
#include <QDialogButtonBox>

// Диалоговое окно CAYVolPan

//IMPLEMENT_DYNAMIC(CBKAYVolPan, CDialogEx)

constexpr auto HSliderStart = 45;
constexpr auto HSliderWidth = 150;
constexpr auto HLabelStart = 5;
constexpr auto HSliderTop = 30;
constexpr auto HSliderHeight = 20;
constexpr auto HSliderInterval = 25;

constexpr auto VSliderStart = HSliderStart + HSliderWidth + (HSliderStart - HLabelStart) + 20;
constexpr auto VSliderWidth = 20;
constexpr auto VLabelStart = 10;
constexpr auto VSliderTop = 50;
constexpr auto VSliderHeight = 165;
constexpr auto VSliderInterval = 25;

constexpr auto HSliderStyle =
    "QSlider::groove:horizontal {"
        "border: 1px solid #999999;"
        "height: 6px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
        "margin: 2px 0;"
    "}"
    "QSlider::handle:horizontal {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
        "border: 1px solid #5c5c5c;"
        "width: 18px;"
        "margin: -4px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */"
        "border-radius: 3px;"
    "}";

constexpr auto VSliderStyle =
        "QSlider::groove:vertical {"
            "border: 1px solid #999999;"
            "width: 4px;"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #B1B1B1, stop:1 #c4c4c4);"
            "margin: 0px 0px;"
            "border-radius: 3px;"
        "}"
        "QSlider::handle:vertical {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
            "border: 1px solid #5c5c5c;"
            "height: 10px;"
            "margin: 0 -6px; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */"
            "border-radius: 3px;"
        "}"
        "QSlider::add-page:vertical {"
            "width: 4px;"
            "border: 1px solid #999999;"
            "background: blue;"
            "border-radius: 3px;"
        "}"
//        "QSlider::sub-page:vertical {"
//            "background: white;"
//        "}"
;

CBKAYVolPan::CBKAYVolPan(QWidget *pParent)
    : QDialog(pParent)
	, m_nVolA(0)
	, m_nVolB(0)
	, m_nVolC(0)
	, m_nPanAL(0)
	, m_nPanBL(0)
	, m_nPanCL(0)
	, m_nPanAR(0)
	, m_nPanBR(0)
	, m_nPanCR(0)
{
	ZeroMemory(&m_orig, sizeof(CConfig::AYVolPan_s));
	ZeroMemory(&m_curr, sizeof(CConfig::AYVolPan_s));

    setWindowTitle("Параметры AY8910");
    setFixedSize(400, 300);

    OnInitDialog();

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonBox->move(210,260);

    m_ctrPanA.resize(HSliderWidth, HSliderHeight);
    m_ctrPanA.move(HSliderStart, HSliderTop + (HSliderInterval-HSliderHeight)/2 + 3);
    m_ctrPanA.setStyleSheet(HSliderStyle);
    m_ctrPanA.setTickPosition(QSlider::TicksBelow);
    m_ctrPanA.setInvertedAppearance(true);
    m_ctrPanA.setFocusPolicy(Qt::ClickFocus);

    m_ctrPanB.resize(HSliderWidth, HSliderHeight);
    m_ctrPanB.move(HSliderStart, HSliderTop + (HSliderInterval-HSliderHeight)/2 + 3 + HSliderInterval);
    m_ctrPanB.setStyleSheet(HSliderStyle);
    m_ctrPanB.setTickPosition(QSlider::TicksBelow);
    m_ctrPanB.setInvertedAppearance(true);
    m_ctrPanB.setFocusPolicy(Qt::ClickFocus);

    m_ctrPanC.resize(HSliderWidth, HSliderHeight);
    m_ctrPanC.move(HSliderStart, HSliderTop + (HSliderInterval-HSliderHeight)/2 + 3 + HSliderInterval * 2);
    m_ctrPanC.setStyleSheet(HSliderStyle);
    m_ctrPanC.setTickPosition(QSlider::TicksBelow);
    m_ctrPanC.setInvertedAppearance(true);
    m_ctrPanC.setFocusPolicy(Qt::ClickFocus);


    m_ctrVolA.move(VSliderStart, VSliderTop);
    m_ctrVolA.resize(VSliderWidth, VSliderHeight);
    m_ctrVolA.setStyleSheet(VSliderStyle);
    m_ctrVolA.setTickPosition(QSlider::TicksBothSides);
    m_ctrVolA.setFocusPolicy(Qt::ClickFocus);

    m_ctrVolB.move(VSliderStart + VSliderInterval, VSliderTop);
    m_ctrVolB.resize(VSliderWidth, VSliderHeight);
    m_ctrVolB.setTickPosition(QSlider::TicksBothSides);
    m_ctrVolB.setStyleSheet(VSliderStyle);
    m_ctrVolB.setFocusPolicy(Qt::ClickFocus);

    m_ctrVolC.move(VSliderStart  + VSliderInterval * 2, VSliderTop);
    m_ctrVolC.resize(VSliderWidth, VSliderHeight);
    m_ctrVolC.setTickPosition(QSlider::TicksBothSides);
    m_ctrVolC.setStyleSheet(VSliderStyle);
    m_ctrVolC.setFocusPolicy(Qt::ClickFocus);

    QLabel *l_ctrPan  = new QLabel("Balance", this);
    l_ctrPan->resize(HSliderWidth, HSliderHeight);
    l_ctrPan->move(HSliderStart, HSliderTop-20);
    l_ctrPan->setAlignment(Qt::AlignHCenter | Qt::AlignTop);


    QLabel *l_ctrPanAL = new QLabel(QString::number(m_nPanAL), this);
    l_ctrPanAL->resize(HSliderStart - HLabelStart-5, HSliderInterval);
    l_ctrPanAL->move(HLabelStart,HSliderTop);
    l_ctrPanAL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel *l_ctrPanAR = new QLabel(QString::number(m_nPanAR), this);
    l_ctrPanAR->resize(HSliderStart - HLabelStart-10, HSliderInterval);
    l_ctrPanAR->move(HSliderStart + HSliderWidth, HSliderTop );
    l_ctrPanAR->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *l_ctrPanBL = new QLabel(QString::number(m_nPanBL), this);
    l_ctrPanBL->resize(HSliderStart - HLabelStart-5, HSliderInterval);
    l_ctrPanBL->move(HLabelStart, HSliderTop + HSliderInterval);
    l_ctrPanBL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel *l_ctrPanBR = new QLabel(QString::number(m_nPanBR), this);
    l_ctrPanBR->resize(HSliderStart - HLabelStart-10, HSliderInterval);
    l_ctrPanBR->move(HSliderStart + HSliderWidth, HSliderTop + HSliderInterval);
    l_ctrPanBR->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *l_ctrPanCL = new QLabel(QString::number(m_nPanCL), this);
    l_ctrPanCL->resize(HSliderStart - HLabelStart-5, HSliderInterval);
    l_ctrPanCL->move(HLabelStart, HSliderTop + HSliderInterval * 2);
    l_ctrPanCL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel *l_ctrPanCR = new QLabel(QString::number(m_nPanCR), this);
    l_ctrPanCR->resize(HSliderStart - HLabelStart-10, HSliderInterval);
    l_ctrPanCR->move(HSliderStart + HSliderWidth, HSliderTop + HSliderInterval * 2);
    l_ctrPanCR->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *l_ctrVol  = new QLabel("Volume\nA     B     C", this);
    l_ctrVol->move(VSliderStart,HSliderTop - 20);
    l_ctrVol->resize(VSliderInterval * 3, 40);
    l_ctrVol->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *l_ctrVolA = new QLabel(QString::number(m_nVolA), this);
    l_ctrVolA->resize(HSliderStart - HLabelStart, HSliderInterval);
    l_ctrVolA->move(HLabelStart,HSliderTop + HSliderInterval * 3);
    l_ctrVolA->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel *l_ctrVolB = new QLabel(QString::number(m_nVolB), this);
    l_ctrVolB->resize(HSliderStart - HLabelStart, HSliderInterval);
    l_ctrVolB->move(HLabelStart,HSliderTop + HSliderInterval * 4);
    l_ctrVolB->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel *l_ctrVolC = new QLabel(QString::number(m_nVolC), this);
    l_ctrVolC->resize(HSliderStart - HLabelStart, HSliderInterval);
    l_ctrVolC->move(HLabelStart,HSliderTop + HSliderInterval * 5);
    l_ctrVolC->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    l_ctrVolA->hide();
    l_ctrVolB->hide();
    l_ctrVolC->hide();

    m_DataLabel.resize(VSliderStart - (HLabelStart+20), 100);
    m_DataLabel.move(HLabelStart+20, HSliderTop + HSliderInterval*3 + 20);
    m_DataLabel.setAlignment(Qt::AlignTop | Qt::AlignLeft);


    connect(&m_ctrPanA, &QSlider::valueChanged, this, [=]()
    {
        OnHScroll(&m_ctrPanA);
        l_ctrPanAL->setText(QString::number(m_nPanAL));
        l_ctrPanAR->setText(QString::number(m_nPanAR));
    } );
    connect(&m_ctrPanB, &QSlider::valueChanged, this, [=]()
    {
        OnHScroll(&m_ctrPanB);
        l_ctrPanBL->setText(QString::number(m_nPanBL));
        l_ctrPanBR->setText(QString::number(m_nPanBR));
    } );
    connect(&m_ctrPanC, &QSlider::valueChanged, this, [=]()
    {
        OnHScroll(&m_ctrPanC);
        l_ctrPanCL->setText(QString::number(m_nPanCL));
        l_ctrPanCR->setText(QString::number(m_nPanCR));
    } );

    connect(&m_ctrVolA, &QSlider::valueChanged, this, [=]() {OnVScroll(&m_ctrVolA); /*l_ctrVolA->setText(QString::number(m_nVolA));*/ } );
    connect(&m_ctrVolB, &QSlider::valueChanged, this, [=]() {OnVScroll(&m_ctrVolB); /*l_ctrVolB->setText(QString::number(m_nVolB));*/ } );
    connect(&m_ctrVolC, &QSlider::valueChanged, this, [=]() {OnVScroll(&m_ctrVolC); /*l_ctrVolC->setText(QString::number(m_nVolC));*/ } );


//    QSignalMapper *signalMapper = new QSignalMapper(this);
//    connect(&m_ctrPanA, SIGNAL(valueChanged(int)), signalMapper, SLOT(map()));
//    signalMapper->setMapping(&m_ctrPanA, &l_ctrPanA);
//    connect(signalMapper, SIGNAL(mapped(QWidget*)),this, SLOT(onMapped(QWidget*)));

//    void Dashboard:: onMapped(QWidget *widget)
//    {
//        QSlider slider = qobject_cast<QSlider>(signalMapper->mapping(widget));
//        QLabel label = qobject_cast<QLabel>(widget);
//        if (label && slider) {
//            label->setText(QString::number(slider->value()));
//        }
//    }


}

CBKAYVolPan::~CBKAYVolPan()
    = default;

//void CBKAYVolPan::DoDataExchange(CDataExchange *pDX)
//{
//    CDialogEx::DoDataExchange(pDX);
//    DDX_Control(pDX, IDC_SLIDER_AYPANA, m_ctrPanA);
//    DDX_Control(pDX, IDC_SLIDER_AYPANB, m_ctrPanB);
//    DDX_Control(pDX, IDC_SLIDER_AYPANC, m_ctrPanC);
//    DDX_Control(pDX, IDC_SLIDER_AYVOLA, m_ctrVolA);
//    DDX_Control(pDX, IDC_SLIDER_AYVOLB, m_ctrVolB);
//    DDX_Control(pDX, IDC_SLIDER_AYVOLC, m_ctrVolC);
//    DDX_Text(pDX, IDC_STATIC_AYVOLA, m_nVolA);
//    DDX_Text(pDX, IDC_STATIC_AYVOLB, m_nVolB);
//    DDX_Text(pDX, IDC_STATIC_AYVOLC, m_nVolC);
//    DDX_Text(pDX, IDC_STATIC_AYPANAL, m_nPanAL);
//    DDX_Text(pDX, IDC_STATIC_AYPANBL, m_nPanBL);
//    DDX_Text(pDX, IDC_STATIC_AYPANCL, m_nPanCL);
//    DDX_Text(pDX, IDC_STATIC_AYPANAR, m_nPanAR);
//    DDX_Text(pDX, IDC_STATIC_AYPANBR, m_nPanBR);
//    DDX_Text(pDX, IDC_STATIC_AYPANCR, m_nPanCR);
//}


//BEGIN_MESSAGE_MAP(CBKAYVolPan, CDialogEx)
//    ON_WM_HSCROLL()
//    ON_WM_VSCROLL()
//END_MESSAGE_MAP()


// Обработчики сообщений CAYVolPan

// константы числа 100%
constexpr double VOL_VALUE_D = 100.0;

constexpr auto AY_VOLPAN_SLIDER_SCALE = 100; // масштаб слайдеров
constexpr auto AY_VOLPAN_SLIDER_TICK_STEP = 10; // шаг насечек слайдеров

void CBKAYVolPan::UpdateData(bool b)
{
    (void)b;
    CString s;
    s.Format("AY Channel A Pan Left = <font color=\"blue\">%d</font><br>"
             "AY Channel B Pan Left = <font color=\"blue\">%d</font><br>"
             "AY Channel C Pan Left = <font color=\"blue\">%d</font><br>"
             "AY Channel A Volume = <font color=\"blue\">%1.2f</font><br>"
             "AY Channel B Volume = <font color=\"blue\">%1.2f</font><br>"
             "AY Channel C Volume = <font color=\"blue\">%1.2f</font><br>",
             m_curr.nA_P, m_curr.nB_P, m_curr.nC_P,
             m_curr.A_V,  m_curr.B_V,  m_curr.C_V);
    m_DataLabel.setText(s);
}

BOOL CBKAYVolPan::OnInitDialog()
{
//    CDialogEx::OnInitDialog();
	CConfig::AYVolPan_s s = g_Config.getVolPan();
	m_orig = s;
    m_curr = s;

	m_nPanAL = s.nA_P;
	m_nPanAR = AY_PAN_BASE - s.nA_P;
    m_ctrPanA.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrPanA.setValue(AY_VOLPAN_SLIDER_SCALE * m_nPanAR / AY_PAN_BASE);

	m_nPanBL = s.nB_P;
	m_nPanBR = AY_PAN_BASE - s.nB_P;
    m_ctrPanB.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrPanB.setValue(AY_VOLPAN_SLIDER_SCALE * m_nPanBR / AY_PAN_BASE);

	m_nPanCL = s.nC_P;
	m_nPanCR = AY_PAN_BASE - s.nC_P;
    m_ctrPanC.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrPanC.setValue(AY_VOLPAN_SLIDER_SCALE * m_nPanCR / AY_PAN_BASE);


    m_ctrVolA.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrVolA.setValue(static_cast<int>(AY_VOLPAN_SLIDER_SCALE * (/*AY_VOL_BASE - */ s.A_V)));
	m_nVolA = int(s.A_V * VOL_VALUE_D);
    m_ctrVolB.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrVolB.setValue(static_cast<int>(AY_VOLPAN_SLIDER_SCALE * (/*AY_VOL_BASE - */ s.B_V)));
	m_nVolB = int(s.B_V * VOL_VALUE_D);
    m_ctrVolC.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrVolC.setValue(static_cast<int>(AY_VOLPAN_SLIDER_SCALE * (/*AY_VOL_BASE - */ s.C_V)));
	m_nVolC = int(s.C_V * VOL_VALUE_D);

//	for (int t = 0; t < AY_VOLPAN_SLIDER_SCALE; t += AY_VOLPAN_SLIDER_TICK_STEP)
//	{
        m_ctrPanA.setTickInterval(AY_VOLPAN_SLIDER_TICK_STEP);
        m_ctrPanB.setTickInterval(AY_VOLPAN_SLIDER_TICK_STEP);
        m_ctrPanC.setTickInterval(AY_VOLPAN_SLIDER_TICK_STEP);
        m_ctrVolA.setTickInterval(AY_VOLPAN_SLIDER_TICK_STEP);
        m_ctrVolB.setTickInterval(AY_VOLPAN_SLIDER_TICK_STEP);
        m_ctrVolC.setTickInterval(AY_VOLPAN_SLIDER_TICK_STEP);
//	}

    UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// Исключение: страница свойств OCX должна возвращать значение FALSE
}


void CBKAYVolPan::Save()
{
	g_Config.setVolPan(m_curr);
    UpdateData(FALSE);
}

void CBKAYVolPan::OnOK()
{
	Save();
//	CDialogEx::OnOK();
}


void CBKAYVolPan::OnCancel()
{
	// при отмене, возвращаем всё как было
	g_Config.setVolPan(m_orig);
//	CDialogEx::OnCancel();
}


void CBKAYVolPan::OnHScroll(QSlider *pSlider)
{

	if (pSlider == &m_ctrPanA)
	{
        m_nPanAR = AY_PAN_BASE * m_ctrPanA.value() / AY_VOLPAN_SLIDER_SCALE;
		m_curr.nA_P = m_nPanAL = AY_PAN_BASE - m_nPanAR;
		Save();
	}
	else if (pSlider == &m_ctrPanB)
	{
        m_nPanBR = AY_PAN_BASE * m_ctrPanB.value() / AY_VOLPAN_SLIDER_SCALE;
		m_curr.nB_P = m_nPanBL = AY_PAN_BASE - m_nPanBR;
		Save();
	}
	else if (pSlider == &m_ctrPanC)
	{
        m_nPanCR = AY_PAN_BASE * m_ctrPanC.value() / AY_VOLPAN_SLIDER_SCALE;
		m_curr.nC_P = m_nPanCL = AY_PAN_BASE - m_nPanCR;
		Save();
	}

}


void CBKAYVolPan::OnVScroll(QSlider *pSlider)
{

	if (pSlider == &m_ctrVolA)
	{
        m_curr.A_V = /*AY_VOL_BASE - */ double(m_ctrVolA.value()) / double(AY_VOLPAN_SLIDER_SCALE);
		m_nVolA = int(m_curr.A_V * VOL_VALUE_D);
		Save();
	}
	else if (pSlider == &m_ctrVolB)
	{
        m_curr.B_V = /*AY_VOL_BASE - */double(m_ctrVolB.value()) / double(AY_VOLPAN_SLIDER_SCALE);
		m_nVolB = int(m_curr.B_V * VOL_VALUE_D);
		Save();
	}
	else if (pSlider == &m_ctrVolC)
	{
        m_curr.C_V = /*AY_VOL_BASE - */double(m_ctrVolC.value()) / double(AY_VOLPAN_SLIDER_SCALE);
		m_nVolC = int(m_curr.C_V * VOL_VALUE_D);
		Save();
	}
}

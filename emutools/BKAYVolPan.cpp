// AYVolPan.cpp: файл реализации
//

#include "pch.h"
//#include "BK.h"
#include "BKAYVolPan.h"
//#include "afxdialogex.h"

#include <QSignalMapper>
#include <QDialogButtonBox>
#include <QLabel>

// Диалоговое окно CAYVolPan

//IMPLEMENT_DYNAMIC(CBKAYVolPan, CDialogEx)

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


    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel, this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setFixedSize(400, 300);

    buttonBox->move(200,200);

    m_ctrPanA.setParent(this);
    m_ctrPanA.move(10, 20);
    m_ctrPanA.setTickPosition(QSlider::TicksBelow);

    m_ctrPanB.setParent(this);
    m_ctrPanB.move(10, 40);
    m_ctrPanB.setTickPosition(QSlider::TicksBelow);

    m_ctrPanC.setParent(this);
    m_ctrPanC.move(10, 60);
    m_ctrPanC.setTickPosition(QSlider::TicksBelow);


    m_ctrVolA.setParent(this);
    m_ctrVolA.move(200, 10);
    m_ctrVolA.setTickPosition(QSlider::TicksBothSides);

    m_ctrVolB.setParent(this);
    m_ctrVolB.move(220, 10);
    m_ctrVolB.setTickPosition(QSlider::TicksBothSides);

    m_ctrVolC.setParent(this);
    m_ctrVolC.move(240, 10);
    m_ctrVolC.setTickPosition(QSlider::TicksBothSides);

    QLabel *l_ctrPanA = new QLabel("", this);
    QLabel *l_ctrPanB = new QLabel("", this);
    QLabel *l_ctrPanC = new QLabel("", this);

    QLabel *l_ctrVolA = new QLabel("", this);
    QLabel *l_ctrVolB = new QLabel("", this);
    QLabel *l_ctrVolC = new QLabel("", this);


    connect(&m_ctrPanA, &QSlider::valueChanged, this, [=]() {l_ctrPanA->setText(IntToString(m_ctrPanA.value())); OnHScroll(&m_ctrPanA); } );
    connect(&m_ctrPanB, &QSlider::valueChanged, this, [=]() {l_ctrPanB->setText(IntToString(m_ctrPanB.value())); OnHScroll(&m_ctrPanB); } );
    connect(&m_ctrPanC, &QSlider::valueChanged, this, [=]() {l_ctrPanC->setText(IntToString(m_ctrPanC.value())); OnHScroll(&m_ctrPanC); } );

    connect(&m_ctrVolA, &QSlider::valueChanged, this, [=]() {l_ctrVolA->setText(IntToString(m_ctrVolA.value())); OnVScroll(&m_ctrVolA); } );
    connect(&m_ctrVolB, &QSlider::valueChanged, this, [=]() {l_ctrVolB->setText(IntToString(m_ctrVolB.value())); OnVScroll(&m_ctrVolB); } );
    connect(&m_ctrVolC, &QSlider::valueChanged, this, [=]() {l_ctrVolC->setText(IntToString(m_ctrVolC.value())); OnVScroll(&m_ctrVolC); } );


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


    OnInitDialog();
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
    m_ctrVolA.setValue(static_cast<int>(AY_VOLPAN_SLIDER_SCALE * (AY_VOL_BASE - s.A_V)));
	m_nVolA = int(s.A_V * VOL_VALUE_D);
    m_ctrVolB.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrVolB.setValue(static_cast<int>(AY_VOLPAN_SLIDER_SCALE * (AY_VOL_BASE - s.B_V)));
	m_nVolB = int(s.B_V * VOL_VALUE_D);
    m_ctrVolC.setRange(0, AY_VOLPAN_SLIDER_SCALE);
    m_ctrVolC.setValue(static_cast<int>(AY_VOLPAN_SLIDER_SCALE * (AY_VOL_BASE - s.C_V)));
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

//	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// Исключение: страница свойств OCX должна возвращать значение FALSE
}


void CBKAYVolPan::Save()
{
	g_Config.setVolPan(m_curr);
//	UpdateData(FALSE);
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

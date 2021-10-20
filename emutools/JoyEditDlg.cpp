// JoyEditDlg.cpp: файл реализации
//

#if 0

#include "pch.h"
#include "BK.h"
#include "JoyEditDlg.h"
#include "afxdialogex.h"
#include "vkeycodes.h"


// CJKField

IMPLEMENT_DYNAMIC(CJKField, CStatic)

CJKField::CJKField()
	: m_nVKey(0)
	, m_strVKeyName(_T(""))
{
}

CJKField::~CJKField()
  = default;


void CJKField::SetVKey(UINT nVKey, CString &strVKName)
{
	m_nVKey = nVKey;
	m_strVKeyName = strVKName;
}

BEGIN_MESSAGE_MAP(CJKField, CStatic)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_CONTROL_REFLECT(BN_CLICKED, &CJKField::OnClicked)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// Обработчики сообщений CJKEdit

void CJKField::PreSubclassWindow()
{
	ModifyStyle(0, SS_NOTIFY);
	CStatic::PreSubclassWindow();
}

void CJKField::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	TRACE("Key Down event\n");

	if (nChar < 256)
	{
		m_nVKey = nChar;
		m_strVKeyName = g_VKeyNames[nChar];
		SetWindowText(m_strVKeyName);
	}
	else
	{
		m_nVKey = 0;
		m_strVKeyName = _T("");
		CString str;
		str.LoadString(IDS_JOYEDIT_UNKNOWN);
		SetWindowText(str);
	}
}

void CJKField::OnKillFocus(CWnd *pNewWnd)
{
	CStatic::OnKillFocus(pNewWnd);
	SetWindowText(m_strVKeyName);
}

void CJKField::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	TRACE("double click event\n");
	CString str;
	str.LoadString(IDS_JOYEDIT_PRESSKEY);
	SetWindowText(str);
	SetFocus();
	CStatic::OnLButtonDblClk(nFlags, point);
}

void CJKField::OnClicked()
{
	TRACE("click event\n");
	CString str;
	str.LoadString(IDS_JOYEDIT_PRESSKEY);
	SetWindowText(str);
	SetFocus();
}

// Диалоговое окно CJoyEditDlg

IMPLEMENT_DYNAMIC(CJoyEditDlg, CDialogEx)

CJoyEditDlg::CJoyEditDlg(CWnd *pParent /*=nullptr*/)
	: CDialogEx(IDD_JOYSTICKEDIT_DLG, pParent)
{
	for (int &n : m_nJoyValuesPos)
	{
		n = 0;
	}
}

CJoyEditDlg::~CJoyEditDlg()
    = default;

void CJoyEditDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_JOYUP,       m_nJoyValuesPos[BKJOY_UP]);
	DDX_CBIndex(pDX, IDC_COMBO_JOYRT,       m_nJoyValuesPos[BKJOY_RIGHT]);
	DDX_CBIndex(pDX, IDC_COMBO_JOYDN,       m_nJoyValuesPos[BKJOY_DOWN]);
	DDX_CBIndex(pDX, IDC_COMBO_JOYLT,       m_nJoyValuesPos[BKJOY_LEFT]);
	DDX_CBIndex(pDX, IDC_COMBO_JOYFIRE,     m_nJoyValuesPos[BKJOY_FIRE]);
	DDX_CBIndex(pDX, IDC_COMBO_JOYALTFIRE,  m_nJoyValuesPos[BKJOY_ALTFIRE]);
	DDX_CBIndex(pDX, IDC_COMBO_JOYBTNA,     m_nJoyValuesPos[BKJOY_A]);
	DDX_CBIndex(pDX, IDC_COMBO_JOYBTNB,     m_nJoyValuesPos[BKJOY_B]);
	DDX_Control(pDX, IDC_BUTTON_JOYUP,      m_ctrlEdJoy[BKJOY_UP]);
	DDX_Control(pDX, IDC_BUTTON_JOYRT,      m_ctrlEdJoy[BKJOY_RIGHT]);
	DDX_Control(pDX, IDC_BUTTON_JOYDN,      m_ctrlEdJoy[BKJOY_DOWN]);
	DDX_Control(pDX, IDC_BUTTON_JOYLT,      m_ctrlEdJoy[BKJOY_LEFT]);
	DDX_Control(pDX, IDC_BUTTON_JOYFIRE,    m_ctrlEdJoy[BKJOY_FIRE]);
	DDX_Control(pDX, IDC_BUTTON_JOYALTFIRE, m_ctrlEdJoy[BKJOY_ALTFIRE]);
	DDX_Control(pDX, IDC_BUTTON_JOYBTNA,    m_ctrlEdJoy[BKJOY_A]);
	DDX_Control(pDX, IDC_BUTTON_JOYBTNB,    m_ctrlEdJoy[BKJOY_B]);
}


BEGIN_MESSAGE_MAP(CJoyEditDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYUP, &CJoyEditDlg::OnCbnSelchangeComboJoyUp)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYRT, &CJoyEditDlg::OnCbnSelchangeComboJoyRt)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYDN, &CJoyEditDlg::OnCbnSelchangeComboJoyDn)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYLT, &CJoyEditDlg::OnCbnSelchangeComboJoyLt)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYFIRE, &CJoyEditDlg::OnCbnSelchangeComboJoyFr)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYALTFIRE, &CJoyEditDlg::OnCbnSelchangeComboJoyAFr)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYBTNA, &CJoyEditDlg::OnCbnSelchangeComboJoyBtA)
	ON_CBN_SELCHANGE(IDC_COMBO_JOYBTNB, & CJoyEditDlg::OnCbnSelchangeComboJoyBtB)
	ON_BN_CLICKED(IDC_BUTTON_JOYDEFAULT, &CJoyEditDlg::OnBnClickedButtonJoydefault)
END_MESSAGE_MAP()


// Обработчики сообщений CJoyEditDlg


BOOL CJoyEditDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetDialogItems();
	return TRUE;  // return TRUE unless you set the focus to a control
	// Исключение: страница свойств OCX должна возвращать значение FALSE
}

void CJoyEditDlg::OnCbnSelchangeComboJoyUp()
{
	CheckIntegrity(BKJOY_UP);
}

void CJoyEditDlg::OnCbnSelchangeComboJoyDn()
{
	CheckIntegrity(BKJOY_DOWN);
}

void CJoyEditDlg::OnCbnSelchangeComboJoyLt()
{
	CheckIntegrity(BKJOY_LEFT);
}

void CJoyEditDlg::OnCbnSelchangeComboJoyRt()
{
	CheckIntegrity(BKJOY_RIGHT);
}

void CJoyEditDlg::OnCbnSelchangeComboJoyFr()
{
	CheckIntegrity(BKJOY_FIRE);
}

void CJoyEditDlg::OnCbnSelchangeComboJoyAFr()
{
	CheckIntegrity(BKJOY_ALTFIRE);
}

void CJoyEditDlg::OnCbnSelchangeComboJoyBtA()
{
	CheckIntegrity(BKJOY_A);
}

void CJoyEditDlg::OnCbnSelchangeComboJoyBtB()
{
	CheckIntegrity(BKJOY_B);
}


void CJoyEditDlg::CheckIntegrity(int num)
{
	int oldValue = m_nJoyValuesPos[num];
	UpdateData(TRUE);
	int newValue = m_nJoyValuesPos[num];

	// теперь заменим дублирующееся значение.
	for (int i = 0; i < BKJOY_PARAMLEN; ++i)
	{
		if (i == num)
		{
			continue;
		}

		if (m_nJoyValuesPos[i] == newValue)
		{
			m_nJoyValuesPos[i] = oldValue;
		}
	}

	UpdateData(FALSE);
}

void CJoyEditDlg::SetDialogItems()
{
	struct sJoyIDs
	{
		UINT nComboID;
		UINT nBtnID;
	};
	const static sJoyIDs pList[BKJOY_PARAMLEN] =
	{
		{IDC_COMBO_JOYUP,       IDC_BUTTON_JOYUP        },
		{IDC_COMBO_JOYRT,       IDC_BUTTON_JOYRT        },
		{IDC_COMBO_JOYDN,       IDC_BUTTON_JOYDN        },
		{IDC_COMBO_JOYLT,       IDC_BUTTON_JOYLT        },
		{IDC_COMBO_JOYFIRE,     IDC_BUTTON_JOYFIRE      },
		{IDC_COMBO_JOYALTFIRE,  IDC_BUTTON_JOYALTFIRE   },
		{IDC_COMBO_JOYBTNA,     IDC_BUTTON_JOYBTNA      },
		{IDC_COMBO_JOYBTNB,     IDC_BUTTON_JOYBTNB      }
	};

	for (int i = 0; i < BKJOY_PARAMLEN; ++i)
	{
		auto comboctrl = static_cast<CComboBox *>(GetDlgItem(pList[i].nComboID));
		uint16_t mask = 0100000;

		for (int j = 0; j < 16; ++j)
		{
			CString str = ::WordToOctString(mask);
			comboctrl->AddString(str);

			if (mask == g_Config.m_arJoystick[i].nMask)
			{
				m_nJoyValuesPos[i] = j;
			}

			mask >>= 1;
		}

		m_ctrlEdJoy[i].SetVKey(g_Config.m_arJoystick[i].nVKey, g_Config.m_arJoystick[i].strVKeyName);
		SetDlgItemText(pList[i].nBtnID, g_Config.m_arJoystick[i].strVKeyName);
	}

	UpdateData(FALSE);
}

void CJoyEditDlg::OnOK()
{
	// вот тут будет надо сохранить изменения
	for (int i = 0; i < BKJOY_PARAMLEN; ++i)
	{
		g_Config.m_arJoystick[i].nVKey = m_ctrlEdJoy[i].GetVKey();
		g_Config.m_arJoystick[i].strVKeyName = m_ctrlEdJoy[i].GetVKeyName();
	}

	CDialogEx::OnOK();
}

void CJoyEditDlg::OnBnClickedButtonJoydefault()
{
	for (int i = 0; i < BKJOY_PARAMLEN; ++i)
	{
		g_Config.m_arJoystick[i] = g_Config.m_arJoystick_std[i];
	}

	SetDialogItems();
}

#endif

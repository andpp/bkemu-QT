
// BKMEMDlg.h : файл заголовка
// !!!доработать

#pragma once
//#include <afxcmn.h>
#include "pch.h"
#include "LockVarType.h"
//#include "afxwin.h"

#include "Config.h"
#include "OpenGlView.h"

#include <QDialog>
#include <QTabWidget>

#define MM_FIRST_PAGE 0
#define MM_SECOND_PAGE 1
#define MM_NUM_PAGES 2

struct MM16kPage_t
{
	bool bExist;        // флаг, что эта страница существует и её надо перерисовывать
	bool bColorMode;    // режим отображения текущей страницы - цветной/ЧБ
	bool bBWAdaptMode;  // режим отображения текущей страницы - адаптивный/обычный
	uint8_t *pBuffer;   // указатель на отображаемый буфер в окне экрана
	size_t nBufSize;    // размер этого буфера
};

// диалоговое окно CBKMEMDlg
class CScreen;

class CBKMEMDlg : public QDialog
{
    Q_OBJECT
//		DECLARE_DYNAMIC(CBKMEMDlg)
		// Создание
	public:
        CBKMEMDlg(BK_DEV_MPI nBKModel, BK_DEV_MPI nBKFDDModel, uint8_t *MainMem, uint8_t *AddMem, QWidget *pParent = nullptr);
		virtual ~CBKMEMDlg() override;
		// Данные диалогового окна

//		enum { IDD = IDD_BKMEM_MAP_DLG };

		void DrawTab();

	protected:
//		virtual void DoDataExchange(CDataExchange *pDX) override;   // поддержка DDX/DDV

		void CreateScreen(int nPage, int idUI);
		void CreateTabs_10();
		void CreateTabs_11M();
		void AddTabsA16M();
		void AddTabsSMK512();
		void SelectTab();
		void SetTabParam(int nPage);
		void ChangeColormode(int nPage);
		void SetColormode(int nPage, bool bMode);
		void ChangeBWMode(int nPage);
		void SetBWMode(int nPage, bool bMode);
		void ViewSprite(int nPage);
		void SaveImg(int nPage);
		void LoadImg(int nPage);

		// Реализация
	protected:
		uint8_t *m_Memory;
		uint8_t *m_MemoryADD;
		BK_DEV_MPI m_BKModel;
		BK_DEV_MPI m_BKFDDmodel;
		LockVarType m_lockStop, m_lockDraw;

		/*
		для конфигурации БК10 - 1 вкладка: 0 и 1 страницы (1 - экран)
		для конфигурации БК11 - 4 вкладки: 8 страниц, по 2 шт. во вкладке
		для A16M - вкладка, с одной страницей. (добавить реализовать такую возможность)
		для смк512 - 16 вкладок
		*/
		int m_nTabsCount;
		int m_nSelectedTab;

//		CToolTipCtrl m_ToolTip;
        QTabWidget *m_pTabWidget;

        COpenGlView *m_Screen[MM_NUM_PAGES];
		MM16kPage_t m_Container[MM_NUM_PAGES][20]; // массив окон во вкладках.

//		CTabCtrl m_tab;

		// Созданные функции схемы сообщений
        BOOL OnInitDialog();
//		virtual void OnCancel() override;
//		virtual BOOL PreTranslateMessage(MSG *pMsg) override;
        void OnTabChanged(int index);
//		afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
        void OnBnClickedButtonMmColorModeP1();
        void OnBnClickedButtonMmBwModeP1();
        void OnBnClickedButtonMmLoadP1();
        void OnBnClickedButtonMmSaveP1();
        void OnBnClickedButtonMmSpriteP1();
        void OnBnClickedButtonMmColorModeP2();
        void OnBnClickedButtonMmBwModeP2();
        void OnBnClickedButtonMmLoadP2();
        void OnBnClickedButtonMmSaveP2();
        void OnBnClickedButtonMmSpriteP2();
        void OnDestroy();
//		DECLARE_MESSAGE_MAP()

private:
    int m_nUpdateScreenTimer;

    inline void         StartTimer()
    {
        m_nUpdateScreenTimer = startTimer(50);
    }
    inline void         StopTimer()
    {
        killTimer(m_nUpdateScreenTimer);
    }

    void timerEvent(QTimerEvent *event) override {
        if (event->timerId() == m_nUpdateScreenTimer) {
            DrawTab();
        }
    }
signals:
    void CloseWindow();
};


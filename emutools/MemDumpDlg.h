#pragma once

#include <QWidget>
#include "Debugger.h"
#include "NumberEditCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryViewDlg dialog

class CDebugger;

enum class DUMP_DISPLAY_MODE : int
{
    DD_WORD_VIEW = 0,
    DD_BYTE_VIEW
};

struct DISPLAY_MODE_PARAM
{
    DUMP_DISPLAY_MODE mode;
    UINT strID;
};

constexpr auto DISPLAY_MODES_NUM = 2;

// диалоговое окно CMemDumpDlg
class CMemDumpDlg : public QWidget
{

       Q_OBJECT

        CDebugger           *m_pDebugger;
//		GFResizeDialogHelper m_Resizer;
//		CDumpListCtrl        m_ListMemory;
        CNumberEdit          *m_pNumberEdit;
        uint                 m_nBase;
        QFont                m_Font;
//		HACCEL               m_hAccelTable;
        int                  m_nCurrentDDM; // индекс текущего режима
        static DISPLAY_MODE_PARAM m_dmArray[DISPLAY_MODES_NUM]; // в этом массиве
        uint                 m_nDumpAddress;

        uint                 m_nDumpStart;
        uint                 m_nOctWidth;
        uint                 m_nAddrStart;
        uint                 m_nAddrWidth;
        uint                 m_nASCIIStart;
        uint                 m_nASCIIWidth;

        uint                 m_nlineHeight;

    public:
        CMemDumpDlg(QWidget *pParent = nullptr);  // стандартный конструктор
        virtual ~CMemDumpDlg() override;

//		CSize GetMinSize();

//		enum { IDD = IDD_MEMDUMP_DLG };
        void                AttachDebugger(CDebugger *pDbgr);

        inline uint16_t     GetDumpAddress()
        {
            return m_nDumpAddress;
        }
        inline void         SetDumpAddress(uint16_t addr)
        {
            m_nDumpAddress = addr;
        }
        uint16_t            GetDebugMemDumpWord(uint16_t dumpAddress);
        uint8_t             GetDebugMemDumpByte(uint16_t dumpAddress);

        inline void         DisplayMemDump()
        {
            repaint();
        }

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event);

public slots:
    void onEditFinished();

private:
    QChar GetMemDumpByteAsANSI(uint8_t byte);
};


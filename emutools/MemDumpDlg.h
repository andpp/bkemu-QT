#pragma once

#include <QWidget>
#include <QTimer>
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

enum class EDITING_MODE : int
{
    EM_ADDRESS = 0,
    EM_DATA,
    EM_ASCII
};

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
        QTimer               m_nTimer;
//		HACCEL               m_hAccelTable;
        int                  m_nCurrentDDM; // индекс текущего режима
        static DISPLAY_MODE_PARAM m_dmArray[DISPLAY_MODES_NUM]; // в этом массиве
        DUMP_DISPLAY_MODE    m_nDisplayMode;
        uint16_t             m_nDumpAddress;

        int                 m_nDumpStart;
        int                 m_nOctWidth;
        int                 m_nAddrStart;
        int                 m_nAddrWidth;
        int                 m_nASCIIStart;
        int                 m_nASCIIWidth;

        uint8_t            m_Mem[65536];

        int                 m_nlineHeight;

        EDITING_MODE        m_nEditingMode;
        int                 m_nEditedAddress;

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
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

public slots:
    void onEditFinished();
    void changeDisplayMode() {
        if( m_nDisplayMode == DUMP_DISPLAY_MODE::DD_BYTE_VIEW) {
            m_nDisplayMode = DUMP_DISPLAY_MODE::DD_WORD_VIEW;
        } else {
            m_nDisplayMode = DUMP_DISPLAY_MODE::DD_BYTE_VIEW;
        }
        repaint();

    }

private:
    QChar GetMemDumpByteAsANSI(uint8_t byte);
};


#ifndef CBKVIEW_H
#define CBKVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>

#include "Screen.h"
#include "BKVKBDView.h"

constexpr double BK_ASPECT_RATIO  = (4.0 / 3.0);

class CMainFrame;

class CBKView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:

    static const GLdouble m_cpTexcoords2[];
    static const GLdouble m_cpVertices2[];
    static const GLint    m_cpIndices1[];

    CBKView(QWidget *parent = 0, CScreen * pScreen = 0);
    ~CBKView();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    CScreen * m_pScreen;

    void DrawScreen() { update(); }

    void SetSmoothing(bool);

protected:
    CMainFrame * m_pParent;
    RECT m_nScreen;
    bool m_bScrSizeChanged;

public slots:
    void cleanup();

signals:

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;


protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    void EmulateKeyDown(UINT nChar, BKKey *nBKKey, UINT nModifier);
    void EmulateKeyUp(UINT nChar, BKKey * nBKKey, UINT nModifier);

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    struct BKKeyPrs_t
    {
        std::vector<UINT> vKeys; // вектор, всех одновременно нажатых клавиш. Если он пуст - то ни одна клавиша не нажата
        bool bKeyPressed; // флаг нажатия и удержания любой алфавитно-цифровой клавиши (для отключения автоповтора)
        void clear()
        {
            bKeyPressed = false;
            vKeys.clear();
        }
    };

    BKKeyPrs_t          m_kprs; // захват нажатых клавиш. Чтобы как в ВП1-014 было, имеет значение первая из всех одновременно нажатых клавиш.
    // все остальные нажатые - полностью игнорируются, пока не будут отпущены все нажатые.

    bool AddKeyToKPRS(uint8_t nScanCode);
    bool DelKeyFromKPRS(uint8_t nScanCode);
    void                ClearKPRS()
    {
        m_kprs.clear();
    }



private:
    int m_windowWidth;
    int m_windowHeight;
//    float m_xMove;
//    float m_yMove;
//    int m_xRot;
//    int m_yRot;
//    int m_zRot;
//    float m_zDistance;
    QPoint m_lastPos;

    bool m_bScrParamChanged;
    int m_nTextureParam;
    int m_nUpdateScreen;
    void set_screen_param();

    inline void         StartTimer()
    {
        m_nUpdateScreen = startTimer(25);
    }
    inline void         StopTimer()
    {
        killTimer(m_nUpdateScreen);
    }

    void timerEvent(QTimerEvent *event) override {
        if (event->timerId() == m_nUpdateScreen) update();
    }

    int m_nCurFPS;
    void RenderText(double x, double y, double z, const QString &str, const QFont &font = QFont("Helvetica",9));
    inline GLint Projection(GLdouble objx, GLdouble objy, GLdouble objz, const GLdouble model[16], const GLdouble proj[16], const GLint viewport[4], GLdouble *winx, GLdouble *winy, GLdouble *winz);
    inline void TransformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);

    int                 m_nTextureWidth;
    int                 m_nTextureHeight;


    // захват видео
            bool                m_bCaptureProcessed;
            bool                m_bCaptureFlag;
    //		HANDLE              m_hChildStd_IN_Rd;
    //		HANDLE              m_hChildStd_IN_Wr;
            std::mutex          m_mutCapture;
            void                PrepareCapture(const CString &strUniq);
            void                CancelCapture();
            void                WriteToPipe();


public:
    // захват видео
    void                SetCaptureStatus(bool bCapture, const CString &strUniq);
    bool                IsCapture()
    {
        return m_bCaptureProcessed;
    }

    void                RestoreFS();
    bool                SetFullScreenMode() { return true; }
    bool                SetWindowMode() { return true; }
    bool                IsFullScreenMode() { return false; }

    inline QWidget        *GetBackgroundWindow()
    {
        return nullptr; // this
    }
    QImage *             GetScreenshot();

    inline int          GetFPS()
            {
                return m_nCurFPS;
            }


};

#endif // CBKVIEW_H

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "Screen.h"

class COpenGlView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

private:
    static const GLdouble m_cpTexcoords2[];
    static const GLdouble m_cpVertices2[];
    static const GLint    m_cpIndices1[];

    int                   m_nTextureWidth;
    int                   m_nTextureHeight;
    int                   m_windowWidth;
    int                   m_windowHeight;
    CScreen              *m_pScreen;

public:
    COpenGlView(QWidget *parent = 0, CScreen *pScreen = 0);
    ~COpenGlView();
    void SetTextureParam(int param);
    inline CScreen * GetScreen() { return m_pScreen; }

    inline void ReDrawScreen() {
        m_pScreen->ReDrawScreen();
        update();
    }

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;

    RECT m_nScreen;
    bool m_bScrSizeChanged;
    int m_nTextureParam;
    bool m_bScrParamChanged;

    void set_screen_param();

private:
    int m_nUpdateScreenTimer;

    inline void         StartTimer()
    {
 //       m_nUpdateScreenTimer = startTimer(25);
    }
    inline void         StopTimer()
    {
//        killTimer(m_nUpdateScreenTimer);
    }

    void timerEvent(QTimerEvent *event) override {
        if (event->timerId() == m_nUpdateScreenTimer) update();
    }

};


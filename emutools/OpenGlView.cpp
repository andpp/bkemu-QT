#include "OpenGlView.h"

static constexpr double BK_ASPECT_RATIO  = (4.0 / 3.0);

const GLdouble COpenGlView::m_cpTexcoords2[] =
{
    0.0, 0.0, 0.9999, 0.0,
    0.9999, 0.9999, 0.0, 0.9999
};

const GLdouble COpenGlView::m_cpVertices2[] =
{
    0.0, 0.0, GLdouble(SCREEN_WIDTH), 0.0,
    GLdouble(SCREEN_WIDTH), GLdouble(SCREEN_HEIGHT), 0.0, GLdouble(SCREEN_HEIGHT)
};

const GLint COpenGlView::m_cpIndices1[] = { 0, 1, 2, 3 };

COpenGlView::COpenGlView(QWidget *parent, CScreen *pScreen, bool bAutoupdate)
    : QOpenGLWidget(parent)
    , m_nTextureWidth(512)
    , m_nTextureHeight(256)
    , m_pScreen(pScreen)
    , m_bScrSizeChanged(true)
{
    m_nTextureParam = GL_NEAREST;
    if(bAutoupdate) {
        StartTimer();
    }
}

COpenGlView::~COpenGlView()
{

}

void COpenGlView::SetTextureParam(int param)
{
    m_nTextureParam = param;
}

void COpenGlView::set_screen_param()
{
    makeCurrent();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Set up the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_nTextureParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_nTextureParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // Enable textures
    glEnable(GL_TEXTURE_2D);
    m_bScrParamChanged = false;
}

void COpenGlView::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1, 0.1, 0.18, 0.0);
    QSurfaceFormat format;
    format.setVersion(3,0);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::NoProfile);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    setFormat(format);
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

void COpenGlView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    uint32_t *pBits = m_pScreen->GetTexBits();

    makeCurrent();
//		if (!wglMakeCurrent(m_HDC, hGLRC))
//		{
//			if (hGLRC) // каждый раз пересоздаём контекст
//			{
//				wglMakeCurrent(m_HDC, nullptr);
//				wglDeleteContext(hGLRC);
//				hGLRC = wglCreateContext(m_HDC);
//				wglMakeCurrent(m_HDC, hGLRC);
//			}

//			m_bScrParamChanged = true;
//		}

//    if (m_bScrParamChanged)
    {
        set_screen_param();
    }

//    if (m_bScrSizeChanged)
    {
//        glViewport(0, 0, 100, 50);
        glViewport(m_nScreen.left, m_nScreen.top, m_nScreen.right, m_nScreen.bottom);
        m_bScrSizeChanged = false;
    }

    // Update Texture

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_nTextureWidth, m_nTextureHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid *>(pBits));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_DOUBLE, 0, m_cpTexcoords2);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_DOUBLE, 0, m_cpVertices2);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, m_cpIndices1);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glFlush();
//    glFinish();
}

void COpenGlView::resizeGL(int width, int height)
{
//    QScreen *screen = QGuiApplication::primaryScreen();
//    QRect  screenGeometry = screen->geometry();

    m_windowWidth = width;
    m_windowHeight = height;
    int dx = width;   // ширина экрана
    int dy = height;  // высота экрана
    int wx = static_cast<int>(static_cast<double>(dy) * BK_ASPECT_RATIO);  // ширина экрана при высоте dy в пропорциях 4/3
    int wy = static_cast<int>(static_cast<double>(dx) / BK_ASPECT_RATIO);  // высота экрана при ширине dx в пропорциях 4/3

    // рассчитаем размеры рисуемой картинки в полноэкранном режиме
//    if (dx <= dy) // если монитор повёрнут на 90 градусов, или нестандартный - квадратный
//    {
//        if (dx < wx) // если не влазит по ширине, вписываем в ширину
//        {
//            wy = dy;
//            wx = static_cast<int>(static_cast<double>(wy) * BK_ASPECT_RATIO);
//        }
//        else
//        {
//            // то вписываем картинку в высоту
//            wx = dx;
//            wy = static_cast<int>(static_cast<double>(wx) / BK_ASPECT_RATIO);
//        }
//    }
//    else    // если монитор в обычном положении
    {
        if (dx < wx) // если не влазит по ширине, вписываем в ширину
        {
            wx = dx;
            wy = static_cast<int>(static_cast<double>(wx) / BK_ASPECT_RATIO);
        }
        else
        {
            // то вписываем картинку в высоту
            wy = dy;
            wx = static_cast<int>(static_cast<double>(wy) * BK_ASPECT_RATIO);
        }
    }

    m_nScreen.left = (dx - wx) / 2;   // выравниваем по центру
    m_nScreen.top = (dy - wy) / 2;
    m_nScreen.right = wx;
    m_nScreen.bottom = wy;
    m_bScrSizeChanged = true;

//    int side = qMin(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);
//    makeCurrent();
//    glViewport((dx - wx) / 2, (dy - wy) / 2, wx, wy);

//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-1, +1, -1, +1, 1.0, 15.0);
//    glMatrixMode(GL_MODELVIEW);
}

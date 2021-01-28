#pragma once
#ifdef BKSCRDLL_EXPORTS
#define BKSCRDLL_API __declspec(dllexport)
#else
#define BKSCRDLL_API __declspec(dllimport)
#endif

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

// Этот класс экспортирован из BKScreenOGL.dll
#include "Screen_Shared.h"

//#include "gl/gl.h"
//#include "gl/glu.h"


typedef struct
{
	GLdouble r;
	GLdouble g;
	GLdouble b;
} RGBCols;

/*BKEXTERN_C*/ class /*BKSCRDLL_API*/ CScreenOGL : public CBKScreen_Shared, public QOpenGLWidget, protected QOpenGLFunctions
{
		static const GLdouble m_cpTexcoords2[];
		static const GLdouble m_cpVertices2[];
		static const GLint    m_cpIndices1[];

	protected:
		bool                m_bScrParamChanged;
		bool                m_bScrSizeChanged;

//		HDC                 m_HDC;

//		HGLRC               hGLRC;
		RECT                m_rectWndVP;   // размеры viewporta в оконном режиме
		RECT                m_rectViewPort; // текущие размеры viewporta
		// left, top - координаты экрана, right, bottom - размеры, а не координаты!!!

//		PIXELFORMATDESCRIPTOR m_pfd;
		GLint               m_nTextureParam;

	public:
        CScreenOGL(QWidget *parent);
		virtual ~CScreenOGL() override;

		virtual HRESULT     BKSS_ScreenView_Init(ScrsParams &sspar, CWnd *pwndScr) override;
		virtual void        BKSS_ScreenView_Done() override;
		virtual void        BKSS_DrawScreen(uint32_t *pBits) override;
		virtual bool        BKSS_SetFullScreenMode() override;
		virtual bool        BKSS_SetWindowMode() override;

		virtual void        BKSS_SetSmoothing(bool bSmoothing) override;
		virtual void        BKSS_SetColorMode() override;
		virtual void        BKSS_OnSize(int cx, int cy) override;

	protected:
        void                CreateContext();
		bool                SetWindowPixelFormat();
		void                set_screen_param();
		void                clear();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
//    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
//    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

};



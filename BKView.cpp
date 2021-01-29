#include "BKView.h"
#include "MainWindow.h"

#include <QMouseEvent>
//#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>

const GLdouble CBKView::m_cpTexcoords2[] =
{
    0.0, 0.0, 0.9999, 0.0,
    0.9999, 0.9999, 0.0, 0.9999
};

const GLdouble CBKView::m_cpVertices2[] =
{
    0.0, 0.0, GLdouble(SCREEN_WIDTH), 0.0,
    GLdouble(SCREEN_WIDTH), GLdouble(SCREEN_HEIGHT), 0.0, GLdouble(SCREEN_HEIGHT)
};

const GLint CBKView::m_cpIndices1[] = { 0, 1, 2, 3 };

CBKView::CBKView(QWidget *parent, CScreen *pScreen)
    : QOpenGLWidget(parent)
    , m_pScreen(pScreen)
    , m_pParent((CMainFrame *)parent)
    , m_nCurFPS(0)
    , m_nTextureWidth(512)
    , m_nTextureHeight(256)
    , m_bCaptureProcessed(false)
    , m_bCaptureFlag(false)

//      m_xMove(0),m_yMove(0),m_xRot(0),m_yRot(0),m_zRot(0),m_zDistance(1.0)
{
//    m_nTextureParam = bSmoothing ? GL_LINEAR : GL_NEAREST;
    m_nTextureParam = GL_NEAREST;
    StartTimer();
}

CBKView::~CBKView()
{
    if (m_bCaptureProcessed)
    {
        CancelCapture();
    }

    cleanup();
}

QSize CBKView::minimumSizeHint() const
{
    return QSize(640, 480);
}

QSize CBKView::sizeHint() const
{
    return QSize(640, 480);
}

void CBKView::cleanup()
{

}

void CBKView::set_screen_param()
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

void CBKView::initializeGL()
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

void CBKView::paintGL()
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
//    {
//        glViewport(0, 0, width(), height());
//        m_bScrSizeChanged = false;
//    }

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

void CBKView::resizeGL(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
//    int side = qMin(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);

//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-1, +1, -1, +1, 1.0, 15.0);
//    glMatrixMode(GL_MODELVIEW);
}

void CBKView::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void CBKView::mouseMoveEvent(QMouseEvent *event)
{
//    int dx = event->x() - m_lastPos.x();
//    int dy = event->y() - m_lastPos.y();
//    float dx_scale = dx * (1 / (float)m_windowWidth);
//    float dy_scale = -dy * (1 / (float)m_windowHeight);

//    if (event->buttons() & Qt::LeftButton)
//    {
//        setXTranslation(m_xMove + dx_scale);
//        setYTranslation(m_yMove + dy_scale);
//    } else if (event->buttons() & Qt::RightButton)
//    {
//        setXRotation(m_xRot + 8 * dy);
//        setZRotation(m_zRot + 8 * dx);
//    }
//    m_lastPos = event->pos();
}

void CBKView::wheelEvent(QWheelEvent *event)
{
//    QPoint degree = event->angleDelta() / 8;

//    if(!degree.isNull())
//    {
//        QPoint step = degree / 15;
//        setZoom(step.y() * 0.1f);
//        //m_zDistance -= step.y() * 0.5f;
//    }
}

HBITMAP CBKView::GetScreenshot()
{
//	if (m_bReverseScreen)
//	{
//		// для DIBDraw надо перевернуть экран, тупо не знаю, как это сделать с помощью готовых средств.
//		// поэтому - вручную.
//		auto pNewBits = new uint32_t[TEXTURE_WIDTH * TEXTURE_HEIGHT];
//		uint32_t *pBits = m_pTexBits + (TEXTURE_WIDTH * TEXTURE_HEIGHT);
//		uint32_t *pBitd = pNewBits;

//		for (int y = 0; y < TEXTURE_HEIGHT; ++y)
//		{
//			pBits -= TEXTURE_WIDTH;
//			memcpy(pBitd, pBits, TEXTURE_WIDTH * sizeof(uint32_t));
//			pBitd += TEXTURE_WIDTH;
//		}

//		HBITMAP hBm = CreateBitmap(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1, 32, pNewBits);
//		delete [] pNewBits;
//		return hBm;
//	}
//	else
//	{
//		HBITMAP hBm = CreateBitmap(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1, 32, m_pTexBits);
//		return hBm;
//	}
    return nullptr;
}


void CBKView::SetCaptureStatus(bool bCapture, const CString &strUniq)
{
    if (bCapture)
    {
        PrepareCapture(strUniq);
    }
    else
    {
        CancelCapture();
    }
}

void CBKView::PrepareCapture(const CString &strUniq)
{
    if (m_bCaptureProcessed)
    {
        CancelCapture();
    }
#if 0
    SECURITY_ATTRIBUTES saAttr;
    // Set the bInheritHandle flag so pipe handles are inherited.
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0 /*m_BitBufferSize*/))
    {
        TRACE("Stdin CreatePipe failed\n");
        return;
    }

    // Ensure the write handle to the pipe for STDIN is not inherited.
    if (!SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
    {
        TRACE("Stdin Wr SetHandleInformation failed\n");
        return;
    }

    // Create a child process that uses the previously created pipes for STDIN and STDOUT.
    CString strName = g_Config.m_strScreenShotsPath + _T("capture") + strUniq + _T(".mp4");
    CString szCmdline;
    szCmdline.Format(g_Config.m_strFFMPEGLine, m_scrParam.nTxX, m_scrParam.nTxY);
    szCmdline += _T(" \"") + strName + _T("\"");
    // Set up members of the PROCESS_INFORMATION structure.
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDIN and STDOUT handles for redirection.
    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.dwFlags = STARTF_USESTDHANDLES;
    siStartInfo.hStdError = nullptr; // GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = nullptr; // GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = m_hChildStd_IN_Rd;
    // Create the child process.
    BOOL bSuccess = CreateProcess(nullptr,
                                  szCmdline.GetBuffer(),     // command line
                                  nullptr,            // process security attributes
                                  nullptr,            // primary thread security attributes
                                  TRUE,               // handles are inherited
                                  CREATE_NEW_CONSOLE, /*CREATE_NO_WINDOW,*/   // creation flags
                                  nullptr,            // use parent's environment
                                  nullptr,            // use parent's current directory
                                  &siStartInfo,       // STARTUPINFO pointer
                                  &piProcInfo);     // receives PROCESS_INFORMATION

    // If an error occurs, exit the application.
    if (!bSuccess)
    {
        TRACE("CreateProcess failed\n");
        return;
    }
    else
    {
        WaitForInputIdle(piProcInfo.hProcess, 1000);
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example.
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }

    m_bCaptureProcessed = true;
    m_bCaptureFlag = true;
#endif
}

void CBKView::CancelCapture()
{
    if (m_bCaptureProcessed)
    {
        std::lock_guard<std::mutex> locker(m_mutCapture);
        // Close the pipe handle so the child process stops reading.
//		CloseHandle(m_hChildStd_IN_Wr);
//		CloseHandle(m_hChildStd_IN_Rd);
        m_bCaptureProcessed = false;
        m_bCaptureFlag = false;
    }
}

// Read from a file and write its contents to the pipe for the child's STDIN.
void CBKView::WriteToPipe()
{
    DWORD dwWritten;
    std::lock_guard<std::mutex> locker(m_mutCapture);

    if (m_bCaptureProcessed)
    {
//		BOOL bSuccess = WriteFile(m_hChildStd_IN_Wr, m_pTexBits, m_nBitBufferSize, &dwWritten, nullptr);
    }
}


void CBKView::keyPressEvent(QKeyEvent *event)
{
    uint nKey = event->key();
    uint nScanCode = event->nativeScanCode();

    m_pParent->setStatusLine("Pressed Key '" + event->text() + "'" + CString::asprintf(" key: %d(%o) scan: %d", nKey, nKey, nScanCode));



#if 0

    if (nChar == VK_MENU) // если нажат Ctrl, то нажатие alt отлавливается тут и только тут
    {
        // uint16_t uScan = HIWORD(pMsg->lParam) & 0x1FF; // это чтобы различать левый/правый альты, если uScan == 56, то левый альт, иначе - правый.
        // если nFlags & 0x100 == 0, то левый альт, если != 0 то правый
//		vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_AR2, true);
//		vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
    }
    else if (board)
    {
        // эмуляция джойстика
        // если условия для джойстика совпадут, то клавиатура не эмулируется,
        // иначе - обрабатывается эмуляция клавиатуры
        if (g_Config.m_bJoystick && !(nFlags & KF_EXTENDED))
        {
            register uint16_t& joystick = board->m_reg177714in;

            for (auto& jp : g_Config.m_arJoystick)
            {
                if (nChar == jp.nVKey)
                {
                    joystick |= jp.nMask;
                    TRACE("Joystick Set Mask\n");
                    return;
                }
            }
        }
#endif
        EmulateKeyDown(nKey, nScanCode);
//    }
}


void CBKView::keyReleaseEvent(QKeyEvent *event)
{
    uint nKey = event->key();
    uint nScanCode = event->nativeScanCode();

    m_pParent->setStatusLine("Released Key '" + event->text() + "'" + CString::asprintf(" key: %d(%o) scan: %d", nKey, nKey, nScanCode));


#if 0
    if (nChar == VK_MENU) // после нажатия alt+клавиша, отжатие alt отлавливается тут и только тут
    {
        // uint16_t uScan = HIWORD(pMsg->lParam) & 0x1FF; // это чтобы различать левый/правый альты, если uScan == 56, то левый альт, иначе - правый.
        // если nFlags & 0x100 == 0, то левый альт, если != 0 то правый
        vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_AR2, false);
        vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
    }
    else if (board)
    {
        // эмуляция джойстика
        // если условия для джойстика совпадут, то клавиатура не эмулируется,
        // иначе - обрабатывается эмуляция клавиатуры
        if (g_Config.m_bJoystick && !(nFlags & KF_EXTENDED))
        {
            register uint16_t& joystick = board->m_reg177714in;

            for (auto& jp : g_Config.m_arJoystick)
            {
                if (nChar == jp.nVKey)
                {
                    joystick &= ~jp.nMask;
                    TRACE("Joystick Clear Mask\n");
                    return;
                }
            }
        }
#endif
        EmulateKeyUp(nKey, nScanCode);
//    }
}

#if 0
void CBKView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    register auto mw = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());
    register auto vkbdvw = mw->GetBKVKBDViewPtr();

    // Если нажали любой Alt
    if (nChar == VK_MENU)
    {
        // uint16_t uScan = HIWORD(pMsg->lParam) & 0x1FF; // это чтобы различать левый/правый альты, если uScan == 56, то левый альт, иначе - правый.
        // если nFlags & 0x100 == 0, то левый альт, если != 0 то правый
        vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_AR2, true);
        vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
    }
    else if (mw->GetBoard())
    {
        EmulateKeyDown(nChar, nFlags);
    }
}

void CBKView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    register auto mw = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());
    register auto vkbdvw = mw->GetBKVKBDViewPtr();

    if (nChar == VK_MENU) // после нажатия alt, отжатие alt отлавливается тут и только тут, если не нажимали при этом других клавиш
    {
        // uint16_t uScan = HIWORD(pMsg->lParam) & 0x1FF; // это чтобы различать левый/правый альты, если uScan == 56, то левый альт, иначе - правый.
        // если nFlags & 0x100 == 0, то левый альт, если != 0 то правый
        vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_AR2, false);
        vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
    }
    else if (mw->GetBoard())
    {
        EmulateKeyUp(nChar, nFlags);
    }
}
#endif

#define VK_PAUSE   Qt::Key_Pause
#define VK_DIVIDE  Qt::Key_division
#define VK_SHIFT   Qt::Key_Shift
#define VK_CONTROL Qt::Key_Control
#define VK_CAPITAL Qt::Key_CapsLock

void CBKView::EmulateKeyDown(UINT nChar, UINT nScanCode)
{
    register auto board = m_pParent->GetBoard();
    register auto vkbdvw = m_pParent->GetBKVKBDViewPtr();

     switch (nChar)
    {
        case VK_PAUSE:      // Если нажали СТОП
        case VK_DIVIDE:
            board->StopInterrupt(); // нажали на кнопку стоп
            break;

        case VK_SHIFT:      // Если нажали Шифт
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SHIFT, true);
//            vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
            break;

        case VK_CONTROL:    // Если нажали СУ (Любой Ctrl)
            // если nFlags & 0x100 == 0, то левый ctrl, если != 0 то правый
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SU, true);
//            vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
            // вот ещё так можно определять
//          GetAsyncKeyState(VK_LCONTROL); // <0 - нажато, >=0 - нет; ret&1 - кнопка нажата после предыдущего вызова GetAsyncKeyState
//          GetAsyncKeyState(VK_RCONTROL);
            break;

        /* кнопки Загл и Стр будем эмулировать капслоком. капслок включён - стр включено, выключен - загл включено, или наоборот */
        case VK_CAPITAL:
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_CAPS, !vkbdvw->GetCapitalStatus());
            break;

        default:
        {
            // Запишем код клавиши в регистр 177662
            uint16_t nScanCode = 0;
            uint16_t nInt = 0;
//            bool bSuccess = vkbdvw->TranslateKey(nChar, !!(nFlags & KF_EXTENDED), &nScanCode, &nInt);
            bool bSuccess = vkbdvw->TranslateKey(nChar, nChar > 0xFFFF, &nScanCode, &nInt);

            if (bSuccess) // если скан код верный
            {
                // uint8_t nUnique = vkbdvw->GetUniqueKeyNum(nScanCode);
                UINT nUnique = nChar;

                // проверяем, зажали мы клавишу и держим её нажатой?
                if (!AddKeyToKPRS(nUnique))   // если нет, то это новая нажатая клавиша
                {
                    TRACE3("push key %d (char %d), pressed chars: %d\n", nUnique, nScanCode, m_kprs.vKeys.size());
                    bSuccess = !m_kprs.bKeyPressed; // ПКшный автоповтор делать?
                }
                else // такая клавиша уже нажата
                {
                    TRACE3("key %d (char %d) already pushed, pressed chars: %d\n", nUnique, nScanCode, m_kprs.vKeys.size());
                    bSuccess = !g_Config.m_bBKKeyboard; // если выключена эмуляция, то обрабатывать
                }

                if (bSuccess) // условие обработки выполняется?
                {
                    m_kprs.bKeyPressed = g_Config.m_bBKKeyboard; // отключаем автоповтор, как на реальной клавиатуре БК
                    TRACE2("processing key %d (char %d)\n", nUnique, nScanCode);

                    // если ещё прошлый код не прочитали, новый игнорируем.
                    if (!(board->m_reg177660 & 0200))
                    {
                        // сюда заходим только если прочитан прошлый код
                        board->m_reg177662in = nScanCode & 0177;
                        board->m_reg177660 |= 0200;
                        board->KeyboardInterrupt((vkbdvw->GetAR2Status()) ? INTERRUPT_274 : nInt);
                    }

                    // Установим в регистре 177716 флаг нажатия клавиши
                    board->m_reg177716in &= ~0100;
                }
            }
        }
    }
}

void CBKView::EmulateKeyUp(UINT nChar, UINT nFlags)
{
    register auto board = m_pParent->GetBoard();
    register auto vkbdvw = m_pParent->GetBKVKBDViewPtr();

    switch (nChar)
    {
        case VK_PAUSE:
        case VK_DIVIDE:
            board->UnStopInterrupt(); // отжали кнопку стоп
            break;

        case VK_SHIFT:
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SHIFT, false);
//            vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
            break;

        case VK_CONTROL:
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SU, false);
//            vkbdvw->Invalidate(FALSE); // vkbdvw->RedrawWindow();
            break;

        default:
        {
            uint16_t nScanCode = 0;
            uint16_t nInt = 0;
//            bool bSuccess = vkbdvw->TranslateKey(nChar, !!(nFlags & KF_EXTENDED), &nScanCode, &nInt);
            bool bSuccess = vkbdvw->TranslateKey(nChar, nChar > 0xFFFF, &nScanCode, &nInt);

            if (bSuccess)
            {
                // uint8_t nUnique = vkbdvw->GetUniqueKeyNum(nScanCode);
                UINT nUnique = nChar;

                switch (nScanCode)
                {
                    case BKKEY_LAT:
                        vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_XLAT, false);
                        break;

                    case BKKEY_RUS:
                        vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_XLAT, true);
                        break;
                }

                if (DelKeyFromKPRS(nUnique))
                {
                    TRACE3("pop key %d (char %d), pressed chars: %d\n", nUnique, nScanCode, m_kprs.vKeys.size());
                }

                if (m_kprs.vKeys.empty())
                {
                    TRACE0("unhit all keys!\n");
                    // Установим в регистре 177716 флаг отпускания клавиши
                    board->m_reg177716in |= 0100;
                    m_kprs.bKeyPressed = false;
                }
            }
        }
    }
}

bool CBKView::AddKeyToKPRS(uint8_t nUnique)
{
    bool bFound = false;

    if (!m_kprs.vKeys.empty())
    {
        for (auto vKey : m_kprs.vKeys)
        {
            if (vKey == nUnique)
            {
                bFound = true;
                break;
            }
        }
    }

    if (!bFound)
    {
        m_kprs.vKeys.emplace_back(nUnique);
    }

    return bFound;
}

bool CBKView::DelKeyFromKPRS(uint8_t nUnique)
{
    bool bFound = false;

    if (!m_kprs.vKeys.empty())
    {
        for (auto p = m_kprs.vKeys.begin(); p != m_kprs.vKeys.end(); ++p)
        {
            if (*p == nUnique)
            {
                m_kprs.vKeys.erase(p);
                bFound = true;
                break;
            }
        }
    }

    return bFound;
}

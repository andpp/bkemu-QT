#include "BKView.h"
#include "MainWindow.h"
#include "KeyDefinitions.h"


#include <QMouseEvent>
#include <QCoreApplication>
#include <math.h>
#include <QScreen>


CBKView::CBKView(QWidget *parent, CScreen *pScreen)
    : QWidget(parent)
    , m_pScreen(pScreen)
    , m_pParent((CMainFrame *)parent)
    , m_nCurFPS(0)
    , m_bCaptureProcessed(false)
    , m_bCaptureFlag(false)
{
    m_pGlView = new COpenGlView(this, pScreen);
    m_pGlView->resize(this->size());
}

CBKView::~CBKView()
{
    if (m_bCaptureProcessed)
    {
        CancelCapture();
    }

    cleanup();
    delete m_pGlView;
}

void CBKView::SetSmoothing(bool bSmoothing)
{
    m_pGlView->SetTextureParam(bSmoothing ? GL_LINEAR : GL_NEAREST);
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

void CBKView::resizeEvent(QResizeEvent*)
{
    m_pGlView->resize(this->size());
}


void CBKView::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void CBKView::mouseMoveEvent(QMouseEvent *event)
{

}

void CBKView::wheelEvent(QWheelEvent *event)
{
//    QPoint degree = event->angleDelta() / 8;

//    if(!degree.isNull())
//    {
//        QPoint step = degree / 15;
//    }
}

QImage * CBKView::GetScreenshot()
{
    QImage *image = new QImage((const unsigned char*)m_pScreen->GetTexBits(), 512, 256, QImage::Format_RGB32);

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
    return image;
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
//    DWORD dwWritten;
    std::lock_guard<std::mutex> locker(m_mutCapture);

    if (m_bCaptureProcessed)
    {
//		BOOL bSuccess = WriteFile(m_hChildStd_IN_Wr, m_pTexBits, m_nBitBufferSize, &dwWritten, nullptr);
    }
}

static ulong key_cnt = 0;

bool CBKView::event(QEvent *event)
{
    if(event->type() == VirtKeyEvent) {
        BKKeyEvent *ke = static_cast<BKKeyEvent *>(event);
        if(ke->n_bKeyDown) {
            EmulateKeyDown(ke->n_KeyValue, ke->n_pBKKey, 0);
        } else {
            EmulateKeyUp(ke->n_KeyValue, ke->n_pBKKey, 0);
        }
        return true;
    }

    return QWidget::event(event);
}

void CBKView::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }
    event->setAccepted(true);

    CMotherBoard *board = m_pParent->GetBoard();

    uint nKey = event->key();
    uint nScanCode = event->nativeScanCode();
    uint nModifier = event->modifiers();

    m_pParent->setStatusLine("Pressed '" + event->text() + "'" + CString::asprintf("%c %lu key: %d(%08X) scan: %d + %08X", nKey, key_cnt++, nKey, nKey, nScanCode, nModifier));

    BKKey * nBkKey = m_pParent->GetBKVKBDViewPtr()->GetBKKeyByScan(nScanCode);

    if (board)
    {
        // эмуляция джойстика
        // если условия для джойстика совпадут, то клавиатура не эмулируется,
        // иначе - обрабатывается эмуляция клавиатуры
        if (g_Config.m_bJoystick)
        {
            register uint16_t& joystick = board->m_reg177714in;

            for (auto& jp : g_Config.m_arJoystick)
            {
                if (nScanCode == jp.nVKey)
                {
                    joystick |= jp.nMask;
                    TRACE("Joystick Set Mask\n");
                    return;
                }
            }
        }

        EmulateKeyDown(nKey, nBkKey, nModifier);
    }
}

void CBKView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }
    event->setAccepted(true);

    CMotherBoard *board = m_pParent->GetBoard();

    uint nKey = event->key();
    uint nScanCode = event->nativeScanCode();
    uint nModifier = event->modifiers();

    m_pParent->setStatusLine("Released '" + event->text() + "'" + CString::asprintf("%c %lu key: %d(%08X) scan: %d + %08X", nKey, key_cnt++, nKey, nKey, nScanCode, nModifier));

    BKKey * nBkKey = m_pParent->GetBKVKBDViewPtr()->GetBKKeyByScan(nScanCode);
    if (board)
    {
        // эмуляция джойстика
        // если условия для джойстика совпадут, то клавиатура не эмулируется,
        // иначе - обрабатывается эмуляция клавиатуры
        if (g_Config.m_bJoystick)
        {
            register uint16_t& joystick = board->m_reg177714in;

            for (auto& jp : g_Config.m_arJoystick)
            {
                if (nScanCode == jp.nVKey)
                {
                    joystick &= ~jp.nMask;
                    TRACE("Joystick Clear Mask\n");
                    return;
                }
            }
        }
        EmulateKeyUp(nKey, nBkKey, nModifier);
    }
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

void CBKView::EmulateKeyDown(UINT nChar, BKKey * nBKKey, UINT nModifier)
{
    register auto board = m_pParent->GetBoard();
    register auto vkbdvw = m_pParent->GetBKVKBDViewPtr();

    if(nBKKey == nullptr)
        return;

    uint nBkUnuquieNum = nBKKey ? nBKKey->nUniqueNum : 0;

    /* кнопки Загл и Стр будем эмулировать капслоком. капслок включён - стр включено, выключен - загл включено, или наоборот */
    if (nBKKey->nScanCode == SCANCODE_CAPS) {
        nBkUnuquieNum =  vkbdvw->GetCapitalStatus() ? KBDKEY_STR : KBDKEY_ZAGL;
    }

    switch (nBkUnuquieNum)
    {
        case KBDKEY_STOP:      // Если нажали СТОП
            board->StopInterrupt(); // нажали на кнопку стоп
            break;

        case KBDKEY_DSHIFT:      // Если нажали Шифт
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SHIFT, true);
            vkbdvw->repaint();
            break;

        case KBDKEY_SU:    // Если нажали СУ (Left Win)
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SU, true);
            vkbdvw->repaint();
            break;

        case KBDKEY_AR2:    // Если нажали AR2 (Left Alt)
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_AR2, true);
            vkbdvw->repaint();
            break;

        case KBDKEY_ZAGL:
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_CAPS, true);
            break;

        case KBDKEY_STR:
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_CAPS, false);
            break;


        default:
        {
            if (nBKKey->nType != BKKeyType::REGULAR)  // Do not precess special keys here
                break;
            if(nBKKey) {
                vkbdvw->SetKeyStatus(nBKKey->nScanCode);
                if(!vkbdvw->isHidden())
                    vkbdvw->repaint();
            }
            // Запишем код клавиши в регистр 177662
            uint16_t nKeyCode = 0;
            uint16_t nInt = 0;
            bool bSuccess = vkbdvw->TranslateKey(nChar, nBKKey, &nKeyCode, &nInt);

            if (bSuccess) // если скан код верный
            {
                uint8_t nUnique = nBKKey->nUniqueNum; // vkbdvw->GetUniqueKeyNum(nKeyCode);

                // проверяем, зажали мы клавишу и держим её нажатой?
                if (!AddKeyToKPRS(nUnique))   // если нет, то это новая нажатая клавиша
                {
                    TRACE3("push key %d (char %d), pressed chars: %d\n", nUnique, nKeyCode, static_cast<int>(m_kprs.vKeys.size()));
                    bSuccess = !m_kprs.bKeyPressed; // ПКшный автоповтор делать?
                }
                else // такая клавиша уже нажата
                {
                    TRACE3("key %d (char %d) already pushed, pressed chars: %d\n", nUnique, nKeyCode, static_cast<int>(m_kprs.vKeys.size()));
                    bSuccess = !g_Config.m_bBKKeyboard; // если выключена эмуляция, то обрабатывать
                }

                if (bSuccess) // условие обработки выполняется?
                {
                    m_kprs.bKeyPressed = g_Config.m_bBKKeyboard; // отключаем автоповтор, как на реальной клавиатуре БК
                    TRACE2("processing key %d (char %d)\n", nUnique, nKeyCode);

                    // если ещё прошлый код не прочитали, новый игнорируем.
                    if (!(board->m_reg177660 & 0200))
                    {
                        // сюда заходим только если прочитан прошлый код
                        board->m_reg177662in = nKeyCode & 0177;
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

void CBKView::EmulateKeyUp(UINT nChar, BKKey * nBKKey, UINT nModifier)
{
    register auto board = m_pParent->GetBoard();
    register auto vkbdvw = m_pParent->GetBKVKBDViewPtr();

    if(nBKKey == nullptr)
        return;

    switch (nBKKey->nUniqueNum)
    {
        case KBDKEY_STOP:
            board->UnStopInterrupt(); // отжали кнопку стоп
            break;

        case KBDKEY_DSHIFT:
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SHIFT, false);
            vkbdvw->repaint();
            break;

        case KBDKEY_SU:
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_SU, false);
            vkbdvw->repaint();
            break;

        case KBDKEY_AR2:    // Если нажали AR2 (Left Alt)
            vkbdvw->SetKeyboardStatus(STATUS_FIELD::KBD_AR2, false);
            vkbdvw->repaint();
            break;

        default:
        {
            if (nBKKey->nType != BKKeyType::REGULAR)  // Do not precess special keys here
                break;
            vkbdvw->SetKeyStatus(0xFFFFFFFF);
            if(!vkbdvw->isHidden())
                vkbdvw->repaint();

            uint16_t nKeyCode = 0;
            uint16_t nInt = 0;
            bool bSuccess = vkbdvw->TranslateKey(nChar, nBKKey, &nKeyCode, &nInt);

            if (bSuccess)
            {
                uint8_t nUnique = nBKKey->nUniqueNum; // vkbdvw->GetUniqueKeyNum(nKeyCode);

                switch (nKeyCode)
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
                    TRACE3("pop key %d (char %d), pressed chars: %d\n", nUnique, nKeyCode, static_cast<int>(m_kprs.vKeys.size()));
                }

//                if (m_kprs.vKeys.empty())
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

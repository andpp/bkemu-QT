#include "MainWindow.h"
#include "encoding.h"
#include "Config.h"

extern CMainFrame *g_pMainFrame;
extern QObject    *g_pBKView;


CLuaScripts::CLuaScripts(QObject *parent)
    : QObject(parent)
    , L(nullptr)
    , m_pThread(nullptr)
{
    Init();
}

CLuaScripts::~CLuaScripts()
{
    if(L)
        lua_close(L);
    L = nullptr;

    if(m_pThread) {
        if (m_pThread->joinable())
            m_pThread->join();
        delete m_pThread;
    }
    m_pThread = nullptr;

}

void CLuaScripts::Init()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    open_libBK(L);

}

bool CLuaScripts::RunScript(const CString &script)
{
    int status = luaL_dostring(L, script.GetString());

    if (status != 0) {
        const char *lua_err = lua_tostring(L, -1);
        printf("Run Script Error: %s\n\n", lua_err);
        return false;
    }

    return true;
}

bool CLuaScripts::RunFileScript(const CString &fname)
{
    if(m_pThread) {
        if (m_pThread->joinable())
            // Wait while the previous thread will be finished
            m_pThread->join();
        delete m_pThread;
    }

    // int status = luaL_loadfile(L, fname.GetString());
    // luaL_loadfile returns error after the first call
    // We will use 'loadfile' lua function instead
    lua_getglobal(L, "loadfile");
    lua_pushstring(L, fname.GetString());

    // Load and run script in a separate thread
    m_pThread = new std::thread([=]{
        int status = lua_pcall(L, 1, LUA_MULTRET, 0);
        if (status != 0) {
            const char *lua_err = lua_tostring(L, -1);
            CString errMsg;
            errMsg.Format("Load File Script Error: %s\n", lua_err);
            g_BKMsgBox.Show(errMsg, MB_OK);
            return;
        } else {
            status = lua_pcall(L, 0, LUA_MULTRET, 0);
        }

        if (status != 0) {
            const char *lua_err = lua_tostring(L, -1);
            CString errMsg;
            errMsg.Format("Run File Script Error: %s\n", lua_err);
            g_BKMsgBox.Show(errMsg, MB_OK);
            return;
        }

    });

    if (m_pThread->joinable())
    {
        m_pThread->detach();
    }

    return true;
}


#define LibFunc(name) {#name, LuaFunc(name)}
#define LuaFunc(name) name##_luafunc
#define defLuaFunc(name)    \
int LuaFunc(name)(lua_State* state)


/* LoadBin(fname)
 * arg:
 *         fname - string
 * return:
 * return:
 *         true  if succes
 *         false if fail
 */
defLuaFunc(LoadBin)
{
  // The number of function arguments will be on top of the stack.
  int args = lua_gettop(state);
  const char *fName;
  bool res = false;

  if(args == 1 && lua_isstring(state, 1)) {
      fName = lua_tostring(state, 1);
      CString sfName(fName);
      res = g_pMainFrame->LoadBinFile(sfName);
  } else {
      lua_pushliteral(state, "LoadBin: incorrect argument");
      lua_error(state);
  }

  lua_pushnumber(state, res);

  return 1; // Number of return values
}

/* LoadBreakpoints(fname)
 * arg:
 *         fname - string
 * return:
 *         true  if succes
 *         false if fail
 */
defLuaFunc(LoadBreakpoints)
{
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    bool res = false;

    if(args == 2 && lua_isstring(state, 1) && lua_isboolean(state,2)) {
        const char *fName = lua_tostring(state, 1);
        bool bMerge = lua_toboolean(state, 2);
        CString sfName(fName);

        res = g_pMainFrame->m_pDebugger->LoadBreakpoints(sfName, bMerge);
    } else {
        lua_pushliteral(state, "LoadBreakpoints: incorrect argument");
        lua_error(state);
    }

    lua_pushnumber(state, res);

    return 1; // Number of return values
}

defLuaFunc(LoadSymbolTable)
{
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    bool res = false;

    if(args == 1 && lua_isstring(state, 1)) {
        const char *fName = lua_tostring(state, 1);
        CString sfName(fName);

        if(!::GetFileExt(sfName).CompareNoCase("stb")) {
            res = g_pMainFrame->m_pDebugger->m_SymTable.LoadSymbolsSTB(sfName);
        } else if(!::GetFileExt(sfName).CompareNoCase("lst")) {
            res = g_pMainFrame->m_pDebugger->m_SymTable.LoadSymbolsLST(sfName);
        } else {
            lua_pushliteral(state, "LoadSymbolTable: unknown file format");
            lua_error(state);
        }
    } else {
        lua_pushliteral(state, "LoadSymbolTable: incorrect argument");
        lua_error(state);
    }

    lua_pushnumber(state, res);

    return 1; // Number of return values
}


static FDD_DRIVE DrvToFDD_DRIVE(int id)
{
    FDD_DRIVE eDrive = FDD_DRIVE::NONE;

    switch (id)
    {
        case ID_FILE_LOADDRIVE_A:
            eDrive = FDD_DRIVE::A;
            break;

        case ID_FILE_LOADDRIVE_B:
            eDrive = FDD_DRIVE::B;
            break;

        case ID_FILE_LOADDRIVE_C:
            eDrive = FDD_DRIVE::C;
            break;

        case ID_FILE_LOADDRIVE_D:
            eDrive = FDD_DRIVE::D;
            break;
    }

    return eDrive;

}

/* GetImageName(DriveNum)
 * arg:
 *         DriveNum - number
 * return:
 *         ImageName or "" if empty - string
 */
defLuaFunc(GetImageName)
{
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    const char *imageName = "";

    if(args == 1 && lua_isnumber(state, 1)) {
        int id = lua_tointeger(state, 1);

        FDD_DRIVE eDrive = DrvToFDD_DRIVE(id);
        if (g_pMainFrame->GetBoard() && eDrive != FDD_DRIVE::NONE)
        {
            CFDDController *fdd = g_pMainFrame->GetBoard()->GetFDD();
            if (fdd && fdd->IsAttached(eDrive)) {
                imageName = g_Config.GetDriveImgName(eDrive).GetString();
            }
        }
    } else {
        lua_pushliteral(state, "GetImageName: incorrect argument");
        lua_error(state);
    }

    lua_pushstring(state, imageName);

    return 1; // Number of return values
}

/* MountImage(DriveNum, ImageName)
 * arg:
 *         DriveNum - number
 *         ImageName - string
 * return:
 *         true  if succes
 *         false if fail
 */
defLuaFunc(MountImage)
{
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    bool res = false;

    if(args == 2 && lua_isnumber(state, 1) && lua_isstring(state,2)) {
        int id = lua_tointeger(state, 1);
        const char *fName = lua_tostring(state, 2);
        CString sfName(fName);

        FDD_DRIVE eDrive = DrvToFDD_DRIVE(id);
        if (g_pMainFrame->GetBoard() && eDrive != FDD_DRIVE::NONE)
        {
            CFDDController *fdd = g_pMainFrame->GetBoard()->GetFDD();
            if(fdd) {
                res = fdd->AttachImage(eDrive, sfName);
            }

            g_Config.SetDriveImgName(eDrive, sfName);
            g_pMainFrame->UpdateToolbarDriveIcons();
        }

    } else {
        lua_pushliteral(state, "MountImage: incorrect arguments");
        lua_error(state);
    }

    lua_pushnumber(state, res);

    return 1; // Number of return values
}

/* UnMountImage(DriveNum)
 * arg:
 *         DriveNum - number
 * return:
 *         true  if succes
 *         false if fail
 */
defLuaFunc(UnMountImage)
{
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    bool res = false;

    if(args == 1 && lua_isnumber(state, 1)) {
        int id = lua_tointeger(state, 1);
        g_pMainFrame->OnFileUnmount(id);
        res = true;
    } else {
        lua_pushliteral(state, "UnMountImage: incorrect argument");
        lua_error(state);
    }

    lua_pushnumber(state, res);

    return 1; // Number of return values
}

static bool SendKey(uint8_t ch, int pause)
{
    while (g_pMainFrame->m_pBoard->m_reg177660 & 0200) {}
    g_pMainFrame->m_pBoard->m_reg177662in = ch & 0177; // Отправим код символа в 177662
    g_pMainFrame->m_pBoard->m_reg177660 |= 0200;       // Установим состояние готовности в 177660
    g_pMainFrame->m_pBoard->m_reg177716in &= ~0100;    // Установим флаг нажатия клавиши в 177716
    g_pMainFrame->m_pBoard->KeyboardInterrupt(INTERRUPT_60);

    Sleep(pause);
    g_pMainFrame->m_pBoard->m_reg177716in |= 0100;

    return true;
}

static bool SendChar(uint8_t ch, int pause)
{
    bool res = true;

    if (ch >= 0300 && !g_pMainFrame->m_paneBKVKBDView->GetXLatStatus()) // если буквы русские, а режим - не русский
    {
        res = SendKey(BKKEY_RUS, pause); // пошлём код переключения языка
        g_pMainFrame->m_paneBKVKBDView->SetXLatStatus(true);
    }
    else if ((0100 <= ch && ch <= 0177) && g_pMainFrame->m_paneBKVKBDView->GetXLatStatus()) // если буквы не русские, а режим - русский
    {
        res = SendKey(BKKEY_LAT, pause); // пошлём код переключения языка
        g_pMainFrame->m_paneBKVKBDView->SetXLatStatus(false);
    }

    if (res) {
        res = SendKey(ch, pause);
    }

    return res;
}

defLuaFunc(PressKey)
{
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    bool res = false;

    if(args == 1 && lua_isinteger(state, 1)) {
        int nKey = lua_tointeger(state, 1);

        // Processing special keys here
        switch(nKey){
            case 0:
                // Send STOP
                g_pMainFrame->m_pBoard->StopInterrupt();
                Sleep(80);
                g_pMainFrame->m_pBoard->UnStopInterrupt();
                res = true;
                break;
             default:
                uint8_t ch = utf82koi(nKey);
                res = SendChar(ch, 100);
                break;
        }
    } else {
        lua_pushliteral(state, "PressKey: incorrect argument");
        lua_error(state);
    }

    lua_pushnumber(state, res);

    return 1; // Number of return values
}

defLuaFunc(SendStringToBK)
{
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    bool res = false;
    const char *pustr = nullptr;
    uint delay=80;

    if(args == 1 && lua_isstring(state, 1)) {
       pustr = lua_tostring(state, 1);
    } else if (args == 2) {
        if (lua_isstring(state, 1) &&
            lua_isnumber(state,2)) {
            pustr = lua_tostring(state, 1);
            delay = lua_tointeger(state, 2);
        }
    }  else {
        lua_pushliteral(state, "SendStringToBK: incorrect argument");
        lua_error(state);
    }

    if(pustr != nullptr) {
       utf8_stream utf8str(pustr, strlen(pustr));
       for(int sym = utf8str.decode_next(); sym > 0; sym = utf8str.decode_next()) {
           uint8_t ch = utf82koi(sym);
           res = SendChar(ch, delay);
           if(!res)
               break;
       }
    }

    lua_pushnumber(state, res);

    return 1; // Number of return values

}

defLuaFunc(Poke)
{
    int args = lua_gettop(state);

    if(args == 2 && lua_isnumber(state, 1) && lua_isnumber(state, 2)) {
        uint nAddr = lua_tonumber(state, 1);
        uint nData = lua_tonumber(state, 2);
        g_pMainFrame->m_pBoard->SetWordIndirect(nAddr, nData);
    } else {
        lua_pushliteral(state, "Poke: incorrect arguments");
        lua_error(state);
    }

    return 0;
}

defLuaFunc(Pokeb)
{
    int args = lua_gettop(state);

    if(args == 2 && lua_isnumber(state, 1) && lua_isnumber(state, 2)) {
        uint nAddr = lua_tonumber(state, 1);
        uint nData = lua_tonumber(state, 2);
        g_pMainFrame->m_pBoard->SetByteIndirect(nAddr, nData);
    } else {
        lua_pushliteral(state, "Pokeb: incorrect arguments");
        lua_error(state);
    }

    return 0;

}

defLuaFunc(Peek)
{
    int args = lua_gettop(state);
    uint res = 0;

    if(args == 1 && lua_isnumber(state, 1)) {
        uint nAddr = lua_tonumber(state, 1);
        res = g_pMainFrame->m_pBoard->GetWordIndirect(nAddr);
    } else {
        lua_pushliteral(state, "Peek: incorrect argument");
        lua_error(state);
    }

    lua_pushnumber(state, res);
    return 1;
}

defLuaFunc(Peekb)
{
    int args = lua_gettop(state);
    uint res = 0;

    if(args == 1 && lua_isnumber(state, 1)) {
        uint nAddr = lua_tonumber(state, 1);
        res = g_pMainFrame->m_pBoard->GetByteIndirect(nAddr);
    } else {
        lua_pushliteral(state, "Peekb: incorrect argument");
        lua_error(state);
    }

    lua_pushnumber(state, res);
    return 1;
}

defLuaFunc(Sleep)
{
    int args = lua_gettop(state);

    if(args == 1 && lua_isnumber(state, 1)) {
        uint mSec = lua_tonumber(state, 1);
        Sleep(mSec);
    } else {
        lua_pushliteral(state, "Sleep: incorrect argument");
        lua_error(state);
    }

    return 0;
}

defLuaFunc(StopCPU)
{
    (void)state;
    if(!g_pMainFrame->m_pBoard->IsCPUBreaked())
        g_pMainFrame->OnDebugBreak();
    return 0;
}

defLuaFunc(StartCPU)
{
    (void)state;
    if(g_pMainFrame->m_pBoard->IsCPUBreaked())
        g_pMainFrame->OnDebugBreak();
    return 0;
}


static const luaL_Reg BKemu_funcs[] = {
    LibFunc(LoadBin),           // LoadBin(strFileName)            - Load Binary file strFileName
    LibFunc(LoadBreakpoints),   // LoadBreakpointd(strFileName)    - Load breakpoints from strFileName
    LibFunc(LoadSymbolTable),   // LoadSymbolTable(strFileName)    - Load symbol table
    LibFunc(GetImageName),      // GetImageName(nFddNumber)        - Return path to the image mounted to FDD
    LibFunc(MountImage),        // MountImage(nFddNumber, strImageName)  - Mount strImageName to FDD
    LibFunc(UnMountImage),      // UnMountImage(nFddNumber)        - Unmount image from Fdd
    LibFunc(PressKey),          // PressKey(nKey)                  - Send Keycode to BK
    LibFunc(SendStringToBK),    // SendStringToBk(strText, nDelay) - Send characters from strText to BK one-by-one. Switch to Rus/Lat if need.
                                //                                 - nDelay - delay between chars in mSek. If omitted, used 80 mSec delay
    LibFunc(Poke),              // poke(nAddr, nData)              - store word nData into nAddr
    LibFunc(Pokeb),             // pokeb(nAddr, bData)             - store byte bData into nAddr
    LibFunc(Peek),              // peek(nAddr)                     - return word in nAddr
    LibFunc(Peekb),             // peekb(nAddr)                    - return byte in nAddr
    LibFunc(Sleep),             // sleep(mSec)                     - sleep mSec millicesonds
    LibFunc(StopCPU),           // StopCPU()                       - Stop CPU if running
    LibFunc(StartCPU),          // StopCPU()                       - Sart CPU if not running
    {NULL, NULL}
};

int CLuaScripts::open_libBK (lua_State *L)
{
    const luaL_Reg *i = BKemu_funcs;
    for(; i->name; i++) {
        lua_register(L, i->name, i->func);
    }
    RunScript(" ");
  return 1;
}

#include "MainWindow.h"

extern CMainFrame *g_pMainFrame;

CLuaScripts::CLuaScripts(QObject *parent)
    : QObject(parent)
    , L(nullptr)
{
    Init();
}

CLuaScripts::~CLuaScripts()
{
    if(L)
        lua_close(L);
    L = nullptr;
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
    // int status = luaL_loadfile(L, fname.GetString());
    // luaL_loadfile returns error after the first call
    // We will use 'loadfile' lua function instead
    lua_getglobal(L, "loadfile");
    lua_pushstring(L, fname.GetString());
    int status = lua_pcall(L, 1, LUA_MULTRET, 0);
    if (status != 0) {
        const char *lua_err = lua_tostring(L, -1);
        printf("Load File Script Error: %s\n", lua_err);
        return false;
    } else {
        status = lua_pcall(L, 0, LUA_MULTRET, 0);
    }

    if (status != 0) {
        const char *lua_err = lua_tostring(L, -1);
        printf("Run File Script Error: %s\n", lua_err);
        return false;
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
    }

    lua_pushnumber(state, res);

    return 1; // Number of return values
}


static const luaL_Reg BKemu_funcs[] = {
    LibFunc(LoadBin),
    LibFunc(LoadBreakpoints),
    LibFunc(GetImageName),
    LibFunc(MountImage),
    LibFunc(UnMountImage),
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

#pragma once

#include <QObject>
#include <thread>

#include <QThread>

#include "pch.h"
#include "lua.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// 1. Because any updates to UI can be dune in the main thread only, any function changing UI MUST be called using emit signal
// 2. Because we can stop executing of Lua script only inside Lua hook function, all C-implemented Lua functions must ether
//    be fast enough ot MUST check status of (bool)StopScriptThread variable and exit immediately if StopScriptThread = true
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class CThread : public QThread
{
    Q_OBJECT;

signals:
    void DebugBreak();
    void UpdateToolbarDriveIcons();
    void LoadBreakpoints(bool *res, const CString &fname, bool merge = false);
    void LoadSymbolsSTB(bool *res, const CString &fname, bool merge = false);
    void LoadSymbolsLST(bool *res, const CString &fname, bool merge = false);
};


class CLuaScripts : public QObject
{
    Q_OBJECT
    lua_State *L;

    void Init();
    int open_libBK(lua_State *L);

public:
    explicit CLuaScripts(QObject *parent = nullptr);
    ~CLuaScripts();

    bool RunScript(const CString &script);
    bool RunFileScript(const CString &fname);
    void StopThread();
};

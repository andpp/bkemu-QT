#pragma once

#include <QObject>
#include <thread>

#include "pch.h"
#include "lua.hpp"

class CLuaScripts : public QObject
{
    Q_OBJECT
    lua_State *L;

    void Init();
    int open_libBK(lua_State *L);
    std::thread *m_pThread;

public:
    explicit CLuaScripts(QObject *parent = nullptr);
    ~CLuaScripts();

    bool RunScript(const CString &script);
    bool RunFileScript(const CString &fname);

signals:

};

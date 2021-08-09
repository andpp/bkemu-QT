#pragma once

#include <QObject>

#include "pch.h"
#include "lua.hpp"

class CLuaScripts : public QObject
{
    Q_OBJECT
    lua_State *L;

    void Init();
    int open_libBK(lua_State *L);

public:
    explicit CLuaScripts(QObject *parent = nullptr);
    ~CLuaScripts();

    bool RunScript(CString &script);
    bool RunFileScript(CString &fname);

signals:

};

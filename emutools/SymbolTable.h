#ifndef CSYMTABLE_H
#define CSYMTABLE_H
#pragma once

#include <QHash>
#include "pch.h"

#include "lua.hpp"

typedef QMultiMap<uint16_t, CString> SymTableAddr_t;
typedef QMap<CString, uint16_t> SymTable_t;

class GSDWriter
{
    FILE *m_fp;
    uint32_t m_nOffset;
    uint8_t m_Buf[1024];
public:
    GSDWriter(const CString&fbname);
    ~GSDWriter();
    int  gsd_init();
    int  gsd_write(const CString &name, int flags, int type, int value);
    int  gsd_flush();
    int  writerec(uint8_t *data, int len);
    int  gsd_end();

};

class CSymTable {
    SymTable_t m_SymbolsMap;
    SymTableAddr_t m_SymbolsAddrMap;
    lua_State          *L;

public:
    constexpr static uint16_t SYMBOL_NOT_EXIST = 0xFFFF;

    CSymTable();

    void EnableLua(lua_State *lua) { L = lua; }

    bool            AddSymbol(const u_int16_t addr, const CString& name);
    bool            AddSymbolIfNotExist(const u_int16_t addr, const CString& name);
    CString         GetSymbolForAddr(const uint16_t addr);
    uint16_t        GetAddrForSymbol(const CString& name);
    bool            RemoveSymbol(const u_int16_t addr);
    bool            RemoveSymbol(const CString& name);
    int             LoadSymbolsLST(const CString &fname, bool bMerge = false);
    int             LoadSymbolsSTB(const CString &fname, bool bMerge = false);
    int             SaveSymbolsSTB(const CString &fname);

    SymTable_t*     GetAllSymbols() { return &m_SymbolsMap;  }
    SymTableAddr_t* GetAllAddresses() { return &m_SymbolsAddrMap; }
    void            RemoveAllSymbols() {m_SymbolsMap.clear(); }
    bool            Contains(const int addr) {return m_SymbolsAddrMap.contains(addr); }
    bool            Contains(const CString &str) { return m_SymbolsMap.contains(str); }
    CString         operator[](int addr) {return m_SymbolsAddrMap.value(addr); }
    uint16_t        operator[](CString &str) {return m_SymbolsMap.value(str); }

private:
    char            unrad50buffer[7];
    void            unrad50(uint16_t word, char *cp);
    // Decodes 6 chars of RAD50 into the temp buffer and returns buffer address
    const char*     unrad50(uint16_t loword, uint16_t hiword);
    // Decodes 6 chars of RAD50 into the temp buffer and returns buffer address
    const char*     unrad50(uint32_t data);
    int             rad50name(char *cp, char *name);
    int             process_gsd_item(const uint8_t* itemw);

};

#endif // CSYMTABLE_H

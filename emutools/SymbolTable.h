#ifndef CSYMTABLE_H
#define CSYMTABLE_H
#pragma once

#include <QHash>
#include "pch.h"

typedef QHash<int16_t, CString> SymTable_t;

class CSymTable {
    SymTable_t m_SymbolsMap;
public:
    constexpr static uint16_t SYMBOL_NOT_EXIST = 0xFFFF;

    CSymTable();

    void          AddSymbol(const u_int16_t addr, const CString& name);
    void          AddSymbolIfNotExist(const u_int16_t addr, const CString& name);
    CString       GetSymbolForAddr(const uint16_t addr);
    uint16_t      GetAddrForSymbol(const CString& name);
    void          RemoveSymbol(const u_int16_t addr);
    void          RemoveSymbol(const CString& name);
    int           LoadSymbols(const CString &fname);
    int           LoadSymbolsSTB(const CString &fname);

    SymTable_t*   GetAllSymbols() { return &m_SymbolsMap;  }
    void          RemoveAllSymbols() {m_SymbolsMap.clear(); }
    bool          Contains(int addr) {return m_SymbolsMap.contains(addr); }
    CString operator[](int addr) {return m_SymbolsMap[addr]; }

private:
    char unrad50buffer[7];
    void unrad50(uint16_t word, char *cp);
    // Decodes 6 chars of RAD50 into the temp buffer and returns buffer address
    const char* unrad50(uint16_t loword, uint16_t hiword);
    // Decodes 6 chars of RAD50 into the temp buffer and returns buffer address
    const char* unrad50(uint32_t data);
    int rad50name(char *cp, char *name);
    int process_gsd_item(const uint8_t* itemw);

};

#endif // CSYMTABLE_H

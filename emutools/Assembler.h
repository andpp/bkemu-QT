#ifndef CASSEMBLER_H
#define CASSEMBLER_H

#include <QObject>
#include "CString.h"
#include <QHash>
#include <ctype.h>

class CDebugger;

enum class CPUCmdGroup
{
    NOOPS,
    CBRANCH,
    EIS,
    TRAP,
    SOB,
    MARK,
    TWOOPREG,
    FIS,
    PUSH,
    ONEOPS,
    TWOOPS,
    WORD
};

struct CPUCommandStruct
{
    uint16_t nOpcode;       // генерируемый опкод
    CPUCmdGroup nGroup;     // группа, к которой принадлежит команда
};

enum {
    TOKEN_NUMBER,
    TOKEN_NAME,
    TOKEN_AT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_RBRACKET,
    TOKEN_LBRACKET,
    TOKEN_DIRECT,
    TOKEN_UNKNOWN,
};

struct token {
    uint type;
    int n;
    CString s;
};

class CReader
{
    int len;
    const char *buffer;
    const char *curr;

public:
    CReader(const CString &str) {
        buffer = new char[str.size()+1];
        memcpy((void *)buffer, (void *)str.toLatin1().data(), str.size()+1);
        curr = buffer;
        len = str.size();
    }

    ~CReader() {
        delete[] buffer;
    }

    void SkipSpaces() {
        while(*curr && *curr == ' ') {
            curr++;
        }
    }

    char GetChar() {
        SkipSpaces();
        char c = *curr;
        if (*curr) {
            curr++;
        }
        return c;
    }

    char CurrChar() {
        SkipSpaces();
        return *curr;
    }


    // Return current positiion in the string
    int GetCurrPos() {
        return curr - buffer;
    }

    bool IsEnd() {
        return (*curr == 0);
    }

    int GetToken(token &t) {
        SkipSpaces();
        int pos = GetCurrPos();
        t.type = TOKEN_UNKNOWN;

        if(*curr == 0)
            return len;

        switch (*curr) {
            case '-':
                if(!isdigit(*curr+1)) {
                     t.type = TOKEN_MINUS;
                     curr++;
                     return pos;
                }
                break;
             case '+':
                t.type = TOKEN_PLUS;
                curr++;
                return pos;
             case '@':
                   t.type = TOKEN_AT;
                   curr++;
                   return pos;
             case '(':
                   t.type = TOKEN_LBRACKET;
                   curr++;
                   return pos;
             case ')':
                   t.type = TOKEN_RBRACKET;
                   curr++;
                   return pos;
             case '#':
                   t.type = TOKEN_DIRECT;
                   curr++;
                   return pos;
        }


        if(isdigit(*curr) || *curr == '-' ) {
            ParseNumber(t);
        } else if (isalpha(*curr) || *curr == '.') {
            // Parse name
            ParseName(t);
        }

        return pos;
    }

    bool ParseNumber(token &t);
    bool ParseName(token &t);

};

class CAssembler : public QObject
{
    Q_OBJECT

    CDebugger  *m_pDebugger;
    QHash<CString, int> g_Regs;

    uint16_t    m_ResultData[3];  // Resulting asembled buffer
    QHash<CString, CPUCommandStruct> g_pCPUCommands;

    bool        bOperandType;
    int         nAriphmType;

    CString     m_sError;       // Error text
    int         m_nErrorPos;    // Error position
    uint16_t    m_nPC;          // PC register
    uint16_t    m_nPCOff;       // Offset from starting PC

public:
    explicit CAssembler(QObject *parent = nullptr);

    void SetDebugger(CDebugger *debugger) {
        m_pDebugger = debugger;
    }

    bool AssembleString(const CString &str, uint16_t startAddr)
    {
        m_nPC = startAddr+2;
        m_nPCOff = 1;
        return AssembleCPUInstruction(str);
    }
    uint GetResultingBuffer(const uint16_t **buffer) {
        *buffer = m_ResultData;
        return m_nPCOff;
    }

    uint GetError(CString &str) {
        str = m_sError;
        return m_nErrorPos;
    }

private:
    void Init();

    bool AssembleCPUInstruction(const CString &str);
    bool assemble2OP(CReader &rdr);
    bool assemble1OP(CReader &rdr);
    bool assemble1OPR(CReader &rdr);
    bool assemble2OPR(CReader &rdr);
    bool assemble2ROP(CReader &rdr);
    bool assembleSOB(CReader &rdr);
    bool assembleTRAP(CReader &rdr);
    bool assembleMARK(CReader &rdr);
    bool assembleBR(CReader &rdr);
    bool assembleWORD(CReader &rdr);
    bool Operand_analyse(CReader &rdr);

    void OutError(const CString &err, int pos) {
        m_sError = err;
        m_nErrorPos = pos;
    }

    int GetReg(token &t) {
        if(t.type == TOKEN_NAME && g_Regs.contains(t.s.toUpper())) {
             return g_Regs.value(t.s.toUpper());
        }

        return -1;
    }

signals:

};

#endif // CASSEMBLER_H

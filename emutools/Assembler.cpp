#include "Assembler.h"
#include "Debugger.h"
#include "QRegularExpression"

enum ERROR_NUMBER
{
    ERROR_100,
    ERROR_101,
    ERROR_102,
    ERROR_103,
    ERROR_104,
    ERROR_105,
    ERROR_106,
    ERROR_107,
    ERROR_108,
    ERROR_109,
    ERROR_110,
    ERROR_111,
    ERROR_112,
    ERROR_113,
    ERROR_114,
    ERROR_115,
    ERROR_116,
    ERROR_117,
    ERROR_118,
    ERROR_119,
    ERROR_120,
    ERROR_121,
    ERROR_122,
    ERROR_123,
    ERROR_124,
    ERROR_125,
    ERROR_126,
    ERROR_127,
    ERROR_128,
    ERROR_129,
    ERROR_130,
    ERROR_131,
    ERROR_132,
    ERROR_133,
    ERROR_134,
    ERROR_135,
};


CAssembler::CAssembler(QObject *parent) : QObject(parent)
{
    Init();
}

void CAssembler::Init()
{
        g_pCPUCommands["HALT"]  = { 0000000,    CPUCmdGroup::NOOPS }; // op
        g_pCPUCommands["WAIT"]  = { 0000001,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["RTI"]   = { 0000002,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["BPT"]   = { 0000003,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["IOT"]   = { 0000004,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["RESET"] = { 0000005,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["RTT"]   = { 0000006,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["START"] = { 0000012,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["STEP"]  = { 0000016,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["NOP"]   = { 0000240,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLC"]   = { 0000241,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLV"]   = { 0000242,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLVC"]  = { 0000243,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLZ"]   = { 0000244,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLZC"]  = { 0000245,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLZV"]  = { 0000246,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLZVC"] = { 0000247,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLN"]   = { 0000250,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLNC"]  = { 0000251,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLNV"]  = { 0000252,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLNVC"] = { 0000253,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLNZ"]  = { 0000254,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLNZC"] = { 0000255,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CLNZV"] = { 0000256,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["CCC"]   = { 0000257,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEC"]   = { 0000261,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEV"]   = { 0000262,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEVC"]  = { 0000263,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEZ"]   = { 0000264,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEZC"]  = { 0000265,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEZV"]  = { 0000266,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEZVC"] = { 0000267,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SEN"]   = { 0000270,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SENC"]  = { 0000271,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SENV"]  = { 0000272,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SENVC"] = { 0000273,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SENZ"]  = { 0000274,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SENZC"] = { 0000275,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SENZV"] = { 0000276,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["SCC"]   = { 0000277,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["RET"]   = { 0000207,    CPUCmdGroup::NOOPS };
        g_pCPUCommands["RETURN"] = { 0000207,    CPUCmdGroup::NOOPS };

        g_pCPUCommands["BR"]    = { 0000400,    CPUCmdGroup::CBRANCH }; // op lll
        g_pCPUCommands["BNE"]   = { 0001000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BEQ"]   = { 0001400,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BGE"]   = { 0002000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BLT"]   = { 0002400,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BGT"]   = { 0003000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BLE"]   = { 0003400,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BPL"]   = { 0100000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BMI"]   = { 0100400,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BHI"]   = { 0101000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BLOS"]  = { 0101400,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BVC"]   = { 0102000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BVS"]   = { 0102400,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BCC"]   = { 0103000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BCS"]   = { 0103400,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BHIS"]  = { 0103000,    CPUCmdGroup::CBRANCH };
        g_pCPUCommands["BLO"]   = { 0103400,    CPUCmdGroup::CBRANCH };

        g_pCPUCommands["MUL"]   = { 0070000,    CPUCmdGroup::EIS }; // op ss,r
        g_pCPUCommands["DIV"]   = { 0071000,    CPUCmdGroup::EIS };
        g_pCPUCommands["ASH"]   = { 0072000,    CPUCmdGroup::EIS };
        g_pCPUCommands["ASHC"]  = { 0073000,    CPUCmdGroup::EIS };

        g_pCPUCommands["EMT"]   = { 0104000,    CPUCmdGroup::TRAP }; // op nnn
        g_pCPUCommands["TRAP"]  = { 0104400,    CPUCmdGroup::TRAP };

        g_pCPUCommands["SOB"]   = { 0077000,    CPUCmdGroup::SOB }; // op r,ll

        g_pCPUCommands["MARK"]  = { 0006400,    CPUCmdGroup::MARK }; // op nn

        g_pCPUCommands["JSR"]   = { 0004000,    CPUCmdGroup::TWOOPREG }; // op r,dd
        g_pCPUCommands["XOR"]   = { 0074000,    CPUCmdGroup::TWOOPREG };

        g_pCPUCommands["FADD"]  = { 0075000,    CPUCmdGroup::FIS }; // op r
        g_pCPUCommands["FSUB"]  = { 0075010,    CPUCmdGroup::FIS };
        g_pCPUCommands["FMUL"]  = { 0075020,    CPUCmdGroup::FIS };
        g_pCPUCommands["FDIV"]  = { 0075030,    CPUCmdGroup::FIS };
        g_pCPUCommands["RTS"]   = { 0000200,    CPUCmdGroup::FIS };

        g_pCPUCommands["PUSH"]  = { 0010046,    CPUCmdGroup::PUSH }; // op ss
        g_pCPUCommands["PUSHB"] = { 0110046,    CPUCmdGroup::PUSH };

        g_pCPUCommands["JMP"]   = { 0000100,    CPUCmdGroup::ONEOPS }; // op dd
        g_pCPUCommands["SWAB"]  = { 0000300,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["MTPS"]  = { 0106400,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["MFPS"]  = { 0106700,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["MTPD"]  = { 0106600,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["MFPD"]  = { 0106500,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["MTPI"]  = { 0006600,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["MFPI"]  = { 0006500,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["SXT"]   = { 0006700,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["CALL"]  = { 0004700,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["POP"]   = { 0012600,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["POPB"]  = { 0112600,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["CLR"]   = { 0005000,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["COM"]   = { 0005100,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["INC"]   = { 0005200,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["DEC"]   = { 0005300,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["NEG"]   = { 0005400,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ADC"]   = { 0005500,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["SBC"]   = { 0005600,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["TST"]   = { 0005700,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ROR"]   = { 0006000,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ROL"]   = { 0006100,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ASR"]   = { 0006200,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ASL"]   = { 0006300,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["CLRB"]  = { 0105000,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["COMB"]  = { 0105100,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["INCB"]  = { 0105200,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["DECB"]  = { 0105300,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["NEGB"]  = { 0105400,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ADCB"]  = { 0105500,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["SBCB"]  = { 0105600,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["TSTB"]  = { 0105700,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["RORB"]  = { 0106000,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ROLB"]  = { 0106100,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ASRB"]  = { 0106200,    CPUCmdGroup::ONEOPS };
        g_pCPUCommands["ASLB"]  = { 0106300,    CPUCmdGroup::ONEOPS };

        g_pCPUCommands["MOV"]   = { 0010000,    CPUCmdGroup::TWOOPS }; // op ss, dd
        g_pCPUCommands["CMP"]   = { 0020000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["BIT"]   = { 0030000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["BIC"]   = { 0040000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["BIS"]   = { 0050000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["MOVB"]  = { 0110000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["CMPB"]  = { 0120000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["BITB"]  = { 0130000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["BICB"]  = { 0140000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["BISB"]  = { 0150000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["ADD"]   = { 0060000,    CPUCmdGroup::TWOOPS };
        g_pCPUCommands["SUB"]   = { 0160000,    CPUCmdGroup::TWOOPS };

        g_pCPUCommands[".WORD"] = { 0000000,    CPUCmdGroup::WORD };

        g_Regs["R0"] = 0;
        g_Regs["R1"] = 1;
        g_Regs["R2"] = 2;
        g_Regs["R3"] = 3;
        g_Regs["R4"] = 4;
        g_Regs["R5"] = 5;
        g_Regs["R6"] = 6;
        g_Regs["R7"] = 7;
        g_Regs["SP"] = 6;
        g_Regs["PC"] = 7;
}

bool CAssembler::AssembleCPUInstruction(const CString &str)
{

    CReader rdr(str);

    rdr.SkipSpaces();
    int opStartPos = rdr.GetCurrPos();

    token t;
    rdr.GetToken(t);
    

    bOperandType = false; // считаем, что по умолчанию - приёмник
    nAriphmType = 1;     // арифметические операции внутри ассемблерной инструкции
    t.s = t.s.toUpper();

   if (t.type == TOKEN_NAME && g_pCPUCommands.contains(t.s)) // если нашли мнемонику
        {
            CPUCommandStruct cmd = g_pCPUCommands[t.s];

            // если нашли нужную мнемонику
            // нужно взять опкод.
            m_ResultData[0] = cmd.nOpcode;
            m_ResultData[1] = 0; // следующие два слова заранее обнулим
            m_ResultData[2] = 0;

            bool bRet = false;
            // и обработать операнды в соответствии с типом группы, к которой принадлежит опкод.
            switch (cmd.nGroup)
            {
                case CPUCmdGroup::NOOPS: // ничего делать не надо.
                    bRet = true;
                    break;

                case CPUCmdGroup::CBRANCH:
                    bRet = assembleBR(rdr);
                    break;

                case CPUCmdGroup::EIS:
                    bRet = assemble2ROP(rdr);
                    break;

                case CPUCmdGroup::TRAP:
                    bRet = assembleTRAP(rdr);
                    break;

                case CPUCmdGroup::SOB:
                    bRet = assembleSOB(rdr);
                    break;

                case CPUCmdGroup::MARK:
                    bRet = assembleMARK(rdr);
                    break;

                case CPUCmdGroup::TWOOPREG:
                    bRet = assemble2OPR(rdr);
                    break;

                case CPUCmdGroup::FIS:
                    bRet = assemble1OPR(rdr);
                    break;

                case CPUCmdGroup::PUSH:
                    bOperandType = true; // меняем тип операнда

                // и переходим к выполнению CPUCmdGroup::ONEOPS
                case CPUCmdGroup::ONEOPS:
                    bRet = assemble1OP(rdr);
                    break;

                case CPUCmdGroup::TWOOPS:
                    bRet = assemble2OP(rdr);
                    break;

                case CPUCmdGroup::WORD:
                    bRet = assembleWORD(rdr);
                    break;
            }
            return bRet;
    }

   OutError(ERROR_107, opStartPos);
   return false;
}

// сборка двухоперандных команд
bool CAssembler::assemble2OP(CReader &rdr)
{
    bOperandType = true; // сперва ss

    if (!assemble1OP(rdr))   // обработаем один операнд
    {
        return false;
    }

    rdr.SkipSpaces();
    char c = rdr.GetChar();

    if(c != ',') {
        OutError(ERROR_128, rdr.GetCurrPos()-1);    // Ошибка в команде - нет второго операнда.
        return false;
    }


    bOperandType = false; // теперь dd
    return assemble1OP(rdr);     // обработаем второй операнд
}

// сборка обычных однооперандных команд
bool CAssembler::assemble1OP(CReader &rdr)
{
    return Operand_analyse(rdr); // разбираемся, что там такое
}

// сборка однооперандных команд, где операнд - регистр
bool CAssembler::assemble1OPR(CReader &rdr)
{
    token t;
    rdr.SkipSpaces();
    int pos = rdr.GetCurrPos();
    rdr.GetToken(t);
    int reg = GetReg(t);
    if (reg >=0 && reg <=7)
    {
        m_ResultData[0] |= reg << 6;
        return true;
    }

    OutError(ERROR_113, pos); // Ошибка в имени регистра.
    return false;
}

// сборка двухоперандных команд, где первый операнд - регистр
bool CAssembler::assemble2OPR(CReader &rdr)
{
    bOperandType = true; // сперва ss

    if (!assemble1OPR(rdr))  // обрабатываем регистр
    {
        return false;
    }

    rdr.SkipSpaces();
    char c = rdr.GetChar();
    if (c != ',')    // проверим наличие второго
    {
        OutError(ERROR_128,rdr.GetCurrPos()-1);    // Ошибка в команде - нет второго операнда.
        return false;
    }

    bOperandType = false; // теперь dd
    return assemble1OP(rdr);     // обработаем второй операнд
}

// сборка двухоперандных команд, где второй операнд - регистр
// Это EIS команды, у которых  при записи мнемоники почему-то поменяны местами
// источник и приёмник, как у x86
bool CAssembler::assemble2ROP(CReader &rdr)
{
    bOperandType = false; // сперва dd

    if (!assemble1OP(rdr))   // обработаем первый операнд
    {
        return false;
    }

    rdr.SkipSpaces();
    char c = rdr.GetChar();
    if (c != ',')    // проверим наличие второго
    {
        OutError(ERROR_128,rdr.GetCurrPos()-1);    // Ошибка в команде - нет второго операнда.
        return false;
    }

    bOperandType = true; // теперь ss
    return assemble1OPR(rdr);    // обработаем второй операнд
}

// сборка команд ветвления
bool CAssembler::assembleBR(CReader &rdr)
{
    token t;
    uint16_t addr;

    int pos = rdr.GetToken(t);

    switch (t.type) {
        case TOKEN_NAME:
            addr = m_pDebugger->m_SymTable.GetAddrForSymbol(t.s);
            break;
        case TOKEN_NUMBER:
            addr = t.n;
            break;
        default:
            return false;
    }


    int offset = ((int)addr - (int)m_nPC) / 2;

    if(offset > - 128 && offset < 127)
    {
        m_ResultData[0] |= (offset & 0xff); // задаём финальное смещение
        return true;
    }
    else
    {
        OutError(ERROR_110, pos); // Ошибка длины перехода по оператору ветвления.
        return false;
    }
}

bool CAssembler::assembleSOB(CReader &rdr)
{
    bOperandType = true;     // сперва ss

    if (!assemble1OPR(rdr))  // обработаем имя регистра
    {
        return false;
    }

    rdr.SkipSpaces();
    char c = rdr.GetChar();
    if (c != ',')    // проверим наличие второго
    {
        OutError(ERROR_124,rdr.GetCurrPos()-1);    // Ошибка в команде - нет второго операнда.
        return false;
    }

    bOperandType = false;    // теперь dd
    token t;

    uint16_t addr;

    int pos = rdr.GetToken(t);

    switch (t.type) {
        case TOKEN_NAME:
            addr = m_pDebugger->m_SymTable.GetAddrForSymbol(t.s);
            break;
        case TOKEN_NUMBER:
            addr = t.n;
            break;
        default:
            return false;
    }

    int offset = ((int)m_nPC - (int)addr) / 2;

    if (offset >= 0 && offset <= 077)     // если смещение < 0,  то всё верно
    {
        m_ResultData[0] |= uint16_t(offset); // формируем опкод
        return true;
    }

    OutError(ERROR_102, pos); // Ошибка длины или направления перехода в команде SOB.
    return false;
}

bool CAssembler::assembleTRAP(CReader &rdr)
{
    int nOp = m_ResultData[0]; // код опкода нужен, чтобы текстошибки правильный вывести

    token t;
    uint16_t trapNum;

    int pos = rdr.GetToken(t);

    switch (t.type) {
        case TOKEN_NAME:
            trapNum = m_pDebugger->m_SymTable.GetAddrForSymbol(t.s);
            break;
        case TOKEN_NUMBER:
            trapNum = t.n;
            break;
        default:
            trapNum = 1000;
    }

    if (trapNum < 0377)
    {
        m_ResultData[0] |= uint16_t(trapNum);
        return true;
    }
    OutError((nOp == 0104400) ? ERROR_125 : ERROR_126, pos); // Ошибка аргумента TRAP, EMT.
    return false;
}

bool CAssembler::assembleMARK(CReader &rdr)
{

    token t;
    uint16_t trapNum;

    int pos = rdr.GetToken(t);

    switch (t.type) {
        case TOKEN_NAME:
            trapNum = m_pDebugger->m_SymTable.GetAddrForSymbol(t.s);
            break;
        case TOKEN_NUMBER:
            trapNum = t.n;
            break;
        default:
            trapNum = 1000;
    }

    if (trapNum < 077)
    {
        m_ResultData[0] |= uint16_t(trapNum);
        return true;
    }


    OutError(ERROR_112, pos); // Ошибка аргумента MARK
    return false;
}

bool CAssembler::assembleWORD(CReader &rdr)
{
    token t;

    int pos = rdr.GetToken(t);

    switch (t.type) {
        case TOKEN_NAME:
            m_ResultData[0] = m_pDebugger->m_SymTable.GetAddrForSymbol(t.s);
            return true;

        case TOKEN_NUMBER:
            m_ResultData[0] = t.n;
            return true;

        default:
            OutError(ERROR_112, pos); // Ошибка аргумента .WORD
            return false;
    }

}

bool CReader::ParseName(token &t)
{
    bool res = true;
    t.type = TOKEN_NAME;

    do {
        t.s += (*curr++);
    } while (isalnum(*curr));


    return res;
}

bool CReader::ParseNumber(token &t)
{
    bool res = true;
    int base = 8;      // Default base is 8!
    int n = 0;
    bool isNeg = false;

    t.type = TOKEN_NUMBER;

    if(*curr == '-') {
        isNeg = true;
        curr++;
    }

    if(*curr == '0') {
        curr++;
        if(*curr == 'x') {
            base = 16;
            curr++;
        } else {
            base = 8;
        }
    }

    switch(base) {
        case 10: {
            while(isdigit(*curr))
                n = n*10 + (*curr++ - '0');
        }
        break;

        case 8: {
            while(*curr >= '0' && *curr <= '7' )
                n = (n << 3) + (*curr++ - '0');
        }
        break;

        case 16: {
            while(isxdigit(*curr)) {
                if (*curr <= '9')
                    n = n*16 + (*curr++ - '0');
                else if (*curr <= 'F')
                    n = n*16 + (*curr++ - 'F');
                else
                    n = n*16 + (*curr++ - 'f');
            }
        }
        break;
    }

    if (isNeg)
        n = -n;

    t.n = n;

    return res;
}

bool CAssembler::Operand_analyse(CReader &rdr)
{
    token t;
    rdr.SkipSpaces();

    int pos;

    int op = 010;
    int reg = 8;
    uint16_t mem = 0;

    bool is_ind = false;
    bool is_mem = false;
    bool is_autoinc = false;
    bool is_autodec = false;
    bool is_direct = false;

// 0 R0
// 1 (R0)
// 2 (R0)+
// 3 @(R0)+
// 4 -(R0)
// 5 @-(R0)
// 6 345(R0)
// 7 @345(R0)

    pos = rdr.GetToken(t);

    if(t.type == TOKEN_UNKNOWN) {
        OutError(ERROR_112, pos);
        return false;
    }

    if (t.type == TOKEN_AT) {
       is_ind = true;  // Will check for memory indirect later
       pos = rdr.GetToken(t);
    }

    if (t.type == TOKEN_MINUS) {
        is_autodec = true;
        pos = rdr.GetToken(t);
    }

    if (t.type == TOKEN_DIRECT) {
        is_direct = true;
        pos = rdr.GetToken(t);
    }


    if(t.type == TOKEN_NUMBER) {
        is_mem = !is_direct;
        mem = t.n;
        pos = rdr.GetToken(t);
    }

    if(t.type == TOKEN_NAME) {
        int treg = GetReg(t);
        if( treg >= 0) {
            // This is a register name
            if (is_ind || is_autodec || is_mem) { // any indirect
                OutError(ERROR_112, pos);  // Incorrect use of register
                return false;
            }

            m_ResultData[0] |= treg << (bOperandType ? 6 : 0);
            return true;
        } else {
            if(m_pDebugger->m_SymTable.Contains(t.s)) {
                 mem = m_pDebugger->m_SymTable.GetAddrForSymbol(t.s);
                 is_mem = !is_direct;
                 pos = rdr.GetToken(t);
            } else {
                OutError(ERROR_112, pos);
                return false;
            }
        }
    }

    // Checking for '('
    if(t.type == TOKEN_LBRACKET) {
        rdr.GetToken(t);
        reg = GetReg(t);
        if(reg < 0) {
            OutError(ERROR_112, pos);  // Invalid register name
            return false;
        }
        rdr.GetToken(t);
        if(t.type != TOKEN_RBRACKET) {
            OutError(ERROR_112, pos);  // No closing ')'
            return false;
        }
        if(rdr.CurrChar() == '+') {
            is_autoinc = true;
        }

    }

    if((is_mem && (is_autodec || is_autoinc)) ||
       (is_autodec && is_autoinc)) {
            OutError(ERROR_112, pos);  // Incorrect modifiers
            return false;
    }

    if(is_direct) {
        op = 020;
        if (reg != 8) {
            OutError(ERROR_112, pos);  // Incorrect use of direct
            return false;
        }
        reg = 7;
    }

    if (is_autoinc) {
        op = 020;
    } else if (is_autodec) {
        op = 040;
    } else if (is_mem){
        op = 060;
        if( reg == 8) {
            reg = 7;
            mem -= m_nPC + m_nPCOff*2;
        }
    }

    if(is_ind)
        op |= 010;

     m_ResultData[0] |= (op | reg) << (bOperandType ? 6 : 0);
     if(is_mem || is_direct)
         m_ResultData[m_nPCOff++] = mem;

     return true;
}


/*********************************** TESTS ************************************/

typedef struct {
    uint16_t startAddr;
    const CString str;
    int16_t size;
    uint16_t data[3];
} asm_tests_t;

asm_tests_t tests[] = {
    { 0002276, "CMP 02340, 02342", 3, {0026767, 0036, 0036} },
    { 0001000, "MOV #010, @#0200", 3, {0012737, 010, 0200}},
    { 0002260, "SOB R1, 002252",   1, {0077104}},
    { 0002266, "BMI 002330",       1, {0100420}},
    { 0002322, "MOV @02340, R5",   2, {0017705, 0012}},
};

void RunAssemblerTests()
{
    int res;
    const uint16_t *buff;
    bool testOk;
    CAssembler casm;

    for (asm_tests_t i : tests) {
        casm.AssembleString(i.str, i.startAddr);
        res = casm.GetResultingBuffer(&buff);
        testOk = true;
        if(res != i.size) {
            testOk = false;
        } else {
            for (int d=0; d< res; d++) {
               if(buff[d] != i.data[d]) {
                   testOk = false;
                   break;
               }
            }
        }

        if(! testOk) {
            printf("Test failed: %s\n", i.str.toLatin1().data());
        } else {
            printf("Test success: %s\n", i.str.toLatin1().data());
        }
            printf("  Got: %d %07o", res, buff[0]);
            if(res > 1)
                printf(" %07o", buff[1]);
            if(res > 2)
                printf(" %07o", buff[2]);
            printf("\n  Tst: %d %07o", res, i.data[0]);
            if(res > 1)
                printf(" %07o", i.data[1]);
            if(res > 2)
                printf(" %07o", i.data[2]);
            printf("\n");
            fflush(stdout);
    }
}

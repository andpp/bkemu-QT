// BreakPoint.cpp: implementation of the CBreakPoint class.
//


#include "pch.h"
#include "BreakPoint.h"
#include "BKMessageBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CBreakPoint::CBreakPoint(uint16_t addr)
	: m_type(BREAKPOINT_ADDRESS)
	, m_breakAddress(addr)
    , m_active(true)
{
}

CBreakPoint::~CBreakPoint()
{
}

int CBreakPoint::SaveBreakpointToBuffer(char *buff)
{
    char *p = buff;
    *p++ = m_type;
    *p++ = m_active;
    *(uint16_t *)p = m_breakAddress;

    return 4;
}

int CBreakPoint::ReadBreakpointFromBuffer(char *buff)
{
    char *p = buff;
    m_active = *p++;
    m_breakAddress = *(uint16_t *)p;

    return 3;
}

CCondBreakPoint::CCondBreakPoint(lua_State *l, uint16_t addr)
    : CBreakPoint(addr)
    , L(l)
    , m_cond("")
    , m_condName("")
{
    m_type = BREAKPOINT_ADDRESS_COND;
    m_re = QRegularExpression("(return )(.*)(end\"]:\\d*:)(.*)");
}

CCondBreakPoint::~CCondBreakPoint()
{

}

void CCondBreakPoint::ShowErrorMsg()
{
    CString err = lua_tostring(L, -1);
    QRegularExpressionMatch match = m_re.match(err);
    QString msg;
    if (match.hasMatch()) {
        msg = match.captured(2);
        msg += "\n" + match.captured(4);
    }
    g_BKMsgBox.Show(msg, MB_OK);

}

bool CCondBreakPoint::SetCond(const CString &cond)
{
    m_condName.CString::Format("f%07o",GetAddress());

    m_cond = cond;

    CString lua_func = "function " + m_condName + "()"
             "  return " + cond +
             " end";

    int status = luaL_dostring(L, lua_func.GetString());
        if (status != 0) {
          ShowErrorMsg();

          m_active = false;
          return false;
        }

    m_active = TestCond();
    return m_active;
}

bool CCondBreakPoint::TestCond()
{
    bool res;
    lua_getglobal(L, m_condName.GetString());
    if (lua_pcall(L, 0, 1, 0) != 0) {
        ShowErrorMsg();
        return false;
    } else {
        /* retrieve result */
        if (lua_isboolean(L, -1) || lua_isnumber(L, -1)) {
            res = true;
        } else {
            g_BKMsgBox.Show("Condition expression must return bool or int", MB_OK);
            res = false;
        }
      lua_pop(L, 1);  /* pop returned value */
      return res;
    }

}

bool CCondBreakPoint::EvaluateCond(void *param)
{
    (void)param;

    if(!m_active)
        return false;

    int res = 0;
    if(m_condName.length()) {
        lua_getglobal(L, m_condName.GetString());
        if (lua_pcall(L, 0, 1, 0) != 0) {
            ShowErrorMsg();
            m_active = false;
        } else {
              /* retrieve result */
              if (lua_isboolean(L, -1)) {
                res = lua_toboolean(L, -1);
              } else if (lua_isnumber(L, -1)) {
                res = lua_tonumber(L, -1);
              } else {
                  TRACE0("Condition expression '%s' must return bool or int", m_cond.GetString());
                  m_active = false;
              }
              lua_pop(L, 1);  /* pop returned value */
        }
    }
    return res > 0;
}

int CCondBreakPoint::SaveBreakpointToBuffer(char *buff)
{
    char *p = buff;
    *p++ = m_type;
    *p++ = m_active;
    *(uint16_t *)p = m_breakAddress;
    p += 2;

    *p++ = m_cond.length() + 1;
    char *c = m_cond.toLocal8Bit().data();
    while (*c) {
        *p++ = *c++;
    }
    *p++ = 0;

    return p - buff;
}

int CCondBreakPoint::ReadBreakpointFromBuffer(char *buff)
{
    char *p = buff;
    m_active = *p++;
    m_breakAddress = *(uint16_t *)p;
    p += 2;

    int len = *p++;
    SetCond(p);
    p += len;

    return p - buff;
}


#ifdef ENABLE_MEM_BREAKPOINT

CMemBreakPoint::CMemBreakPoint(uint16_t beg_addr, uint16_t end_addr, uint accessType)
    : CBreakPoint()
    , m_begAddr(beg_addr)
    , m_endAddr(end_addr)
    , m_value(0xFFFF)
    , m_AccessType(accessType)
{
    m_type = BREAKPOINT_MEMORY_ACCESS;
    m_breakAddress = ((uint32_t)beg_addr << 16) | end_addr;
}

CMemBreakPoint::~CMemBreakPoint()
{

}

bool CMemBreakPoint::EvaluateCond(void *param)
{
    if(!m_active)
        return false;

    MemAccess_t *mem = (MemAccess_t *)param;
    for(int i=0; i< mem->nRead; i++) {
        uint16_t addr = mem->rAddrs[i];
        if(m_begAddr <= addr && addr <= m_endAddr) {
//          TODO:
//            if((cond & m_cond) & BREAKPOINT_MEMACCESS_LESS && value < m_value) {
//                return true;
//            }
//            if((cond & m_cond) & BREAKPOINT_MEMACCESS_EQUAL && value == m_value) {
//                return true;
//            }
//            if((cond & m_cond) & BREAKPOINT_MEMACCESS_GREAT && value > m_value) {
//                return true;
//            }
            if(m_AccessType & BREAKPOINT_MEMACCESS_READ)
                return true;
        }
    }

    for(int i=0; i< mem->nWrite; i++) {
        uint16_t addr = mem->wAddrs[i];
        if(m_begAddr <= addr && addr <= m_endAddr) {
            if(m_AccessType & BREAKPOINT_MEMACCESS_WRITE)
                return true;
        }
    }


    return false;
}

int CMemBreakPoint::SaveBreakpointToBuffer(char *buff)
{
    char *p = buff;
    *p++ = m_type;
    *p++ = m_active;
    *(uint16_t *)p = m_begAddr;
    p +=2;
    *(uint16_t *)p = m_endAddr;
    p +=2;
    *(uint16_t *)p = m_AccessType;

    return 8;
}

int CMemBreakPoint::ReadBreakpointFromBuffer(char *buff)
{
    char *p = buff;
    m_active = *p++;
    m_begAddr = *(uint16_t *)p;
    p += 2;
    m_endAddr = *(uint16_t *)p;
    p += 2;
    m_AccessType = *(uint16_t *)p;

    m_breakAddress = ((uint32_t)m_begAddr << 16) | m_endAddr;

    return 7;
}
#endif

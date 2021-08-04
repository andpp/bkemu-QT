// BreakPoint.cpp: implementation of the CBreakPoint class.
//


#include "pch.h"
#include "BreakPoint.h"

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
}

CCondBreakPoint::~CCondBreakPoint()
{

}

bool CCondBreakPoint::AddCond(const CString &cond)
{
    m_condName.CString::Format("f%07o",GetAddress());

    m_cond = cond;

    CString lua_func = "function " + m_condName + "()"
             "  return " + cond +
             " end";

    int status = luaL_dostring(L, lua_func.GetString());
        if (status != 0) {
          const char *lua_err = lua_tostring(L, -1);
          printf("Load Buffer Error: %s\n\n", lua_err);
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
        CString err = lua_tostring(L, -1);
        TRACE1("error running function `f': %s", lua_tostring(L, -1));
        return false;
    } else {
        /* retrieve result */
        if (lua_isboolean(L, -1) || lua_isnumber(L, -1)) {
            res = true;
        } else {
            TRACE0("Condition expression must return bool or int");
            res = false;
        }
      lua_pop(L, 1);  /* pop returned value */
      return res;
    }

}

bool CCondBreakPoint::EvaluateCond(UINT accessType)
{
    (void)accessType;
    if(!m_active)
        return false;

    int res = 0;
    if(m_condName.length()) {
        lua_getglobal(L, m_condName.GetString());
        if (lua_pcall(L, 0, 1, 0) != 0) {
            CString err = lua_tostring(L, -1);
            TRACE1("error running function `f': %s", lua_tostring(L, -1));
        } else {
              /* retrieve result */
              if (lua_isboolean(L, -1)) {
                res = lua_toboolean(L, -1);
              } else if (lua_isnumber(L, -1)) {
                res = lua_tonumber(L, -1);
              } else {
                  TRACE0("Condition expression must return bool or int");
              }
              lua_pop(L, 1);  /* pop returned value */
        }
    }
    return res > 0;
}

bool CCondBreakPoint::RemoveCond()
{
    return true;
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
    AddCond(p);
    p += len;

    return p - buff;
}


CMemBreakPoint::CMemBreakPoint(uint16_t beg_addr, uint16_t end_addr, UINT accessType)
    : CBreakPoint(beg_addr)
    , m_begAddr(beg_addr)
    , m_endAddr(end_addr)
    , m_accessType(accessType)
{
    m_type = BREAKPOINT_MEMORY_ACCESS;
}

CMemBreakPoint::~CMemBreakPoint()
{

}

bool CMemBreakPoint::EvaluateCond(UINT accessType)
{
    if(!m_active)
        return false;

    if(accessType & m_accessType) {
        return true;
    }

    return false;
}

int CMemBreakPoint::SaveBreakpointToBuffer(char *buff)
{
    char *p = buff;
    *p++ = m_type;
    *p++ = m_active;
    *(uint16_t *)p = m_breakAddress;

    return 4;
}

int CMemBreakPoint::ReadBreakpointFromBuffer(char *buff)
{
    char *p = buff;
    m_active = *p++;
    m_breakAddress = *(uint16_t *)p;

    return 3;
}

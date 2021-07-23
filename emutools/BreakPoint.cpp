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


CBreakPoint::CBreakPoint()
	: m_type(BREAKPOINT_ADDRESS)
	, m_breakAddress(0177777)
{
}

CBreakPoint::CBreakPoint(uint16_t addr)
	: m_type(BREAKPOINT_ADDRESS)
	, m_breakAddress(addr)
{
}

CBreakPoint::~CBreakPoint()
{
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
          return false;
        }

    return TestCond();
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
    if(accessType & m_accessType) {
        return true;
    }

    return false;
}

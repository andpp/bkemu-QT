// BreakPoint.h: interface for the CBreakPoint class.
//


#pragma once

#include "pch.h"
#include <QList>

#include "lua.hpp"

enum
{
	BREAKPOINT_ADDRESS = 1,
	BREAKPOINT_MEMORY_ACCESS = 2
};


class CBreakPoint
{
		UINT                m_type;
		uint16_t            m_breakAddress;
	public:
		CBreakPoint();
		CBreakPoint(uint16_t addr);
		virtual ~CBreakPoint();

		inline bool         IsAddress()
		{
			return (m_type == BREAKPOINT_ADDRESS);
		}

		inline UINT         GetType()
		{
			return m_type;
		}

		inline uint16_t     GetAddress()
		{
			return m_breakAddress;
		}

        virtual bool AddCond(const CString &cond) { (void)cond; return true;}
        virtual bool EvaluateCond() { return true;}
        virtual bool RemoveCond() { return true;}

};

class CCondBreakPoint : public CBreakPoint
{
        lua_State *L;
        CString m_cond;
        CString m_condName;

    public:
        CCondBreakPoint(uint16_t addr = 0177777);
        CCondBreakPoint(lua_State *l, uint16_t addr = 0177777);
        virtual ~CCondBreakPoint();

        virtual bool AddCond(const CString &cond);
        virtual bool EvaluateCond();
        virtual bool RemoveCond();
};

using CBreakPointList = QList<CBreakPoint*>;

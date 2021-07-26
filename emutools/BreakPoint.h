// BreakPoint.h: interface for the CBreakPoint class.
//


#pragma once

#include "pch.h"
#include <QMap>

#include "lua.hpp"

enum
{
    BREAKPOINT_ADDRESS       = 001,
    BREAKPOINT_ADDRESS_COND  = 002,
    BREAKPOINT_MEMORY_ACCESS = 004
};

enum
{
    BREAKPOINT_MEMACCESS_READ  = 001,
    BREAKPOINT_MEMACCESS_WRITE = 002,
};


class CBreakPoint
{
    protected:
		UINT                m_type;
		uint16_t            m_breakAddress;
        bool                m_active;
	public:
        CBreakPoint(uint16_t addr = 0177777);
		virtual ~CBreakPoint();

		inline bool         IsAddress()
		{
            return (m_type & (BREAKPOINT_ADDRESS | BREAKPOINT_ADDRESS_COND) );
		}

		inline UINT         GetType()
		{
			return m_type;
		}

		inline uint16_t     GetAddress()
		{
			return m_breakAddress;
		}

        inline bool         IsActive()
        {
            return m_active;
        }

        virtual void SetActive(bool activate) {  m_active = activate; }
        virtual bool AddCond(const CString &cond) { (void)cond; return true; }
        virtual bool EvaluateCond(UINT accessType = 0) { (void)accessType; return m_active; }
        virtual bool RemoveCond() { return true;}
        virtual bool AddrWithingRange(uint16_t addr) { (void)addr; return false; }

};

class CCondBreakPoint : public CBreakPoint
{
        lua_State *L;
        CString m_cond;
        CString m_condName;

    public:
        CCondBreakPoint(lua_State *l, uint16_t addr = 0177777);
        virtual ~CCondBreakPoint();

        virtual void SetActive(bool activate) {
            m_active = activate ? TestCond() : false;
        }
        virtual bool AddCond(const CString &cond);
        virtual bool EvaluateCond(UINT accessType = 0);
        virtual bool RemoveCond();

        const CString& GetCond() {return m_cond; }
    private:
        bool TestCond();
};

class CMemBreakPoint : public CBreakPoint
{
    uint16_t m_begAddr;
    uint16_t m_endAddr;
    UINT m_accessType;

    public:
        CMemBreakPoint(uint16_t beg_addr = 0177777, uint16_t end_addr = 0177777,
                       UINT accessType = BREAKPOINT_MEMACCESS_READ | BREAKPOINT_MEMACCESS_WRITE);
        virtual ~CMemBreakPoint();

        virtual bool AddCond(const CString &cond) { (void)cond; return true; }
        virtual bool EvaluateCond(UINT accessType = 0);
        virtual bool RemoveCond() { return true; }
        virtual bool AddrWithingRange(uint16_t addr) {
            return addr >= m_begAddr && addr <= m_endAddr;
        }
};

using CBreakPointList = QMap<uint32_t, CBreakPoint*>;

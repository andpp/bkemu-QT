// BreakPoint.h: interface for the CBreakPoint class.
//


#pragma once

#include "pch.h"
#include <QMap>

#include "lua.hpp"
#include "MemBreakPointStruct.h"

enum
{
    BREAKPOINT_ADDRESS       = 001,
    BREAKPOINT_ADDRESS_COND  = 002,
    BREAKPOINT_MEMORY_ACCESS = 004
};

enum
{
    BREAKPOINT_MEMACCESS_READ    = 001,
    BREAKPOINT_MEMACCESS_WRITE   = 002,
    BREAKPOINT_MEMACCESS_LESS    = 004,
    BREAKPOINT_MEMACCESS_GREAT   = 010,
    BREAKPOINT_MEMACCESS_EQUAL   = 020,
    BREAKPOINT_MEMACCESS_CHANGED = 040,
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

        static constexpr uint16_t HDR_MAGIC = 0x0AF0;

		inline bool         IsAddress()
		{
            return (m_type & (BREAKPOINT_ADDRESS | BREAKPOINT_ADDRESS_COND) );
		}

        inline bool         IsMemory()
        {
            return (m_type & BREAKPOINT_MEMORY_ACCESS);
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

        void SetActive(bool activate) {  m_active = activate; }
        bool AddCond(const CString &cond) { (void)cond; return true; }
        virtual bool EvaluateCond(void *param = nullptr) { (void)param; return m_active; }
        virtual int SaveBreakpointToBuffer(char *buff);
        virtual int ReadBreakpointFromBuffer(char *buff);
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
        bool SetCond(const CString &cond);
        const CString& GetCond() {return m_cond; }
        virtual bool EvaluateCond(void *param = nullptr);
        virtual int SaveBreakpointToBuffer(char *buff);
        virtual int ReadBreakpointFromBuffer(char *buff);

    private:
        bool TestCond();
};

#ifdef ENABLE_MEM_BREAKPOINT

class CMemBreakPoint : public CBreakPoint
{
    uint16_t m_begAddr;
    uint16_t m_endAddr;
    uint16_t m_value;
    uint m_cond;

    public:
        CMemBreakPoint(uint16_t beg_addr = 0177777, uint16_t end_addr = 0177777,
                       uint accessType = BREAKPOINT_MEMACCESS_READ | BREAKPOINT_MEMACCESS_WRITE);
        virtual ~CMemBreakPoint();

        bool SetCond(uint16_t cond) { m_cond = cond; return true; }
        virtual bool EvaluateCond(void *param = nullptr);
        bool RemoveCond() { return true; }
        virtual bool AddrWithingRange(uint16_t addr) {
            return addr >= m_begAddr && addr <= m_endAddr;
        }
        virtual int SaveBreakpointToBuffer(char *buff);
        virtual int ReadBreakpointFromBuffer(char *buff);
};

using CMemBreakPointList = QMap<uint32_t, CMemBreakPoint*>;

#endif

using CBreakPointList = QMap<uint32_t, CBreakPoint*>;

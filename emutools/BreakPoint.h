// BreakPoint.h: interface for the CBreakPoint class.
//


#pragma once

#include "pch.h"
#include <QList>

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
};

using CBreakPointList = QList<CBreakPoint>;

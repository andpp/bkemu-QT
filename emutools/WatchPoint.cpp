#include "WatchPoint.h"

CWatchPoint::CWatchPoint(uint16_t addr, uint16_t size, uint type)
  : m_nAddr(addr)
  , m_nSize(size)
  , m_nType(type)
  , m_bShowSymbolName(true)
{

}

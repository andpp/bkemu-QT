#ifndef CWATCHPOINT_H
#define CWATCHPOINT_H

#include <QMap>

enum  {
    WPTYPE_WORD = 0,
    WPTYPE_BYTE,
    WPTYPE_STRING,
    WPTYPE_POINTER,
    WPTYPE_ANY = 255
};

class CWatchPoint
{
    friend class CWatchpointView;
    friend class CWatchpointEdit;

protected:
    uint16_t m_nAddr;
    uint16_t m_nSize;
    uint16_t m_nType;

    bool     m_bShowSymbolName;
public:
    CWatchPoint(uint16_t addr, uint16_t size = 2, uint type = WPTYPE_WORD);

    uint16_t GetType() {
        return m_nType;
    }

    void     ShowSymbol(bool b) {
        m_bShowSymbolName = b;
    }
};

using CWatchPointList = QMultiMap<uint16_t, CWatchPoint*>;

#endif // CWATCHPOINT_H

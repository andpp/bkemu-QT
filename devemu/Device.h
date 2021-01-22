// Device.h: interface for the CDevice class.
//
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <QObject>

#define ULONGLONG uint64_t

class CDevice : public QObject
{
	protected:
		ULONGLONG           m_tickCount; // Device ticks
		CDevice            *m_pParent;

	public:
		CDevice();
		virtual ~CDevice() override;

		void AttachBoard(CDevice *pParent)
		{
			m_pParent = pParent;
		};
		// Method for count device ticks
		void                Reset();
		virtual void        NextTick();

		// Virtual method called after reset command
        virtual void        OnReset() = 0;

		// Methods for Set/Get byte/word
        virtual void        GetByte(uint16_t addr, uint8_t *pValue) = 0;
        virtual void        GetWord(uint16_t addr, uint16_t *pValue) = 0;
        virtual void        SetByte(uint16_t addr, uint8_t value) = 0;
        virtual void        SetWord(uint16_t addr, uint16_t value) = 0;
};


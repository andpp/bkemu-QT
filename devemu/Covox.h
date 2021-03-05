// Covox.h: interface for the CCovox class.
//


#pragma once

#include "BKSoundDevice.h"

class CCovox : public CBKSoundDevice
{
	public:
		CCovox();
		virtual ~CCovox() override;
		virtual void        ReInit() override;

		virtual void        SetData(uint16_t inVal) override;
		virtual void        GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR) override;
};

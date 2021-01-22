// Covox.cpp: implementation of the CCovox class.
//


#include "pch.h"
#include "Covox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CCovox::CCovox()
{
	ReInit();
}

CCovox::~CCovox()
{
}

void CCovox::ReInit()
{
	CalcFIR(m_pH, FIR_LENGTH, 8000.0, 0.0, FIR_FILTER::LOWPASS);
}

void CCovox::SetSample(uint16_t inVal)
{
	if (m_bEnableSound)
	{
		m_dLeftAcc  = double(LOBYTE(inVal)) / 256.0;
		m_dRightAcc = double(HIBYTE(inVal)) / 256.0;
	}
	else
	{
		m_dLeftAcc = 0.0;
		m_dRightAcc = 0.0;
	}
}

void CCovox::GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR)
{
	sampleL = m_dLeftAcc;
	sampleR = m_dRightAcc;
//  sampleL = DCOffset(sampleL, m_dAvgL, m_pdBufferL, m_nBufferPosL);
//  sampleR = DCOffset(sampleR, m_dAvgR, m_pdBufferR, m_nBufferPosR);
	// фильтр
	sampleL = FIRFilter(sampleL, m_LeftBuf, m_nLeftBufPos);
	sampleR = FIRFilter(sampleR, m_RightBuf, m_nRightBufPos);

	if (!m_bStereo)
	{
		sampleR = sampleL;
	}
}

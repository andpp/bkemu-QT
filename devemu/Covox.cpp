// Covox.cpp: implementation of the CCovox class.
//


#include "pch.h"
#include "Config.h"
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
	if (CreateFIRBuffers(FIR_LENGTH))
	{
		ReInit();
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}

CCovox::~CCovox()
{
}

void CCovox::ReInit()
{
	double w0 = 2 * 11000.0 / double(g_Config.m_nSoundSampleRate);
	double w1 = 0.0;
    fir_linphase(m_nFirLength, w0, w1, FIR_FILTER::LOWPASS,
	                       FIR_WINDOW::BLACKMAN_HARRIS, true, 0.0, m_pH);
}

void CCovox::SetData(uint16_t inVal)
{
	if (m_bEnableSound)
	{
		m_dLeftAcc  = double(LOBYTE(inVal)) / 256.0;
		m_dRightAcc = m_bStereo ? double(HIBYTE(inVal)) / 256.0 : m_dLeftAcc;
	}
	else
	{
		m_dLeftAcc = m_dRightAcc = 0.0;
	}
}

void CCovox::GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR)
{
	sampleL = m_dLeftAcc;
	sampleR = m_dRightAcc;

	if (m_bDCOffset)
	{
		sampleL = DCOffset(sampleL, m_dAvgL, m_pdBufferL, m_nBufferPosL);
		sampleR = DCOffset(sampleR, m_dAvgR, m_pdBufferR, m_nBufferPosR);
	}

	sampleL = FIRFilter(sampleL, m_pLeftBuf, m_nLeftBufPos);
	sampleR = FIRFilter(sampleR, m_pRightBuf, m_nRightBufPos);
}

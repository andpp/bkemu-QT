/*  This file is part of BKBTL.
    BKBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    BKBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
BKBTL. If not, see <http://www.gnu.org/licenses/>. */

// SoundGen.cpp
//

#include "pch.h"
#include "BKSound.h"
#include "BKMessageBox.h"
#include "MainWindow.h"
#include "Tape.h"

#include "LockVarType.h"

// сколько будет вообще буферов
constexpr auto BKSOUND_BLOCK_COUNT = 8;

/*
* сколько звуковых фреймов в секунду
* 100 - это количество звуковых фреймов в секунду, которое может нормально обрабатываться waveOut;
* если оставить 50 - буфер получается слишком большой, и латентность заметна;
* если делать больше 100 - буфер получается слишком маленький, и очень много ресурсов уходит на заполнение
* и отправку данных в waveOut.
*/
constexpr auto SOUND_FRAMERATE = 100;

// коэффициент увеличения длины буфера для устройств, работающих со звуком
constexpr auto FRAMESIZE_EXP = 4;

CBkSound::CBkSound()
	: m_bSoundGenInitialized(false)
	, m_dSampleL(0.0)
	, m_dSampleR(0.0)
	, m_dFeedbackL(0.0)
	, m_dFeedbackR(0.0)
	, m_nWaveCurrentBlock(0)
    , m_nBufCurPos(0)
//	, m_pWaveBlocks(nullptr)
//    , m_pBufferIO(nullptr)
    , m_bCaptureProcessed(false)
	, m_bCaptureFlag(false)
    , m_nWaveLength(0)
#if (DCOFFSET_1)
	, m_nBufferPosL(0)
	, m_nBufferPosR(0)
	, m_dAvgL(0.0)
	, m_dAvgR(0.0)
#endif
#if (BKSYNCHRO_SEMAPHORE)
//	, m_hSem(CreateSemaphore(nullptr, BKSOUND_BLOCK_COUNT, BKSOUND_BLOCK_COUNT, nullptr))
#endif
{
#if (DCOFFSET_1)
	m_pdBufferL = new double[DCOFFSET_BUFFER_LEN1];
	m_pdBufferR = new double[DCOFFSET_BUFFER_LEN1];

	if (m_pdBufferL && m_pdBufferR)
	{
		for (int i = 0; i < DCOFFSET_BUFFER_LEN1; ++i)
		{
			m_pdBufferL[i] = m_pdBufferR[i] = 0.0;
		}
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

#endif

	Init(0x7fff);
}

CBkSound::~CBkSound()
{
	UnInit();
#if (DCOFFSET_1)
	SAFE_DELETE_ARRAY(m_pdBufferL);
	SAFE_DELETE_ARRAY(m_pdBufferR);
#endif
#if (BKSYNCHRO_SEMAPHORE)
//	CloseHandle(m_hSem);
#endif
}

int CBkSound::ReInit()
{
	if (m_nSoundSampleRate != g_Config.m_nSoundSampleRate)
	{
		uint16_t vol = SoundGen_GetVolume();
		UnInit();
		Init(vol);
	}

// для остальных устройств не обязательно иметь очень мелкий буфер, можно и побольше.
// главное, соблюдать кратность.
// и на выходе - длина буфера в сэмплах.
	return m_nSoundSampleRate * FRAMESIZE_EXP / SOUND_FRAMERATE;
}

void CBkSound::Init(uint16_t volume)
{
	m_nSoundSampleRate = g_Config.m_nSoundSampleRate;   // текущая частота дискретизации
	m_nBufSize = (m_nSoundSampleRate * static_cast<int>(SAMPLE_IO_BLOCKALIGN)) / SOUND_FRAMERATE;  // размер звукового буфера в байтах
	m_nBufSizeInSamples = m_nBufSize / SAMPLE_IO_SIZE; // размер буфера в сэмплах (в сумме для всех каналов)
	SoundGen_Initialize(volume);
}

void CBkSound::UnInit()
{
	SoundGen_Finalize();
}

void CBkSound::SoundGen_SetVolume(uint16_t volume)
{
    if (m_bSoundGenInitialized)
    {
        alSourcef(m_nSource, AL_GAIN, volume/65535.0);
    }
}

uint16_t CBkSound::SoundGen_GetVolume()
{
    if (m_bSoundGenInitialized)
    {
        ALfloat volume;
        alGetSourcef(m_nSource, AL_GAIN, &volume);
        return volume * 65535;
    }

    return 0;
}


void CBkSound::SoundGen_Initialize(uint16_t volume)
{
    if (!m_bSoundGenInitialized)
    {

        volatile ALCenum error;

        m_audioOutput = alcOpenDevice(nullptr);
        if (!m_audioOutput)
            return;

        // Create context
        m_audioContext = alcCreateContext(m_audioOutput, NULL);
        if (m_audioContext == NULL)
        {
           alcCloseDevice(m_audioOutput);
           return;
        }

        // Set active context
        if (!alcMakeContextCurrent(m_audioContext))
        {
           alcDestroyContext(m_audioContext);
           alcCloseDevice(m_audioOutput);
           return;
        }

        error = alGetError();

        m_pBuffer = (ALuint *)malloc(BKSOUND_BLOCK_COUNT * sizeof(ALuint));
        m_mBufferPull = (SAMPLE_IO**)malloc(BKSOUND_BLOCK_COUNT * sizeof(SAMPLE_IO*));

        alGenSources((ALuint)1, &m_nSource);
        // Set the default volume
        alSourcef(m_nSource, AL_GAIN, 1);
        alSourcef(m_nSource, AL_PITCH, 1);
        alSource3f(m_nSource, AL_POSITION, 0, 0, 0);
        alSource3f(m_nSource, AL_VELOCITY, 0, 0, 0);
        alSourcei(m_nSource, AL_LOOPING, AL_FALSE);

        alGenBuffers(BKSOUND_BLOCK_COUNT, m_pBuffer);
        error = alGetError();

        m_nFrequency = m_nSoundSampleRate;
        m_nFormat = AL_FORMAT_STEREO16;

        for(int i=0; i< BKSOUND_BLOCK_COUNT; i++) {
            m_mBufferPull[i] =  (SAMPLE_IO *)malloc(m_nBufSize*2);
            memset(m_mBufferPull[i], 0, m_nBufSize*2);
        }
        m_nWaveCurrentBlock = 0;
        m_mBufferIO = m_mBufferPull[0];
        m_nWaveFreeBlockCount = BKSOUND_BLOCK_COUNT;

        alSourcef(m_nSource, AL_GAIN, volume/65535.0);

        StartAudioThread();
        m_bSoundGenInitialized = true;
    }
}

void CBkSound::SoundGen_Finalize()
{
    if (m_bSoundGenInitialized)
    {
        m_bSoundGenInitialized = false;

        StopAudioThread();
        alDeleteSources(1, &m_nSource);
        alDeleteBuffers(BKSOUND_BLOCK_COUNT, m_pBuffer);
        auto device = alcGetContextsDevice(m_audioContext);
        alcMakeContextCurrent(NULL);
        alcDestroyContext(m_audioContext);
        alcCloseDevice(device);

        for (int i=0; i<BKSOUND_BLOCK_COUNT; i++) {
            free(m_mBufferPull[i]);
        }
        free(m_mBufferPull);
        free(m_pBuffer);

        m_mBufferPull = nullptr;

//#if (BKSYNCHRO_SEMAPHORE)

//		for (int i = 0; i < BKSOUND_BLOCK_COUNT; i++)
//		{
//			WaitForSingleObject(m_hSem, INFINITE);
//		}

//		LONG lPrevCount;
//		ReleaseSemaphore(m_hSem, BKSOUND_BLOCK_COUNT, &lPrevCount);
//#else

//		while (m_nWaveFreeBlockCount < BKSOUND_BLOCK_COUNT)
//		{
//			Sleep(10);
//		}

//#endif

    }
}

#if (!BKSYNCHRO_SEMAPHORE)
// статические переменные для каллбака
std::mutex      CBkSound::m_mutCS;
volatile int    CBkSound::m_nWaveFreeBlockCount = 0;
#endif


void CBkSound::StopAudioThread()
{
//	MSG msg;

    if (!m_bKillThreadEvent)
    {
        alSourceStop(m_nSource);

        m_bKillThreadEvent = true;
        while (m_bKillThreadEvent) // ждём завершения потока
        {

            Sleep(10);
        }
        // поток остановился, и можно продолжать остальные действия по остановке
    }
}

bool CBkSound::StartAudioThread()
{
    m_AudioThread = std::thread(&CBkSound::AudioThreadFunc, this);

    if (m_AudioThread.joinable())
    {
        m_AudioThread.detach();
        return true;
    }

    g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
    return false;
}

void CBkSound::AudioThreadFunc()
{
    ALint iBuffersProcessed;
    ALuint uiBuffer;

    // Change the state to play
    alSourcePlay(m_nSource);

    do {
        ALint state;
        alGetSourcei(m_nSource, AL_SOURCE_STATE, &state);
        if(state != AL_PLAYING) {
            alSourcePlay(m_nSource);
        }
        alGetSourcei(m_nSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
        if(iBuffersProcessed == 0) {
            Sleep(1);
            continue;
        } else {
            // Unqueue buffer
            alSourceUnqueueBuffers(m_nSource, 1, &uiBuffer);
            InterlockedIncrement(&m_nWaveFreeBlockCount);
        }
    } while (!m_bKillThreadEvent);       // пока не придёт событие остановки

    m_bKillThreadEvent = false;
}

void CBkSound::SoundGen_FeedDAC_Mixer(SAMPLE_INT &L, SAMPLE_INT &R)
{
    L = m_dSampleL;
    R = m_dSampleR;
#if (DCOFFSET_1)
    L = DCOffset(L, m_dAvgL, m_pdBufferL, m_nBufferPosL); // на выходе m_nBufferPosL указывает на следующую позицию.
    R = DCOffset(R, m_dAvgL, m_pdBufferL, m_nBufferPosL); // на выходе m_nBufferPosL указывает на следующую позицию.
#endif

    if (m_bSoundGenInitialized)
    {
        m_mBufferIO[m_nBufCurPos++] = static_cast<SAMPLE_IO>(L * FLOAT_BASE);
        m_mBufferIO[m_nBufCurPos++] = static_cast<SAMPLE_IO>(R * FLOAT_BASE);

        if (m_nBufCurPos >= m_nBufSizeInSamples)
        {
            m_nBufCurPos = 0;
            // если свободных блоков нет, подождём.
//#if (BKSYNCHRO_SEMAPHORE)
//            WaitForSingleObject(m_hSem, INFINITE);
//#else

            while ((volatile int32_t)m_nWaveFreeBlockCount <= 0)
            {
//                Sleep(1, TRUE);
            }

//            // резервируем очередной блок
            InterlockedDecrement(&m_nWaveFreeBlockCount);
//            {
//                m_mutCS.lock();
//                m_nWaveFreeBlockCount--;
//                m_mutCS.unlock();
//            }
//#endif
            // отправляем звучать заполненный блок
            alBufferData(m_pBuffer[m_nWaveCurrentBlock], m_nFormat, m_mBufferPull[m_nWaveCurrentBlock], m_nBufSize, m_nFrequency);
            alSourceQueueBuffers(m_nSource, 1, &m_pBuffer[m_nWaveCurrentBlock]);

            if (m_bCaptureFlag)
            {
                WriteToCapture();
            }

            // переходим циклически к следующему блоку
            if (++m_nWaveCurrentBlock >= BKSOUND_BLOCK_COUNT)
            {
                m_nWaveCurrentBlock = 0;
            }

            // и инициализируем указатель на заполняемый блок.
            m_mBufferIO = reinterpret_cast<SAMPLE_IO *>(m_mBufferPull[m_nWaveCurrentBlock]);
        }
    }
}

#if (DCOFFSET_1)
// автоматическое выравнивание уровня
SAMPLE_INT CBkSound::DCOffset(register SAMPLE_INT sample, register SAMPLE_INT &fAvg, register SAMPLE_INT *pBuf, register int &nBufPos)
{
	/*
	идея такая:
	avg0 = (buf[0] + buf[1] + ... + buf[buf_len-1]) / buf_len

	avg1 = avg0 - buf[buf_len-1] / buf_len + sample / buf_len

	не нужно сдвигать содержимое буфера, выкидывая последнее значение и добавляя в начало текущее.
	и не нужно каждый раз суммировать всё содержимое буфера, для расчёта среднего значения.
	имеем указатель на позицию в буфере, прочитанное значение из буфера - это выкидываемый последний сэмпл
	а занесение нового значения - запись текущего сэмпла в буфер на заданную позицию.
	*/
	fAvg += (sample - pBuf[nBufPos]) / SAMPLE_INT(DCOFFSET_BUFFER_LEN1);
	pBuf[nBufPos++] = sample;
	nBufPos &= DCOFFSET_BUFFER_MASK1;
	// устраняем клиппинг, может появляться из-за резкой смены знака амплитуды
	register SAMPLE_INT t = sample - fAvg;

	if (t < 0.0)
	{
		t = -t;
	}

	if (t > SAMPLE_INT(MAX_SAMPLE) / FLOAT_BASE)
	{
		return sample + fAvg;
	}
	else
	{
		return sample - fAvg;
	}
}
#endif

void CBkSound::SoundGen_ResetSample(SAMPLE_INT L, SAMPLE_INT R)
{
	m_dSampleL = L;
	m_dSampleR = R;
}

void CBkSound::SoundGen_SetSample(SAMPLE_INT &L, SAMPLE_INT &R)
{
	m_dSampleL = L;
	m_dSampleR = R;
}

void CBkSound::SoundGen_MixSample(SAMPLE_INT &L, SAMPLE_INT &R)
{
	register SAMPLE_INT t = L * m_dSampleL;
	m_dSampleL += L;

	if (t > 0.0) // если множители были одного знака
	{
		if (L > 0.0) // если оба положительные
		{
			m_dSampleL -= t;
		}
		else // если оба отрицательные
		{
			m_dSampleL += t;
		}
	}

	// если разного или 0, то просто сумма
	t = R * m_dSampleR;
	m_dSampleR += R;

	if (t > 0.0) // если множители были одного знака
	{
		if (R > 0.0) // если оба положительные
		{
			m_dSampleR -= t;
		}
		else // если оба отрицательные
		{
			m_dSampleR += t;
		}
	}

	// если разного или 0, то просто сумма
}


void CBkSound::SetCaptureStatus(bool bCapture, const CString &strUniq)
{
	if (bCapture)
	{
		PrepareCapture(strUniq);
	}
	else
	{
		CancelCapture();
	}
}

void CBkSound::PrepareCapture(const CString &strUniq)
{
	if (m_bCaptureProcessed)
	{
		CancelCapture();
	}

	CString strWavExt;
	strWavExt.LoadString(IDS_FILEEXT_WAVE);
	CString strName = g_Config.m_strScreenShotsPath + _T("capture") + strUniq + strWavExt;

	if (!m_waveFile.Open(strName, CFile::modeCreate | CFile::modeReadWrite))
	{
		return;
	}

	m_nWaveLength = 0;
	DataHeader dataHeader;
	dataHeader.dataTag = DATA_TAG;
	dataHeader.dataSize = static_cast<DWORD>(m_nWaveLength) * m_wfx.nBlockAlign;
	WaveHeader waveHeader;
	waveHeader.riffTag = RIFF_TAG;
	waveHeader.size = sizeof(WaveHeader) + sizeof(WAVEFORMATEX) + sizeof(DataHeader) + dataHeader.dataSize;
	waveHeader.waveTag = WAVE_TAG;
	waveHeader.fmtTag = FMT_TAG;
	waveHeader.fmtSize = sizeof(WAVEFORMATEX);
	m_waveFile.Write(&waveHeader, sizeof(WaveHeader));
	m_waveFile.Write(&m_wfx, waveHeader.fmtSize);
	m_waveFile.Write(&dataHeader, sizeof(DataHeader));
	m_bCaptureProcessed = true;
	m_bCaptureFlag = true;
}

void CBkSound::CancelCapture()
{
	if (m_bCaptureProcessed)
	{
		m_mutCapture.lock();
//		m_waveFile.SeekToBegin();
		DataHeader dataHeader;
		dataHeader.dataTag = DATA_TAG;
		dataHeader.dataSize = static_cast<DWORD>(m_nWaveLength) * m_wfx.nBlockAlign;
		WaveHeader waveHeader;
		waveHeader.riffTag = RIFF_TAG;
		waveHeader.size = sizeof(WaveHeader) + sizeof(WAVEFORMATEX) + sizeof(DataHeader) + dataHeader.dataSize;
		waveHeader.waveTag = WAVE_TAG;
		waveHeader.fmtTag = FMT_TAG;
		waveHeader.fmtSize = sizeof(WAVEFORMATEX);
		m_waveFile.Write(&waveHeader, sizeof(WaveHeader));
		m_waveFile.Write(&m_wfx, waveHeader.fmtSize);
		m_waveFile.Write(&dataHeader, sizeof(DataHeader));
		m_waveFile.Close();
		m_bCaptureProcessed = false;
		m_bCaptureFlag = false;
		m_mutCapture.unlock();
	}
}

void CBkSound::WriteToCapture()
{
	m_mutCapture.lock();

	if (m_bCaptureProcessed)
	{
//		m_waveFile.Write(m_pWaveBlocks[m_nWaveCurrentBlock].lpData, m_nBufSize);
		m_nWaveLength += m_nBufSize;
	}

	m_mutCapture.unlock();
}



#if (!BKSYNCHRO_SEMAPHORE)
// статические переменные для каллбака
std::mutex      CBkSound::m_mutCS;
volatile int    CBkSound::m_nWaveFreeBlockCount = 0;
#endif

//void CALLBACK CBkSound::WaveCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
//{
//	if (uMsg == WOM_DONE)
//	{
//#if (BKSYNCHRO_SEMAPHORE)
//		ReleaseSemaphore(reinterpret_cast<HANDLE>(dwInstance), 1, nullptr);
//#else
//		auto freeBlockCounter = reinterpret_cast<int *>(dwInstance);
//		m_mutCS.lock();
//		(*freeBlockCounter)++;
//		// свободный блок появился
//		m_mutCS.unlock();
//#endif
//	}
//}



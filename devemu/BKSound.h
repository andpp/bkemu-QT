/*  This file is part of BKBTL.
    BKBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    BKBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
BKBTL. If not, see <http://www.gnu.org/licenses/>. */

// SoundGen.h
//

#pragma once

#define DIRECTSOUND_VERSION 0x1000

//#include <Mmsystem.h>
#include <QAudioOutput>
#include <QBuffer>
#include "Config.h"
#include <mutex>
#include <QMutex>

#include "LockVarType.h"

// способ синхронизации
// 1 - через семафор (нагрузка на процессор меньше)
// 0 - через переменную-счётчик
#define BKSYNCHRO_SEMAPHORE 1

// Включение выравнивания осмещения пост. тока
// 1 - вкл.
// 0 - выкл.
#define DCOFFSET_1 0

#if (DCOFFSET_1)
constexpr auto DCOFFSET_BUFFER_MASK1 = 0x7fff; // степень двойки минус 1
constexpr auto DCOFFSET_BUFFER_LEN1 = (DCOFFSET_BUFFER_MASK1 + 1); // степень двойки
#endif // endif

#define WAVEHDR short
#define HWAVEOUT short

class CFifo : public QIODevice
{
    Q_OBJECT

public:
    CFifo(int size);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 bytesAvailable() const override;
    bool start();
    void close() override;

    qint64 getSize() const;

    bool push(SAMPLE_IO sample);
    bool push(SAMPLE_IO *buff, uint size);
    bool pushTimeout(SAMPLE_IO sample, ulong ms_timeout);

private:
//    void generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate);
//    LockVarType m_Lock;
    mutable QMutex m_Lock;

private:
    volatile qint64 m_head;
    volatile qint64 m_tail;
    qint64 m_size;
    QByteArray m_buffer;
};

class CBkSound
{
	protected:
        bool            m_bSoundGenInitialized;

		SAMPLE_INT      m_dSampleL, m_dSampleR;
		SAMPLE_INT      m_dFeedbackL, m_dFeedbackR;
		uint32_t        m_nWaveCurrentBlock;
		uint32_t        m_nBufCurPos;
		uint32_t        m_nBufSize;             // размер звукового буфера в байтах
		uint32_t        m_nBufSizeInSamples;    // размер звукового буфера в сэмплах
		int             m_nSoundSampleRate;
//		SAMPLE_IO      *m_mBufferIO;            // указатель на текущий заполняемый буфер воспроизведения
        SAMPLE_IO      *m_mBufferPull;          // указатель на пулл буфферов
//        QAudioOutput   *m_pAudio;
//        QScopedPointer<Generator> m_generator;
        QScopedPointer<QAudioOutput> m_audioOutput;
        QScopedPointer<CFifo>        m_pBufferIO;
        QByteArray     *m_pBuffer;
        WAVEFORMATEX    m_wfx;
#if (DCOFFSET_1)
		// переменные для вычисления смещения DC.
		int             m_nBufferPosL, m_nBufferPosR;
		SAMPLE_INT      m_dAvgL, m_dAvgR;
		SAMPLE_INT     *m_pdBufferL;
		SAMPLE_INT     *m_pdBufferR;
#endif

#if (BKSYNCHRO_SEMAPHORE)
//		const HANDLE    m_hSem;
#else
		static std::mutex       m_mutCS;
		static volatile int     m_nWaveFreeBlockCount;
#endif
		// захват звука
		bool            m_bCaptureProcessed;
		bool            m_bCaptureFlag;
		std::mutex      m_mutCapture;
		CFile           m_waveFile;
		size_t          m_nWaveLength;

	public:
		CBkSound();
		virtual ~CBkSound();
		int             ReInit();
		void            SoundGen_SetVolume(uint16_t volume);
		uint16_t        SoundGen_GetVolume();
		void            SoundGen_ResetSample(SAMPLE_INT L, SAMPLE_INT R);
		void            SoundGen_SetSample(SAMPLE_INT &L, SAMPLE_INT &R);
		void            SoundGen_MixSample(SAMPLE_INT &L, SAMPLE_INT &R);
		void            SoundGen_FeedDAC_Mixer(SAMPLE_INT &L, SAMPLE_INT &R);
		void            SetCaptureStatus(bool bCapture, const CString &strUniq);
		bool            IsCapture()
		{
			return m_bCaptureProcessed;
		}
	protected:
		void            SoundGen_Initialize(uint16_t volume);
		void            SoundGen_Finalize();

		void            Init(uint16_t volume);
		void            UnInit();

		void            PrepareCapture(const CString &strUniq);
		void            CancelCapture();
		void            WriteToCapture();
#if (DCOFFSET_1)
		SAMPLE_INT      DCOffset(register SAMPLE_INT sample, register SAMPLE_INT &fAvg, register SAMPLE_INT *pBuf, register int &nBufPos);
#endif
//		static void CALLBACK WaveCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
};

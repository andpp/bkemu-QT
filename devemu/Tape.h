// Tape.h: interface for the CTape class.
//


#pragma once
#include "pch.h"

#include "BKSound_Defines.h"
//#include <mmsystem.h>

// "RIFF"
constexpr DWORD RIFF_TAG = 0x46464952;
// "fmt "
constexpr DWORD FMT_TAG = 0x20746d66;
// "data"
constexpr DWORD DATA_TAG = 0x61746164;
// "WAVE"
constexpr DWORD WAVE_TAG = 0x45564157;

#pragma pack(push)
#pragma pack(1) // envelope period (envT, R13-R14) has invalid word alignment
struct TAPE_FILE_INFO
{
	DWORD               start_tuning;
	DWORD               synchro_start;
	DWORD               marker1;
	DWORD               synchro_header;
	DWORD               header;
	uint16_t            address;
	uint16_t            length;
	uint8_t             name[16];
	DWORD               marker2;
	DWORD               synchro_data;
	DWORD               data;
	uint16_t            crc;
	DWORD               synchro6;
	DWORD               end_tuning;
	DWORD               end;
};
#pragma pack(pop)

struct WaveHeader
{
	DWORD               riffTag;
	DWORD               size;
	DWORD               waveTag;
	DWORD               fmtTag;
	DWORD               fmtSize;
};

struct DataHeader
{
	DWORD               dataTag;
	DWORD               dataSize;
};

constexpr auto MAX_TABLE_SIZE = 65000;

class CTape
{
		WAVEFORMATEX    m_WorkingWFX;       // текущие рабочие парамметры wave

		bool            m_bPlay;
		bool            m_bWaveLoaded;
		SAMPLE_INT     *m_pWave;            // собственно массив wave.
		int             m_nWaveMaxLen;      // размер массива m_pWave в сэмплах
		int             m_nWaveLength;      // размер данных в массиве m_pWave в сэмплах (может быть меньше m_nWaveMaxLen)
		int             m_nPlayPos;         // текущая позиция воспроизведения в сэмплах

		bool            m_bRecord;          // флаг записи
		SAMPLE_INT     *m_pRecord;          // массив, куда записывается звук
		int             m_nRecordPos;       // текущая позиция в массиве в сэмплах
		int             m_nRecordLength;    // длина массива в сэмплах

		bool            m_bAutoBeginRecord;
		bool            m_bAutoEndRecord;

		uint8_t        *m_pBin;             // массив бинарных данных, в которые преобразуются wav и tap
		int             m_nPos;
		SAMPLE_INT      m_nAverage;
		int             m_nAvgLength;
		double          m_dAvgLength;
		bool            m_bInverse;

		uint16_t       *m_pScanTable[MAX_TABLE_SIZE]; // массив для упаковки/распаковки
		int             m_nScanTableSize;

		// функции упаковки/распаковки wav в tap
		void            InitTables();
		void            ClearTables();
		int             TableLookup_Fast(uint8_t *pPackBits, uint16_t prefix, int end_pos);
		void            AddWord_Fast(uint8_t *pPackBits, uint16_t prefix, int end_pos);
		bool            UnpackWord(uint8_t *pPackBits, int &nPackBitsLength, uint16_t code);
		// функции преобразования модулированного wav в bin
		bool            FindTuning(int nLength, DWORD &wave_pos, int &wave_length);
		bool            FindMarker();
		bool            FindMarker1();
		bool            FindSyncro6();
		int             CalcImpLength(SAMPLE_INT *pWave, int &pos, int length);
		inline SAMPLE_INT GetCurrentSampleMono(register SAMPLE_INT *pWave, register int pos);
		void            SetCurrentSampleMono(SAMPLE_INT *pWave, int pos, SAMPLE_INT sample);
		int             DefineLength(int length);
		bool            ReadBit(bool &bBit);
		bool            ReadByte(uint8_t &byte);
		// функции преобразования bin в модулированный wav
		bool            SaveTuning(int length);
		bool            SaveSyncro6();
		bool            SaveImp(int size);
		bool            SaveBit(bool bBit);
		bool            SaveByte(uint8_t byte);

		int             ConvertSamples(WAVEFORMATEX wfx_in, void *inBuf, UINT nBufSize);
		void            ResampleBuffer(int nSrcSSR, int nDstSSR);
		bool            FindRecordBegin(int buffLength);
		bool            FindRecordEnd(int buffLength);
		void            CalculateAverage();

	public:
		CTape();
		virtual ~CTape();
		void            SetWaveParam(int nWorkingSSR = DEFAULT_SOUND_SAMPLE_RATE, int nWorkingChn = BUFFER_CHANNELS);
		int             GetWorkingSSR()
		{
			return m_WorkingWFX.nSamplesPerSec;
		}
		int             GetWorkingChannels()
		{
			return m_WorkingWFX.nChannels;
		}
		// Wave pack/unpack methods
		void            PackBits(uint8_t *pPackBits, int nPackBitsLength);
		void            UnpackBits(uint8_t *pPackBits, int nPackBitsLength);

		int             PackLZW_Fast(uint8_t *pPackBits, int nPackBitsLength, uint16_t *pPackLZW);
		void            UnpackLZW_Fast(uint8_t *pPackBits, int nPackBitsLength, uint16_t *pPackLZW, int nPackLZWLength);

		// Wave read/write methods
		bool            SetWaveFile(TAPE_FILE_INFO *pTfi);
		bool            GetWaveFile(TAPE_FILE_INFO *pTfi, bool bHeaderOnly = false);
		uint16_t        CalcCRC(TAPE_FILE_INFO *pTfi);

		bool            LoadWaveFile(const CString &strPath);
		bool            SaveWaveFile(const CString &strPath);

        bool            LoadMP3File(const CString &strPath);

		bool            LoadBinFile(const CString &strPath, TAPE_FILE_INFO *pTfi);
		bool            SaveBinFile(const CString &strPath, TAPE_FILE_INFO *pTfi);

		bool            LoadMSFFile(const CString &strPath, bool bSilent = false);
		bool            SaveMSFFile(const CString &strPath);

		bool            LoadTmpFile(const CString &strPath);

		// Wave buffer methods
		bool            AllocWaveBuffer(int nLenInSamples);
		bool            LoadBuffer(SAMPLE_INT *pBuff, int nLenInSamples);
		inline int      GetWaveLength()
		{
			return m_nWaveLength;
		}
		inline SAMPLE_INT *GetWaveBuffer()
		{
			return m_pWave;
		}

		// Playing methods
		inline int      GetPlayWavePos()
		{
			return m_nPlayPos;
		}

		void            PlayWaveGetBuffer(SAMPLE_INT *pBuff, int nBufSampleLen);

		inline void     ResetPlayWavePos()
		{
			m_nPlayPos = 0;
		}
		inline void     StartPlay()
		{
			m_bPlay = true;
		}
		inline void     StopPlay()
		{
			m_bPlay = false;
		}
		inline bool     IsPlaying()
		{
			return m_bPlay;
		}
		inline void     SetWaveLoaded(bool b)
		{
			m_bWaveLoaded = b;
		}

		inline bool     IsWaveLoaded()
		{
			return m_bWaveLoaded;
		}

		void            StartRecord(bool bAutoBeginRecord, bool bAutoEndRecord);
		void            RecordWaveGetBuffer(SAMPLE_INT *pBuff, int nBufSampleLen);
		void            StopRecord();

		inline bool     IsRecording()
		{
			return m_bRecord;
		}

};

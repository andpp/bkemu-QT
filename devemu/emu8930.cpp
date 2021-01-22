/****************************************************************************

emu8930.cpp -- AY-3-8930 emulator
прототип. Не рабочий.
Надо доделать:
1. Реализацию Duty Cycle для расширенного режима
2. Реализацию различного функционирования 8910 и 8930 в режиме совместимости с 8910
3. Уточнить таблицы громкостей.
4. Оптимизировать.

*****************************************************************************/
#include "pch.h"
#include "emu8930.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

constexpr auto GETA_BITS = 24;

const double dutycycle[9] =
{
	3.125,  // 0
	6.25,   // 1
	12.50,  // 2
	25.00,  // 3
	50.00,  // 4
	75.00,  // 5
	87.50,  // 6
	93.75,  // 7
	96.875  // 8
};

CEMU8930::CEMU8930()
	: m_bExpandedMode(false)
	, m_nExpandedBank(0)
	, m_nClock(0)
	, m_nRate(0)
	, m_nSynthReg(0xff)
{
	ReInit();
}

CEMU8930::~CEMU8930()
{
}

void CEMU8930::ReInit()
{
	PSG_init(g_Config.m_nSoundChipFrequency * 2, g_Config.m_nSoundSampleRate);
	PSG_reset(); // использует значения, заданные в PSG_init
	PSG_setVolumeMode();
	CalcFIR(m_pH, FIR_LENGTH, 13000.0, 0.0, FIR_FILTER::LOWPASS);
}
void CEMU8930::PSG_init(int c, int r)
{
	m_nClock = c;
	m_nRate = r;
	PSG_set_quality(true);
}

void CEMU8930::PSG_set_quality(bool q)
{
	m_bHQ = q;

	if (m_bHQ && m_nRate) // если включено высшее качество и задана частота дискретизации
	{
		m_nBaseIncrement = 1 << GETA_BITS;
		m_dRealStep = 1.0 / double(m_nRate);
		m_dPSGStep = 16.0 / double(m_nClock); // m_dPSGStep = 1.0 / double(m_nClock >> 4);
	}
	else
	{
		// если нет чего-то одного из этого, то режим HQ невозможен
		m_bHQ = false;
		m_dRealStep = 0.0;
		m_dPSGStep = 0.0;

		if (m_nRate)
		{
			m_nBaseIncrement = int(double(1 << GETA_BITS) * double(m_nClock) / double(m_nRate << 4));
		}
		else
		{
			m_nBaseIncrement = int(double(1 << GETA_BITS) * double(m_nClock) / double(1 << 4));
		}
	}
}

// таблица громкости для обычного режима должна быть как для оригинального AY,
// а для расширенного - пока будем использовать ямаховскую
const int CEMU8930::m_voltbl[2][32] =
{
	{
		0x0000, 0x0000, 0x0340, 0x0340, 0x04C0, 0x04C0, 0x06F2, 0x06F2, 0x0A44, 0x0A44, 0x0F13, 0x0F13, 0x1510, 0x1510, 0x227E, 0x227E,
		0x289F, 0x289F, 0x414E, 0x414E, 0x5B21, 0x5B21, 0x7258, 0x7258, 0x905E, 0x905E, 0xB550, 0xB550, 0xD7A0, 0xD7A0, 0xFFFF, 0xFFFF
	},
	{
		0x0000, 0x0000, 0x00EF, 0x01D0, 0x0290, 0x032A, 0x03EE, 0x04D2, 0x0611, 0x0782, 0x0912, 0x0A36, 0x0C31, 0x0EB6, 0x1130, 0x13A0,
		0x1751, 0x1BF5, 0x20E2, 0x2594, 0x2CA1, 0x357F, 0x3E45, 0x475E, 0x5502, 0x6620, 0x7730, 0x8844, 0xA1D2, 0xC102, 0xE0A2, 0xFFFF
	}
};

void CEMU8930::PSG_setVolumeMode()
{
	m_Channel[CHAN_A].pVolume = &g_Config.m_A_V;
	m_Channel[CHAN_A].pPanL = &g_Config.m_A_L;
	m_Channel[CHAN_A].pPanR = &g_Config.m_A_R;
	m_Channel[CHAN_B].pVolume = &g_Config.m_B_V;
	m_Channel[CHAN_B].pPanL = &g_Config.m_B_L;
	m_Channel[CHAN_B].pPanR = &g_Config.m_B_R;
	m_Channel[CHAN_C].pVolume = &g_Config.m_C_V;
	m_Channel[CHAN_C].pPanL = &g_Config.m_C_L;
	m_Channel[CHAN_C].pPanR = &g_Config.m_C_R;

	int idx = m_bExpandedMode ? MODEL_YM2149 : MODEL_AY_3_8910;

	for (int i = 0; i < 32; ++i)
	{
		m_vols[i] = double(m_voltbl[idx][i]) / double(0x10000 * SOUND_CHANNELS);
	}
}

void CEMU8930::PSG_reset()
{
	m_nBaseCount = 0;
	m_dPSGTime = 0;
	m_nAddr = 0;
	// чистим массив регистров
	memset(m_reg, 0, 32);

	for (auto &i : m_Channel)
	{
		i.nCount = 0;
		i.nVolume = 0;
		i.nFreq = 1;
		i.bEnv = false;
		i.bToneToggle = false;
		i.bNoiseMask = true;
		i.bToneMask = true;
		i.nDutyCycle = 4;
		i.nEnvPtr = 0;
		i.nDEnv = 0;
		i.nEnvFreq = 1;
		i.nEnvCount = 0;
		i.bEnvAttack = false;
		i.bEnvLValue = false;
		i.bEnvHold = false;
		i.bEnvAlt = false;
	}

	m_nNoiseSeed = 1;
	m_nNoiseValue = 0;
	m_nNoiseCount = 0;
	m_nNoiseFreq = 1;
	m_nNoiseORMask = 0;
	m_nNoiseANDMask = -1;
	PSG_setVolumeMode();
}

uint8_t CEMU8930::PSG_readIO()
{
	return m_reg[m_nAddr];
}

uint8_t CEMU8930::PSG_readReg(int r)
{
	return m_reg[r & 0x0F];
}

void CEMU8930::PSG_writeIO(uint32_t nAddr, uint8_t val)
{
	if (nAddr & 1)   // если адрес регистра нечётный
	{
		PSG_writeReg(m_nAddr, val);    // то запись значения по адресу, определяемому 4-мя мл. битами
	}
	else
	{
		m_nAddr = val & 0x0f;    // а иначе - значение и есть адрес, который потом используется в PSG_readIO
	}

	// это наверное какой-то алгоритм, используемый в Z80
}

void CEMU8930::PSG_writeReg(uint32_t rg, uint8_t val)
{
	rg &= 0x0F;
	uint32_t t = (1 << rg);

	if (m_bExpandedMode)
	{
		if ((t & ((1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8))) && (m_nExpandedBank == 1))
		{
			val &= 0x0F;
		}
		else if ((t & ((1 << 8) | (1 << 9) | (1 << 10))) && (m_nExpandedBank == 0))
		{
			val &= 0x3F;
		}

		if (m_nExpandedBank == 1)
		{
			rg += 16; // второй банк регистров
		}

		m_reg[rg] = val;

		switch (rg)
		{
			// 0,2,4 - младшие 8 бит частоты каналов A,B,C
			// 1,3,5 - старшие 8 бит частоты каналов A,B,C
			case 0:
			case 1:
				m_Channel[CHAN_A].nFreq = m_RExpand.fA;
				break;

			case 2:
			case 3:
				m_Channel[CHAN_B].nFreq = m_RExpand.fB;
				break;

			case 4:
			case 5:
				m_Channel[CHAN_C].nFreq = m_RExpand.fC;
				break;

			case 6:
				m_nNoiseFreq = val;   // 0..ff
				break;

			// регистр управления смесителем, 0 - разрешено, 1 - запрещено
			// биты 3..1 - флаг разрешения тона каналов C,B,A
			// биты 6..4 - флаг разрешения шума каналов C,B,A
			// биты 8..7 - флаг ввода/вывода в порты B,A: 0 - in, 1 - out
			/*
			на AY8910 если регистром-миксером отключить тон и шум то при записи
			потоковых данных в регистр громкости AY превращается в 4-х битный
			ЦАП. таким образом на нем проигрывают оцифрованные записи.
			Если в регистре громкости выставить в 1 пятый бит, то на выход
			попрёт форма огибающей, чем успешно пользуются музыканты.
			т.е. если в регистре громкости 020, то огибающая будет звучать
			независимо от того, включён ли канал в миксере.
			В AY8930 эту особенность судя по всему не учли.
			*/
			case 7:
				m_Channel[CHAN_A].bToneMask  = !!(val & (1 << 0)); // тон false - включён, true - выключен
				m_Channel[CHAN_B].bToneMask  = !!(val & (1 << 1));
				m_Channel[CHAN_C].bToneMask  = !!(val & (1 << 2));
				m_Channel[CHAN_A].bNoiseMask = !!(val & (1 << 3)); // шум false - включён, true - выключен
				m_Channel[CHAN_B].bNoiseMask = !!(val & (1 << 4));
				m_Channel[CHAN_C].bNoiseMask = !!(val & (1 << 5));
				break;

			// амплитуда каналов A,B,C, мл 4 бита - амплитуда, бит 5 - V - флаг выбора волнового пакета,
			case 8:
			case 9:
			case 10:
				rg -= 8;
				m_Channel[rg].bEnv = !!(val & 0x20);
				m_Channel[rg].nVolume = (val & 0x01f);
				break;

			// 11 - младшие 8 бит периода волнового пакета канала A
			// 12 - старшие 8 бит периода волнового пакета канала A
			case 11:
			case 12:
				m_Channel[CHAN_A].nEnvFreq = int(m_RExpand.envTA);
				break;

			/*форма волнового пакета канала A
			бит 4 - флаг цикличности, 1 - действует в зависимости от бита 1, один цикл или бесконечно ,
			0 - в конце цикла счётчик сбросится в 0 и зафиксирует это значение
			бит 3 - атака, 1 - возрастание, счёт от 0000 до 1111, 0 - затухание, счёт от 1111 до 0000
			бит 2 - alternate, 1 - счётчик огибающей меняет направление счёта каждый цикл.
			бит 1 - удержание, 1 - ограничивается одним циклом, зафиксировав последнее значение счётчика
			огибающей, 0000 или 1111, зависит от того, какой был отсчёт, по восходящей или по нисходящей
			*/
			case 13:
			case 29:
				// проверяем режим
			{
				bool bPrevMode = m_bExpandedMode;
				int nMode = (val >> 4) & 0xf;

				if ((nMode >> 1) == 5)
				{
					m_bExpandedMode = true;
					m_nExpandedBank = nMode & 1;
				}
				else
				{
					m_bExpandedMode = false;
				}

				if (bPrevMode != m_bExpandedMode)
				{
					PSG_reset();
				}
			}

			rg = 0;
			goto exp_volpack;

			case 14:
			case 15:
			default:
				break;

			// 0b - младшие 8 бит периода волнового пакета канала B
			// 1b - старшие 8 бит периода волнового пакета канала B
			case 16:
			case 17:
				m_Channel[CHAN_B].nEnvFreq = int(m_RExpand.envTB);
				break;

			// 2b - младшие 8 бит периода волнового пакета канала C
			// 3b - старшие 8 бит периода волнового пакета канала C
			case 18:
			case 19:
				m_Channel[CHAN_C].nEnvFreq = int(m_RExpand.envTC);
				break;

			// 4b - форма волнового пакета канала B
			// 5b - форма волнового пакета канала C
			case 20:
			case 21:
				rg -= 19;
exp_volpack:
				val &= 0xf;

				if (val < 4)
				{
					val = 9;
				}
				else if (val < 8)
				{
					val = 15;
				}

				m_Channel[rg].nEnvCount = 0;
				m_Channel[rg].bEnvHold = !!(val & 1);
				m_Channel[rg].bEnvAlt  = !!(val & 2);
				m_Channel[rg].bEnvAttack = !!(val & 4);
				m_Channel[rg].bEnvLValue = m_Channel[rg].bEnvAttack ^ m_Channel[rg].bEnvAlt;
				m_Channel[rg].bEnvAlt = m_Channel[rg].bEnvAlt && !m_Channel[rg].bEnvHold;

				if (m_Channel[rg].bEnvAttack)
				{
					m_Channel[rg].nEnvPtr = 0;
					m_Channel[rg].nDEnv = 1;
				}
				else
				{
					m_Channel[rg].nEnvPtr = 0x1f;
					m_Channel[rg].nDEnv = -1;
				}

				break;

			// 6b, 7b, 8b - duty cycle ch. A,B,C
			case 22:
			case 23:
			case 24:
				rg -= 22;
				m_Channel[rg].nDutyCycle = (val <= 8) ? val : 8;
				break;

			case 25:
				m_nNoiseANDMask = val;
				break;

			case 26:
				m_nNoiseORMask = val;
				break;
		}
	}
	else
	{
		// режим совместимости
		if (t & ((1 << 1) | (1 << 3) | (1 << 5)))
		{
			val &= 0x0F;
		}
		else if (t & ((1 << 6) | (1 << 8) | (1 << 9) | (1 << 10)))
		{
			val &= 0x1F;
		}

		// помещаем значение в массив
		m_reg[rg] = val;

		// а теперь обработаем логически
		switch (rg)
		{
			// 0,2,4 - младшие 8 бит частоты каналов A,B,C
			// 1,3,5 - старшие 4 бита частоты каналов A,B,C
			case 0:
			case 1:
				m_Channel[CHAN_A].nFreq = m_RLegacy.fA;
				break;

			case 2:
			case 3:
				m_Channel[CHAN_B].nFreq = m_RLegacy.fB;
				break;

			case 4:
			case 5:
				m_Channel[CHAN_C].nFreq = m_RLegacy.fC;
				break;

			// регистр генератор шума
			case 6:
				m_nNoiseFreq = (val << 1) + 1;   // 0..3e
				break;

			// регистр управления смесителем, 0 - разрешено, 1 - запрещено
			// биты 3..1 - флаг разрешения тона каналов C,B,A
			// биты 6..4 - флаг разрешения шума каналов C,B,A
			// биты 8..7 - флаг ввода/вывода в порты B,A: 0 - in, 1 - out
			/*
			    на AY8910 если регистром-миксером отключить тон и шум то при записи
			    потоковых данных в регистр громкости AY превращается в 4-х битный
			    ЦАП. таким образом на нем проигрывают оцифрованные записи.
			    Если в регистре громкости выставить в 1 пятый бит, то на выход
			    попрёт форма огибающей, чем успешно пользуются музыканты.
			    т.е. если в регистре громкости 020, то огибающая будет звучать
			    независимо от того, включён ли канал в миксере.
			    В AY8930 эту особенность судя по всему не учли.
			*/
			case 7:
				m_Channel[CHAN_A].bToneMask  = !!(val & (1 << 0)); // тон false - включён, true - выключен
				m_Channel[CHAN_B].bToneMask  = !!(val & (1 << 1));
				m_Channel[CHAN_C].bToneMask  = !!(val & (1 << 2));
				m_Channel[CHAN_A].bNoiseMask = !!(val & (1 << 3)); // шум false - включён, true - выключен
				m_Channel[CHAN_B].bNoiseMask = !!(val & (1 << 4));
				m_Channel[CHAN_C].bNoiseMask = !!(val & (1 << 5));
				break;

			// амплитуда каналов A,B,C, мл 4 бита - амплитуда, бит 5 - V - флаг выбора волнового пакета,
			case 8:
			case 9:
			case 10:
				rg -= 8;
				m_Channel[rg].bEnv = !!(val & 0x10);
				m_Channel[rg].nVolume = ((val & 0x0f) << 1) + 1;
				break;

			// 11 - младшие 8 бит периода волнового пакета
			// 12 - старшие 8 бит периода волнового пакета
			case 11:
			case 12:
				m_Channel[CHAN_A].nEnvFreq = int(m_RLegacy.envT);
				break;

			/*форма волнового пакета
			    бит 4 - флаг цикличности, 1 - действует в зависимости от бита 1, один цикл или бесконечно ,
			        0 - в конце цикла счётчик сбросится в 0 и зафиксирует это значение
			    бит 3 - атака, 1 - возрастание, счёт от 0000 до 1111, 0 - затухание, счёт от 1111 до 0000
			    бит 2 - alternate, 1 - счётчик огибающей меняет направление счёта каждый цикл.
			    бит 1 - удержание, 1 - ограничивается одним циклом, зафиксировав последнее значение счётчика
			    огибающей, 0000 или 1111, зависит от того, какой был отсчёт, по восходящей или по нисходящей
			*/
			case 13:
				// проверяем режим
			{
				bool bPrevMode = m_bExpandedMode;
				int nMode = (val >> 4) & 0xf;

				if ((nMode >> 1) == 5)
				{
					m_bExpandedMode = true;
					m_nExpandedBank = nMode & 1;
				}
				else
				{
					m_bExpandedMode = false;
				}

				if (bPrevMode != m_bExpandedMode)
				{
					PSG_reset();
				}
			}

			val &= 0xf;

			if (val < 4)
			{
				val = 9;
			}
			else if (val < 8)
			{
				val = 15;
			}

			m_Channel[CHAN_A].nEnvCount = 0;
			m_Channel[CHAN_A].bEnvHold = !!(val & 1);
			m_Channel[CHAN_A].bEnvAlt  = !!(val & 2);
			m_Channel[CHAN_A].bEnvAttack = !!(val & 4);
			m_Channel[CHAN_A].bEnvLValue = m_Channel[CHAN_A].bEnvAttack ^ m_Channel[CHAN_A].bEnvAlt;
			m_Channel[CHAN_A].bEnvAlt = m_Channel[CHAN_A].bEnvAlt && !m_Channel[CHAN_A].bEnvHold;

			if (m_Channel[CHAN_A].bEnvAttack)
			{
				m_Channel[CHAN_A].nEnvPtr = 0;
				m_Channel[CHAN_A].nDEnv = 1;
			}
			else
			{
				m_Channel[CHAN_A].nEnvPtr = 0x1f;
				m_Channel[CHAN_A].nDEnv = -1;
			}

			break;

			case 14:
			case 15:
			default:
				break;
		}
	}
}

void CEMU8930::calc()
{
	m_dMixL = 0.0;
	m_dMixR = 0.0;
	m_nBaseCount += m_nBaseIncrement;
	register int incr = (m_nBaseCount >> GETA_BITS);
	m_nBaseCount &= ((1 << GETA_BITS) - 1);

	if (m_bExpandedMode)
	{
		/* Считаем шум  m_nNoiseCount должен считать 250 кГц*/
		register bool bNoiseToggle;
		m_nNoiseCount += incr;

		if (m_nNoiseCount >= m_nNoiseFreq)
		{
			m_nNoiseCount = 0;

			if (--m_nNoiseValue <= 0)
			{
				register uint32_t bit0x3 = (m_nNoiseSeed ^ (m_nNoiseSeed >> 2)) & 1;
				m_nNoiseSeed = (m_nNoiseSeed | (bit0x3 << 16)) >> 1;
				m_nNoiseValue = (m_nNoiseSeed & m_nNoiseANDMask | m_nNoiseORMask) & 0xff;
				bNoiseToggle = true;
			}
			else
			{
				bNoiseToggle = false;
			}
		}

		for (auto &i : m_Channel)
		{
			// и тут ещё с частотой тона непонятно. 16 битная это полноценная
			// или 16 битная ~~ 12 битная << 4
			/* Считаем огибающую волнового пакета*/
			i.nEnvCount += incr;

			if (i.nEnvCount >= i.nEnvFreq)
			{
				i.nEnvCount = 0;
				i.nEnvPtr += i.nDEnv;

				if (i.nEnvPtr & ~0x1f) // Цикл кончился?
				{
					if (i.bEnvAlt)  // если бит 2
					{
						// меняем направление счёта
						i.nDEnv = -i.nDEnv;
						i.nEnvPtr += i.nDEnv;
					}
					else
					{
						// или начинаем заново
						i.nEnvPtr &= 0x1f;
					}

					if (i.bEnvHold) // если бит 1 - то прекращаем счёт
					{
						i.nDEnv = 0;
						i.nEnvPtr = (i.bEnvLValue) ? 0x1f : 0;
					}
				}
			}

			// рассчитаем Duty Cycle
			// вот примерно так, это для тона.
			i.frq[0] = i.nFreq * 2 * dutycycle[i.nDutyCycle] / 100.0;
			i.frq[1] = i.nFreq * 2 - i.frq[0];
			i.phase = 0;
			i.frq_t = i.frq[i.phase];

			if (++i.nCount >= i.frq_t)
			{
				i.nCount = 0;
				++i.phase &= 1;
				i.frq_t = i.frq[i.phase];

				if (i.nFreq)
				{
					i.bToneToggle = !i.bToneToggle;
				}
			}

			// для волпака я думаю, точно так же как и для тона.
			i.nCount += incr;

			if (i.nCount >= i.nFreq)
			{
				i.nCount = 0;

				if (i.nFreq)
				{
					i.bToneToggle = !i.bToneToggle;
				}
			}

			register bool isON = ((i.bToneToggle || i.bToneMask) && (bNoiseToggle || i.bNoiseMask));

			if (isON)
			{
				register int en = i.bEnv ? i.nEnvPtr : i.nVolume;
				register double v = m_vols[en] * (*i.pVolume) * (*i.pPanL);
				m_dMixL += v;
				v = m_vols[en] * (*i.pVolume) * (*i.pPanR);
				m_dMixR += v;
			}
		}
	}
	else
	{
		// TODO: реализовать различия функционирования в режиме совместимости
		// между 8930 и 8910
		// данные для волпака берутся фиксированно из данных канала A расширенного режима.
		/* Считаем огибающую волнового пакета*/
		m_Channel[CHAN_A].nEnvCount += incr;

		if (m_Channel[CHAN_A].nEnvCount >= m_Channel[CHAN_A].nEnvFreq)
		{
			m_Channel[CHAN_A].nEnvCount = 0;
			m_Channel[CHAN_A].nEnvPtr += m_Channel[CHAN_A].nDEnv;

			if (m_Channel[CHAN_A].nEnvPtr & ~0x1f) // Цикл кончился?
			{
				if (m_Channel[CHAN_A].bEnvAlt)  // если бит 2
				{
					// меняем направление счёта
					m_Channel[CHAN_A].nDEnv = -m_Channel[CHAN_A].nDEnv;
					m_Channel[CHAN_A].nEnvPtr += m_Channel[CHAN_A].nDEnv;
				}
				else
				{
					// или начинаем заново
					m_Channel[CHAN_A].nEnvPtr &= 0x1f;
				}

				if (m_Channel[CHAN_A].bEnvHold) // если бит 1 - то прекращаем счёт
				{
					m_Channel[CHAN_A].nDEnv = 0;
					m_Channel[CHAN_A].nEnvPtr = (m_Channel[CHAN_A].bEnvLValue) ? 0x1f : 0;
				}
			}
		}

		/* Считаем шум  m_nNoiseCount должен считать частоту 125кГц*/
		m_nNoiseCount += incr;

		if (m_nNoiseCount >= m_nNoiseFreq)
		{
			m_nNoiseCount = 0;
			register uint32_t bit0x3 = (m_nNoiseSeed ^ (m_nNoiseSeed >> 2)) & 1;
			m_nNoiseSeed |= (bit0x3 << 16);
			m_nNoiseSeed >>= 1;
		}

		register bool bNoiseToggle = !!(m_nNoiseSeed & 2);

		for (auto &i : m_Channel)
		{
			i.nCount += incr;

			if (i.nCount >= i.nFreq)
			{
				i.nCount = 0;

				if (i.nFreq)
				{
					i.bToneToggle = !i.bToneToggle;
				}
			}

			register bool isON = ((i.bToneToggle || i.bToneMask) && (bNoiseToggle || i.bNoiseMask));

			if (isON)
			{
				register int en = i.bEnv ? m_Channel[CHAN_A].nEnvPtr : i.nVolume;
				register double v = m_vols[en] * (*i.pVolume) * (*i.pPanL);
				m_dMixL += v;
				v = m_vols[en] * (*i.pVolume) * (*i.pPanR);
				m_dMixR += v;
			}
		}
	}
}

void CEMU8930::PSG_calc(SAMPLE_INT &L, SAMPLE_INT &R)
{
	if (m_bHQ)
	{
		register double f_mixL1 = 0.0;
		register double f_mixR1 = 0.0;
		register int n = 0;

		/* Simple m_nRate converter */
		do
		{
			calc();
			f_mixL1 += m_dMixL;
			f_mixR1 += m_dMixR;
			n++;
			m_dPSGTime += m_dPSGStep;
		}
		while (m_dPSGTime < m_dRealStep);

		m_dPSGTime -= m_dRealStep;
		L = SAMPLE_INT(f_mixL1 / n); // если выбирать среднее арифметическое, то шум становится мягким
		R = SAMPLE_INT(f_mixR1 / n);
		//  L = m_dMixL; // а если выбирать отдельные сэмплы, качество на слух не страдает,
		//  R = m_dMixR; // но шумовые эффекты гораздо резче выражены
	}
	else
	{
		calc();
		L = SAMPLE_INT(m_dMixL);
		R = SAMPLE_INT(m_dMixR);
	}
}

void CEMU8930::Reset()
{
	PSG_reset();
}

/*
 АУшка использует только верхний полупериод сэмпла
*/
void CEMU8930::GetSample(SAMPLE_INT &sampleL, SAMPLE_INT &sampleR)
{
	if (m_bEnableSound)
	{
		PSG_calc(sampleL, sampleR);
	}
	else
	{
		sampleL = 0.0;
		sampleR = 0.0;
	}

	// для чистоты огибающей это лучше не включать.
//  sampleL = DCOffset(sampleL, m_dAvgL, m_pdBufferL, m_nBufferPosL);
//  sampleR = DCOffset(sampleR, m_dAvgR, m_pdBufferR, m_nBufferPosR);
	// фильтр
	sampleL = FIRFilter(sampleL, m_LeftBuf, m_nLeftBufPos);
	sampleR = FIRFilter(sampleR, m_RightBuf, m_nRightBufPos);
}

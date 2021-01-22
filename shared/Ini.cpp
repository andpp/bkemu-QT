#include "pch.h"
#include "Ini.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CIni::CIni()
	: SetOfSections(nullptr)
	, m_strFileName(_T(""))
	, m_iniStatus(IniStatus::NOTREADED)
	, m_iniError(IniError::OK_NOERROR)
	, m_LineNum(0)
	, m_Encoding(0)
{
}

CIni::~CIni()
{
	Clear();
}

void CIni::Clear()
{
	if (SetOfSections)
	{
		if ((m_iniError != IniError::WRITE_ERROR) && (m_iniError != IniError::READ_ERROR))
		{
			FlushIni();
		}

		INI_SECTION_STRUCT *pISS = SetOfSections;
		INI_SECTION_STRUCT *pISS_prev = nullptr;
		SetOfSections = nullptr;
		m_iniStatus = IniStatus::NOTREADED;

		while (pISS)
		{
			INI_KEY_STRUCT *pIKS = pISS->Keys;
			INI_KEY_STRUCT *pIKS_prev = nullptr;
			pISS->Keys = nullptr;

			while (pIKS)
			{
				pIKS_prev = pIKS;
				pIKS = pIKS->next;

				if (pIKS_prev)
				{
					delete (pIKS_prev);
				}
			}

			pISS_prev = pISS;
			pISS = pISS->next;

			if (pISS_prev)
			{
				delete (pISS_prev);
			}
		}
	}
}
// получение хеша строки для быстрого поиска, чтобы сравнивать числа, а не строки
DWORD CIni::GetHash(const CString &str)
{
	DWORD h = 0; // расчёт функцией FAQ6
	DWORD hval = 0x811c9dc5; // расчёт функцией FNV
	constexpr auto FNV_32_PRIME = 0x01000193;

	if (!str.IsEmpty())
	{
        CString strlc = str.toLower(); // сделаем строки регистронезависимыми.
//		strlc.MakeLower(); // а то какая-то вообще ерунда получается.
		int nLen = strlc.GetLength();

		for (int i = 0; i < nLen; ++i)
		{
			h += strlc[i];
			h += (h << 10);
			h ^= (h >> 6);
			hval ^= DWORD(strlc[i]);
			hval *= FNV_32_PRIME;
		}

		h += (h << 3);
		h ^= (h >> 11);
		h += (h << 15);
	}

	return h ^ ((hval << 13) | (hval >> 19));
}
/*
выход: true - добавили новый элемент
false - не добавили новый элемент, по разным причинам
*/
bool CIni::Add(const CString &strSection, const CString &strKey, const CString &strVal, const CString &strComm, bool bChangeComment)
{
	if (strSection.IsEmpty() || strKey.IsEmpty())
	{
		return false;
	}

	DWORD hashSection = GetHash(strSection);
	// начинаем поиск
	INI_SECTION_STRUCT *pISS = SetOfSections;
	INI_SECTION_STRUCT *pISS_prev = nullptr;

	while (pISS)
	{
		if (pISS->Hash == hashSection)
		{
#ifdef DEBUG

			if (pISS->Name != strSection)
			{
				TRACE(_T("HASH_COLLISION:: %#08X : \"%s\" and \"%s\" \n"), hashSection, pISS->Name, strSection);
				ASSERT(false);
			}

#endif // DEBUG
			// нашли секцию
			DWORD hashKey = GetHash(strKey);
			INI_KEY_STRUCT *pIKS = pISS->Keys;
			INI_KEY_STRUCT *pIKS_prev = nullptr;

			while (pIKS)
			{
				if (pIKS->Hash == hashKey)
				{
#ifdef DEBUG

					if (pIKS->Key != strKey)
					{
						TRACE(_T("HASH_COLLISION:: %#08X : \"%s\" and \"%s\" \n"), hashKey, pIKS->Key, strKey);
						ASSERT(false);
					}

#endif // DEBUG

					// нашли ключ
					if (strVal.Compare(CString(pIKS->Value)) != 0)
					{
						// и старое значение не совпадает с новым
						pIKS->Value = strVal;
						return true;
					}
					else
					{
						return false;    // нечего добавлять
					}
				}

				pIKS_prev = pIKS;
				pIKS = pIKS->next;
			}

			// в секции ключа не нашли, надо добавить
			pIKS = new INI_KEY_STRUCT;

			if (pIKS)
			{
				pIKS->Hash = hashKey;
				pIKS->Key = strKey;
				pIKS->Value = strVal;

				if (bChangeComment)
				{
					pIKS->Comment = strComm;
				}

				pIKS->next = nullptr;

				if (pIKS_prev) // если есть к чему добавлять
				{
					pIKS_prev->next = pIKS;
				}
				else // такое может быть, если в секции вообще не было ключей
				{
					pISS->Keys = pIKS;    // это событие маловероятное, но вполне возможное,
				}

				// хотя алгоритмом вообще не предусмотрено создание пустых секций
				return true;
			}
			else
			{
				return false;
			}
		}

		pISS_prev = pISS;
		pISS = pISS->next;
	}

	// даже секции такой не нашли, надо создать
	// и новый ключ тоже создадим
	auto pIKS = new INI_KEY_STRUCT;

	if (pIKS)
	{
		pIKS->Hash = GetHash(strKey);
		pIKS->Key = strKey;
		pIKS->Value = strVal;

		if (bChangeComment)
		{
			pIKS->Comment = strComm;
		}

		pIKS->next = nullptr;
	}
	else
	{
		return false;
	}

	// теперь секцию
	pISS = new INI_SECTION_STRUCT;

	if (pISS)
	{
		pISS->Hash = hashSection;
		pISS->Name = strSection;
		pISS->next = nullptr;
		pISS->Keys = pIKS;

		if (pISS_prev) // если есть к чему добавлять
		{
			pISS_prev->next = pISS;
		}
		else // если мы в первый раз создаём первую секцию
		{
			SetOfSections = pISS;
		}

		return true;
	}

	return false;
}

bool CIni::ReadIni()
{
	m_iniStatus = IniStatus::NOTREADED;
	m_iniError = IniError::OK_NOERROR;

	if (m_strFileName.IsEmpty())
	{
		return false;    // если имя файла не задано, то и читать нечего
	}

	CStdioFile file;

	if (!file.Open(m_strFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
	{
		m_iniError = IniError::READ_ERROR;
		return false;
	}

	CString strRead;
	CString strCurrentSectionName = _T("");
	m_LineNum = 0;

	while (file.ReadString(strRead))
	{
		m_LineNum++;

		if (strRead.IsEmpty())
		{
			continue;    // если пустая строка - игнорируем
		}

        strRead.Trim(); // уберём пустоту в начале и в конце строки
		TCHAR chFch = strRead.GetAt(0);

		// пояснительные и философские комменты проигнорируем
		if (chFch == _T(';'))
		{
			continue;    // если комментарий - игнорируем
		}

		if (chFch == _T('/') && strRead.GetAt(1) == _T('/'))
		{
			continue;    // это тоже комментарий, тоже игнорируем
		}

		if (chFch == _T('[')) // мы уже убрали возможную пустоту перед [, поэтому этот символ обязательно должен быть первым, иначе это не имя секции
		{
			// начало секции
			int pos = strRead.Find(_T(']')); // ищем конец секции

			if (pos > 0) // если этот символ найден
			{
				CString strName = strRead.Mid(1, pos - 1);
				strName.Trim(); // вот. мы даже допускаем многословное имя секции,
				// strName.Replace(_T(' '), _T('_')); // хотя можем и заменить пробелы на _
				strCurrentSectionName = strName; // имя текущей секции
                TRACE(_T("%#08X : %s\n"), GetHash(strCurrentSectionName), strCurrentSectionName.toUtf8().data());
				continue;
			}
			else
			{
				m_iniError = IniError::PARSE_ERROR;
				return false;
			}
		}

		// сюда попадаем, если у нас простая строка имя=значение ;коммент
		int pos = strRead.Find(_T('='));

		// если строка не начинается с =, и = вообще есть
		if (pos > 0)
		{
			// разберёмся
			CString strKey = strRead.Left(pos++);
			strKey.Trim();
			CString strTmp = strRead.Mid(pos);
			CString strComment = _T("");
			CString strValue = _T("");
			// проверим, вдруг есть комментарий
			int commentpos = -1;
			int cpos;

			if ((cpos = strTmp.Find(_T(';'))) >= 0)
			{
				commentpos = cpos;
			}
			else if (((cpos = strTmp.Find(_T('/'))) >= 0) && (strTmp.GetAt(cpos + 1) == _T('/')))
			{
				commentpos = cpos + 1;
			}

			if (commentpos >= 0)
			{
				// если комментарий таки есть, то
				strValue = strTmp.Left(commentpos++); // вот значение
				strComment = strTmp.Mid(commentpos); // вот комментарий
			}
			else
			{
				// комментария нету, только значение
				strValue = strRead.Mid(pos);
			}

			strValue.Trim();
			strValue.Trim(_T('"')); // удалим ещё и кавычки (если есть)
			Add(strCurrentSectionName, strKey, strValue, strComment, true);
            TRACE(_T("%#08X : %s\n"), GetHash(strKey), strKey.toUtf8().data());
		}

		// иначе проигнорируем такое безобразие
	}

	file.Close();
	m_iniStatus = IniStatus::READED;
	return true;
}

bool CIni::FlushIni()
{
	if (m_iniStatus == IniStatus::NOTREADED)
	{
		return false;    // если ини не прочитан, то и записывать нечего
	}

	if (m_iniStatus == IniStatus::FLUSHED)
	{
		return true;    // если ини уже записан, то второй раз - не надо
	}

	if (!SetOfSections)
	{
		return false;    // если инифайл пустой, то и делать нечего
	}

	if (m_strFileName.IsEmpty())
	{
		return false;    // если имя файла не задано, то и записывать нечего
	}

	CStdioFile file; // CMemFile - для создания файла в памяти

	if (!file.Open(m_strFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText))
	{
		m_iniError = IniError::WRITE_ERROR;
		return false;
	}

	m_LineNum = 0;
	INI_SECTION_STRUCT *pISS = SetOfSections;
	CString strOut;

	while (pISS)
	{
		strOut = _T("[") + pISS->Name + _T("]\n");
		file.WriteString(strOut);
		m_LineNum++;
		INI_KEY_STRUCT *pIKS = pISS->Keys;

		while (pIKS)
		{
			strOut = pIKS->Key + _T(" = ") + pIKS->Value;

			// если есть комментарий
			if (!pIKS->Comment.IsEmpty())
			{
				strOut += _T("\t;") + pIKS->Comment;
			}

			strOut += _T("\n");
			file.WriteString(strOut);
			m_LineNum++;
			pIKS = pIKS->next;
		}

		file.WriteString(_T("\n")); // и для красоты добавим пустую строку
		m_LineNum++;
		pISS = pISS->next;
	}

	file.Close();
	m_iniStatus = IniStatus::FLUSHED;
	m_iniError = IniError::OK_NOERROR;
	return true;
}

// возвращает размер файла в памяти
ULONGLONG CIni::FlushIniToMemory(uint8_t *pBuff, UINT nSize)
{
	if (m_iniStatus == IniStatus::NOTREADED)
	{
		return 0;    // если ини не прочитан, то и записывать нечего
	}

	if (!SetOfSections)
	{
		return 0;    // если инифайл пустой, то и делать нечего
	}

	CMemFile file(pBuff, nSize, 1024); // создаём файл в памяти
	m_LineNum = 0;
	INI_SECTION_STRUCT *pISS = SetOfSections;
	CString strOut;

	while (pISS)
	{
		strOut = _T("[") + pISS->Name + _T("]\n");
		file.Write(strOut, strOut.GetLength() * sizeof(TCHAR));
		m_LineNum++;
		INI_KEY_STRUCT *pIKS = pISS->Keys;

		while (pIKS)
		{
			strOut = pIKS->Key + _T(" = ") + pIKS->Value;

			// если есть комментарий
			if (!pIKS->Comment.IsEmpty())
			{
				strOut += _T("\t;") + pIKS->Comment;
			}

			strOut += _T("\n");
			file.Write(strOut, strOut.GetLength() * sizeof(TCHAR));
			m_LineNum++;
			pIKS = pIKS->next;
		}

		pISS = pISS->next;
	}

	ULONGLONG len = file.GetLength();
	file.Close();
	m_iniStatus = IniStatus::FLUSHED;
	m_iniError = IniError::OK_NOERROR;
	return len;
}

bool CIni::ReadIniFromMemory(uint8_t *pBuff, UINT nSize)
{
	m_iniStatus = IniStatus::NOTREADED;
	m_iniError = IniError::OK_NOERROR;
	CMemFile file(pBuff, nSize, 0); // создаём файл в памяти
	CString strRead;
	CString strCurrentSectionName = _T("");
	m_LineNum = 0;
	bool bEof = false;
	TCHAR tch;

	while (!bEof)
	{
		strRead = _T("");

		for (;;)
		{
			file.Read(&tch, sizeof(TCHAR));

			if (file.GetPosition() >= nSize)
			{
				bEof = true;
				break;
			}

			if (tch == L'\n')
			{
				break;
			}
			else
			{
				strRead += tch;
			}
		}

		m_LineNum++;

		if (strRead.IsEmpty())
		{
			continue;    // если пустая строка - игнорируем
		}

		strRead.Trim(); // уберём пустоту в начале и в конце строки
		TCHAR chFch = strRead.GetAt(0);

		if (chFch == _T('[')) // мы уже убрали возможную пустоту перед [, поэтому этот символ обязательно должен быть первым, иначе это не имя секции
		{
			// начало секции
			int pos = strRead.Find(_T(']')); // ищем конец секции

			if (pos > 0) // если этот символ найден
			{
				CString strName = strRead.Mid(1, pos - 1);
				strName.Trim(); // вот. мы даже допускаем многословное имя секции,
				// strName.Replace(_T(' '), _T('_')); // хотя можем и заменить пробелы на _
				strCurrentSectionName = strName; // имя текущей секции
                TRACE(_T("%#08X : %s\n"), GetHash(strCurrentSectionName), strCurrentSectionName.toUtf8().data());
				continue;
			}
			else
			{
				m_iniError = IniError::PARSE_ERROR;
				return false;
			}
		}

		// сюда попадаем, если у нас простая строка имя=значение ;коммент
		int pos = strRead.Find(_T('='));

		// если строка не начинается с =, и = вообще есть
		if (pos > 0)
		{
			// разберёмся
			CString strKey = strRead.Left(pos++);
			strKey.Trim();
			CString strTmp = strRead.Mid(pos);
			CString strComment = _T("");
			CString strValue = _T("");
			// проверим, вдруг есть комментарий
			int commentpos = -1;
			int cpos;

			if ((cpos = strTmp.Find(_T(';'))) >= 0)
			{
				commentpos = cpos;
			}
			else if (((cpos = strTmp.Find(_T('/'))) >= 0) && (strTmp.GetAt(cpos + 1) == _T('/')))
			{
				commentpos = cpos + 1;
			}

			if (commentpos >= 0)
			{
				// если комментарий таки есть, то
				strValue = strTmp.Left(commentpos++); // вот значение
				strComment = strTmp.Mid(commentpos); // вот комментарий
			}
			else
			{
				// комментария нету, только значение
				strValue = strRead.Mid(pos);
			}

			strValue.Trim();
			strValue.Trim(_T('"')); // удалим ещё и кавычки (если есть)
			Add(strCurrentSectionName, strKey, strValue, strComment, true);
            TRACE(_T("%#08X : %s\n"), GetHash(strKey), strKey.toUtf8().data());
		}

		// иначе проигнорируем такое безобразие
	}

	file.Close();
	m_iniStatus = IniStatus::READED;
	return true;
}

/* Внутренняя функция получения значения
Вход:   strSection - строковое значение имени секции
        strKey - строковое значение имени ключа
        strValue - переменная, куда выдаётся результат
Выход:  true - значение нашлось
        false - не нашлось
*/
bool CIni::_intGetValueString(const CString &strSection, const CString &strKey, CString &strValue)
{
	if (m_iniStatus == IniStatus::NOTREADED)
	{
		ReadIni();
	}

	DWORD hashSection = GetHash(strSection);
	INI_SECTION_STRUCT *pISS = SetOfSections;

	while (pISS)
	{
		if (pISS->Hash == hashSection)
		{
#ifdef DEBUG

			if (pISS->Name != strSection)
			{
                TRACE(_T("HASH_COLLISION:: %#08X : \"%s\" and \"%s\" \n"), hashSection, pISS->Name.toUtf8().data(), strSection.toUtf8().data());
				ASSERT(false);
			}

#endif // DEBUG
			// нашли секцию
			DWORD hashKey = GetHash(strKey);
			INI_KEY_STRUCT *pIKS = pISS->Keys;

			while (pIKS)
			{
				if (pIKS->Hash == hashKey)
				{
#ifdef DEBUG

					if (pIKS->Key != strKey)
					{
                        TRACE(_T("HASH_COLLISION:: %#08X : \"%s\" and \"%s\" \n"), hashKey, pIKS->Key.toUtf8().data(), strKey.toUtf8().data());
						ASSERT(false);
					}

#endif // DEBUG
					// нашли ключ
					strValue = pIKS->Value;
					return true;
				}

				pIKS = pIKS->next;
			}
		}

		pISS = pISS->next;
	}

	return false;
}

bool CIni::DeleteKey(const CString &strSection, const CString &strKey)
{
	if (m_iniStatus == IniStatus::NOTREADED)
	{
		ReadIni();
	}

	DWORD hashSection = GetHash(strSection);
	INI_SECTION_STRUCT *pISS = SetOfSections;
	INI_SECTION_STRUCT *pISS_prev = nullptr;

	while (pISS)
	{
		if (pISS->Hash == hashSection)
		{
#ifdef DEBUG

			if (pISS->Name != strSection)
			{
                TRACE(_T("HASH_COLLISION:: %#08X : \"%s\" and \"%s\" \n"), hashSection, pISS->Name.toUtf8().data(), strSection.toUtf8().data());
				ASSERT(false);
			}

#endif // DEBUG
			// нашли секцию
			DWORD hashKey = GetHash(strKey);
			INI_KEY_STRUCT *pIKS = pISS->Keys;
			INI_KEY_STRUCT *pIKS_prev = nullptr;

			while (pIKS)
			{
				if (pIKS->Hash == hashKey)
				{
#ifdef DEBUG

					if (pIKS->Key != strKey)
					{
                        TRACE(_T("HASH_COLLISION:: %#08X : \"%s\" and \"%s\" \n"), hashKey, pIKS->Key.toUtf8().data(), strKey.toUtf8().data());
						ASSERT(false);
					}

#endif // DEBUG
					// нашли ключ, нужно его удалить
					INI_KEY_STRUCT *pToDeleteIKS = pIKS;

					// тут возможны 3 варианта
					// 1. ключ первый в списке
					// 2. ключ внутри списка
					// 3. ключ последний в списке
					if (pIKS->next == nullptr)
					{
						// значит ключ последний в списке
						if (pIKS_prev == nullptr) // если ключ вообще единственный в списке
						{
							pISS->Keys = nullptr;
							delete pToDeleteIKS;
							// тут надо бы и саму секцию удалить, зачем нам пустые секции
							INI_SECTION_STRUCT *pToDeleteISS = pISS;

							if (pISS->next == nullptr)
							{
								// значит секция последняя в списке
								if (pISS_prev == nullptr) // если секция вообще единственная в списке
								{
									SetOfSections = nullptr;
									delete pToDeleteISS;
								}
								else
								{
									pISS_prev->next = nullptr;
									delete pToDeleteISS;
								}
							}
							else if (pISS_prev == nullptr)
							{
								// значит секция первая в списке
								SetOfSections = pISS->next;
								delete pToDeleteISS;
							}
							else
							{
								// секция внутри списка и её удаление - самая простая вещь
								pISS_prev->next = pISS->next;
								delete pToDeleteISS;
							}
						}
						else
						{
							// удаляем последний ключ
							pIKS_prev->next = nullptr;
							delete pToDeleteIKS;
						}
					}
					else if (pIKS_prev == nullptr)
					{
						// значит ключ первый в списке
						pISS->Keys = pIKS->next;
						delete pToDeleteIKS;
					}
					else
					{
						// ключ внутри списка и его удаление - самая простая вещь
						pIKS_prev->next = pIKS->next;
						delete pToDeleteIKS;
					}

					return true;
				}

				pIKS_prev = pIKS;
				pIKS = pIKS->next;
			}
		}

		pISS_prev = pISS;
		pISS = pISS->next;
	}

	return false;
}

CString CIni::GetValueString(int nSection, const CString &strKey, const CString &strDefault)
{
	CString strSection, strValue;

	if (strSection.LoadString(nSection))
	{
		if (_intGetValueString(strSection, strKey, strValue))
		{
			return strValue;
		}

		// не нашли. тогда надо его добавить
		SetValueString(nSection, strKey, strDefault);
	}

	return strDefault;
}

CString CIni::GetValueString(int nSection, int nKey, const CString &strDefault)
{
	CString strSection, strKey, strValue;

	if (strSection.LoadString(nSection) && strKey.LoadString(nKey))
	{
		if (_intGetValueString(strSection, strKey, strValue))
		{
			return strValue;
		}

		// не нашли. тогда надо его добавить
		SetValueString(nSection, nKey, strDefault);
	}

	return strDefault;
}

int CIni::GetValueInt(int nSection, int nKey, const int nDefault)
{
	int nReturn = nDefault;
	CString strReturn = GetValueString(nSection, nKey, CString(_T("")));

	if (!strReturn.IsEmpty())
	{
		nReturn = _ttoi(strReturn);
	}

	return nReturn;
}

double CIni::GetValueFloat(int nSection, int nKey, const double fDefault)
{
	double fReturn = fDefault;
	CString strReturn = GetValueString(nSection, nKey, CString(_T("")));

	if (!strReturn.IsEmpty())
	{
		fReturn = _ttof(strReturn);
	}

	return fReturn;
}

bool CIni::GetValueBool(int nSection, int nKey, const bool bDefault)
{
	bool bReturn = bDefault;
	CString strReturn = GetValueString(nSection, nKey, CString(_T("")));

	if (!strReturn.IsEmpty())
	{
		if ((!strReturn.CompareNoCase(_T("yes"))) || (!strReturn.Compare(_T("1"))))
		{
			bReturn = true;
		}
		else
		{
			bReturn = false;
		}
	}

	return bReturn;
}

bool CIni::SetValueString(int nSection, const CString &strKey, const CString &strVal)
{
	CString strSection;

	if (strSection.LoadString(nSection))
	{
		if (Add(strSection, strKey, strVal, _T(""), false))
		{
			m_iniStatus = IniStatus::READED;
			return true;
		}
	}

	return false;
}

bool CIni::SetValueString(int nSection, int nKey, const CString &strVal)
{
	CString strSection, strKey;

	if (strSection.LoadString(nSection) && strKey.LoadString(nKey))
	{
		if (Add(strSection, strKey, strVal, _T(""), false))
		{
			m_iniStatus = IniStatus::READED;
			return true;
		}
	}

	return false;
}

bool CIni::SetValueInt(int nSection, int nKey, const int iVal)
{
	CString str;
	str.Format(_T("%d"), iVal);
	return SetValueString(nSection, nKey, str);
}

bool CIni::SetValueFloat(int nSection, int nKey, const double fVal)
{
	CString str;
	str.Format(_T("%f"), fVal);
	return SetValueString(nSection, nKey, str);
}

bool CIni::SetValueBool(int nSection, int nKey, const bool bVal)
{
	CString str = (bVal) ? _T("Yes") : _T("No");
	return SetValueString(nSection, nKey, str);
}

//LPSTR CIni::ConvertUnicodetoStr(UINT type, const CString &ustr)
//{
//	// сперва узнаем, какого размера будет новая строка
//	int t_len = WideCharToMultiByte(type, 0, ustr, -1, nullptr, 0, nullptr, nullptr);
//	int sizeOfString = t_len + 1; // и +1 для завершающего нуля
//	auto lpszAscii = new CHAR[sizeOfString]; // а потом, необходимо самим делать delete [] использованных строк

//	if (lpszAscii)
//	{
//		int t_len = WideCharToMultiByte(type, 0, ustr, -1, lpszAscii, sizeOfString, nullptr, nullptr);
//	}

//	return lpszAscii;
//}

//CString CIni::ConvertStrtoUnicode(UINT type, LPCSTR astr)
//{
//	// сперва узнаем, какого размера будет новая строка
//	int t_len = MultiByteToWideChar(type, 0, astr, -1, nullptr, 0);
//	int sizeOfString = t_len + 1; // и +1 для завершающего нуля
//	// вот тут будет строка
//	CString sret;
//	// конвертируем
//	t_len = MultiByteToWideChar(type, 0, astr, -1, sret.GetBufferSetLength(sizeOfString), sizeOfString);
//	sret.ReleaseBuffer();
//	return sret;
//}

// расширенный функционал. С кастомизацией
CString CIni::GetValueStringEx(const CString &strCustomName, int nSection, int nKey, const CString &strDefault)
{
	CString strSection, strKey, strValue;
	strSection.LoadString(nSection);
	strKey.LoadString(nKey);
	CString strCustomSection = strSection + _T(".") + strCustomName;

	// сперва ищем значение в кастомной секции
	if (_intGetValueString(strCustomSection, strKey, strValue))
	{
		// если нашли
		return strValue; // вернём что нашли
	}

	// если не нашли, поищем в базовой секции
	if (_intGetValueString(strSection, strKey, strValue))
	{
		// если нашли
		return strValue; // вернём что нашли
	}

	// если и там не нашли, тогда надо его добавить
	if (Add(strSection, strKey, strDefault, _T(""), false))
	{
		m_iniStatus = IniStatus::READED;
	}

	return strDefault;
}

int CIni::GetValueIntEx(const CString &strCustomName, int nSection, int nKey, const int nDefault)
{
	int nReturn = nDefault;
	CString strDef;
	strDef.Format(_T("%d"), nDefault);
	CString strReturn = GetValueStringEx(strCustomName, nSection, nKey, strDef);

	if (!strReturn.IsEmpty())
	{
		nReturn = _ttoi(strReturn);
	}

	return nReturn;
}

double CIni::GetValueFloatEx(const CString &strCustomName, int nSection, int nKey, const double fDefault)
{
	double fReturn = fDefault;
	CString strDef;
	strDef.Format(_T("%f"), fDefault);
	CString strReturn = GetValueStringEx(strCustomName, nSection, nKey, strDef);

	if (!strReturn.IsEmpty())
	{
		fReturn = _ttof(strReturn);
	}

	return fReturn;
}

bool CIni::GetValueBoolEx(const CString &strCustomName, int nSection, int nKey, const bool bDefault)
{
	bool bReturn = bDefault;
	CString strReturn = GetValueStringEx(strCustomName, nSection, nKey, (bDefault ? _T("Yes") : _T("No")));

	if (!strReturn.IsEmpty())
	{
		if ((!strReturn.CompareNoCase(_T("yes"))) || (!strReturn.Compare(_T("1"))))
		{
			bReturn = true;
		}
		else
		{
			bReturn = false;
		}
	}

	return bReturn;
}

bool CIni::SetValueStringEx(const CString &strCustomName, int nSection, int nKey, const CString &strVal, bool bForce)
{
	// флаг bForce - принудительно писать в кастомную секцию
	CString strSection, strKey;

	if (strSection.LoadString(nSection) && strKey.LoadString(nKey))
	{
		CString strCustomSection = strSection + _T(".") + strCustomName;

		if (!bForce)    // если сохранять в кастомную секцию только по необходимости
		{
			CString strOldValue;
			// читаем значение из стандартной секции
			bool bReaded = _intGetValueString(strSection, strKey, strOldValue);

			// если прочиталось
			if (bReaded)
			{
				// и оно такое, же какое мы хотим сохранить
				if (strOldValue == strVal)
				{
					// то его вообще необязательно сохранять, а из кастомной секции старое
					// значение, какое бы оно ни было, нужно удалить
					// на всякий случай прочитаем значение кастомной секции
					bReaded = _intGetValueString(strCustomSection, strKey, strOldValue);

					if (bReaded)
					{
						// если прочиталось, то значение из кастомной секции можно удалить
						return DeleteKey(strCustomSection, strKey);
					}
					else // если не прочиталось, то там и не было ничего
					{
						return true; // значит и не надо, в стандартной секции же есть.
					}

					// тут вообще можно удалять не читая, если такого параметра нет, DeleteKey
					// возвратит false, поэтому нужно делать
					// DeleteKey(strCustomSection, strKey);
					// return true;
				}

				// если не такое, то новое значение сохраним в кастомной секции
			}
			else // если не прочиталось, то надо сохранять.
			{
				if (Add(strSection, strKey, strVal, _T(""), false))
				{
					m_iniStatus = IniStatus::READED;
					return true;
				}

				return false;
			}
		}

		// сюда попадаем, если нужно принудительно сохранять значение в кастомную секцию,
		// или оно не совпадает со значением в основной секции
		if (Add(strCustomSection, strKey, strVal, _T(""), false))
		{
			m_iniStatus = IniStatus::READED;
			return true;
		}
	}

	return false;
}

bool CIni::SetValueIntEx(const CString &strCustomName, int nSection, int nKey, const int iVal, bool bForce)
{
	CString str;
	str.Format(_T("%d"), iVal);
	return SetValueStringEx(strCustomName, nSection, nKey, str, bForce);
}

bool CIni::SetValueFloatEx(const CString &strCustomName, int nSection, int nKey, const double fVal, bool bForce)
{
	CString str;
	str.Format(_T("%f"), fVal);
	return SetValueStringEx(strCustomName, nSection, nKey, str, bForce);
}

bool CIni::SetValueBoolEx(const CString &strCustomName, int nSection, int nKey, const bool bVal, bool bForce)
{
	CString str = bVal ? _T("Yes") : _T("No");
	return SetValueStringEx(strCustomName, nSection, nKey, str, bForce);
}

bool CIni::CopySection(CIni *pSrcIni, const CString &strSectionName)
{
	DWORD hashSection = GetHash(strSectionName);
	INI_SECTION_STRUCT *pISS = pSrcIni->SetOfSections;

	while (pISS)
	{
		if (pISS->Hash == hashSection)
		{
#ifdef DEBUG

			if (pISS->Name != strSectionName)
			{
                TRACE(_T("HASH_COLLISION:: %#08X : \"%s\" and \"%s\" \n"), hashSection, pISS->Name.toUtf8().data(), strSectionName.toUtf8().data());
				ASSERT(false);
			}

#endif // DEBUG
			// нашли секцию, теперь добавим все ключи
			INI_KEY_STRUCT *pIKS = pISS->Keys;

			while (pIKS)
			{
				Add(strSectionName, pIKS->Key, pIKS->Value, pIKS->Comment);
				pIKS = pIKS->next;
			}

			m_iniStatus = IniStatus::READED;
			return true;
		}

		pISS = pISS->next;
	}

	return false;
}

CIni::INI_SECTION_STRUCT *CIni::GetSectionPtr(int nSection)
{
	CString strSection;

	if (strSection.LoadString(nSection))
	{
		DWORD hashSection = GetHash(strSection);
		INI_SECTION_STRUCT *pISS = SetOfSections;

		while (pISS)
		{
			if (pISS->Hash == hashSection)
			{
				// нашли секцию
				return pISS;
			}

			pISS = pISS->next;
		}
	}

	return nullptr;
}

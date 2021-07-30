#include "SymbolTable.h"

#include <QTextStream>

#define MAX_SYM_LEN 64

constexpr static char radtbl[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ$. 0123456789";

CSymTable::CSymTable()
{
    RemoveAllSymbols();
}

void CSymTable::AddSymbol(const u_int16_t addr, const CString& name)
{
    m_SymbolsMap[addr] = name;
}

void CSymTable::AddSymbolIfNotExist(const u_int16_t addr, const CString& name)
{
    if(!m_SymbolsMap.contains(addr))
        m_SymbolsMap[addr] = name;
}


CString CSymTable::GetSymbolForAddr(const uint16_t addr)
{
    if(m_SymbolsMap.contains(addr))
        return m_SymbolsMap[addr];

    return "";
}

uint16_t CSymTable::GetAddrForSymbol(const CString& name)
{

    SymTable_t::const_iterator i = m_SymbolsMap.cbegin();

    for(; i != m_SymbolsMap.cend(); i++) {
        if (i.value() == name) {
            return i.key();
        }

    }
    return SYMBOL_NOT_EXIST;
}

void CSymTable::RemoveSymbol(const u_int16_t addr)
{
    m_SymbolsMap.remove(addr);
}

void CSymTable::RemoveSymbol(const CString& name)
{
    QHashIterator<int16_t, CString> i(m_SymbolsMap);
    for(; i.hasNext(); i.next()) {
        if (i.value() == name) {
            m_SymbolsMap.remove(i.key());
            break;
        }
    }
}

int   CSymTable::LoadSymbols(const CString &fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

    RemoveAllSymbols();

    QTextStream in(&file);
    while (!in.atEnd()) {
        CString line = in.readLine();
        QStringList list;
        list = line.split(QRegularExpression("\\W+"), QString::SkipEmptyParts);
        if (list.size() < 2) continue;
        for(int i=0; i<list.size()-1; i+=2) {
            AddSymbol(list[i+1].trimmed().toInt(nullptr, 8), list[i].trimmed());
        }
    }
    file.close();
    return true;
}

int   CSymTable::LoadSymbolsSTB(const CString &fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly))
            return false;

    RemoveAllSymbols();

    size_t objfilesize = file.size();
    uint8_t *objfiledata = (uint8_t *)malloc(objfilesize + 10);
    int res = file.read((char *)objfiledata, objfilesize);
    file.close();

    if(res != (int)objfilesize) {
        return false;
    }


    size_t offset = 0;
    while (offset < objfilesize)
    {
        uint8_t* data = (uint8_t*)(objfiledata) + offset;
        uint16_t* dataw = (uint16_t*)(data);
        if (*dataw != 1)
        {
            // Possibly that is filler at the end of the block
            size_t offsetnext = (offset + 511) & ~511;
            size_t shift = offsetnext - offset;
            data += shift; offset += shift;
            if (offset >= objfilesize)
                break;  // End of file
            dataw = (uint16_t*)(data);
            if (*dataw != 1)
                printf("Unexpected word %06ho at %06lu in %s\n", *dataw, offset, fname.toLocal8Bit().data());
        }

        uint16_t blocksize = ((uint16_t*)data)[1];
        uint16_t blocktype = ((uint16_t*)data)[2];

        switch (blocktype) {
            case  1:  { // 1 - START GSD RECORD
                uint16_t blocksize = ((uint16_t*)data)[1]-6;
                for(int i=0; i<blocksize; )
                {
                    const uint8_t* itemw = (const uint8_t*)(data + 6 + i);

                    i += process_gsd_item(itemw);
                }
                break;
            }
            case 0:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                break;
            default: {
                printf("ERROR: Illegal record type at %06lu in %s\n", offset, GetFileTitle(fname).toLocal8Bit().data());
            }
        }

        offset += blocksize;
        offset += 1;  // Skip checksum
    }


    free(objfiledata);
    return true;
}

int CSymTable::SaveSymbolsSTB(const CString &fname)
{
    GSDWriter gsd(fname);

    if(!gsd.gsd_init())
        return 0;

    SymTable_t::const_iterator i = m_SymbolsMap.cbegin();

    for(; i != m_SymbolsMap.cend(); i++) {
        gsd.gsd_write(i.value(), 010, 4, i.key());
    }

    gsd.gsd_flush();
    gsd.gsd_end();

    return 1;
}

GSDWriter::GSDWriter(const CString &fname)
  : m_nOffset(0)
{
    m_fp = fopen(fname.toLocal8Bit().data(), "wb");
}

GSDWriter::~GSDWriter()
{
    fclose(m_fp);
}

int GSDWriter::gsd_init()
{
    if(m_fp == NULL)
        return 0;
    m_Buf[0] = 1; //OBJ_GSD;             /* GSD records start with 1,0 */
    m_Buf[1] = 0;
    m_nOffset = 2;                   /* Offset for further additions */
    return 1;
}

/* gsd_flush - write buffered GSD records */

int GSDWriter::gsd_flush()
{
    if (m_nOffset > 2) {
        if (!writerec(m_Buf, m_nOffset))
            return 0;
        gsd_init();
    }
    return 1;
}

/* gsd_write - buffers a GSD record */

/* 0xFFFF name */
/* 1 byte flags */
/* 1 byte type */
/* 2 bytes value */

int GSDWriter::gsd_write(const CString &symName, int flags, int type, int value)
{
    uint8_t  *cp;
    char  *name = symName.toLocal8Bit().data();

    int nlen =  1 + 2 + symName.length();

    if (m_nOffset > sizeof(m_Buf) - nlen - 4) {
        if (!gsd_flush())
            return 0;
    }

    cp = m_Buf + m_nOffset;

    *cp++ = 0xff; *cp++ = 0xff;
    while(*name) {
        *cp++ = *name++;
    }
    *cp++=0;

    *cp++ = flags;
    *cp++ = type;

    *cp++ = value & 0xff;
    *cp++ = (value >> 8) & 0xff;

    m_nOffset  = cp - m_Buf;

    return 1;
}

int GSDWriter::gsd_end()
{
    m_Buf[0] = 2; //OBJ_ENDGSD;
    m_Buf[1] = 0;
    return writerec(m_Buf, 2);
}


int GSDWriter::writerec(uint8_t *data, int len)
{
    int             chksum;     /* Checksum is negative sum of all
                                   bytes including header and length */
    int             i;
    unsigned        hdrlen = len + 4;

    if (m_fp == NULL)
        return 1;                      /* Silently ignore this attempt to write. */

    chksum = 0;
    if (fputc(1, m_fp) == EOF)   /* All recs begin with 1,0 */
        return 0;
    chksum -= 1;
    if (fputc(0, m_fp) == EOF)
        return 0;
    chksum -= 0;

    i = hdrlen & 0xff;                 /* length, lsb */
    chksum -= i;
    if (fputc(i, m_fp) == EOF)
        return 0;

    i = (hdrlen >> 8) & 0xff;          /* length, msb */
    chksum -= i;
    if (fputc(i, m_fp) == EOF)
        return 0;

    i = fwrite(data, 1, len, m_fp);
    if (i < len)
        return 0;

    while (len > 0) {                  /* All the data bytes */
        chksum -= *data++ & 0xff;
        len--;
    }

    chksum &= 0xff;

    fputc(chksum, m_fp);                 /* Followed by the checksum byte */

    return 1;                          /* Worked okay. */
}


#define WORD(cp) ((*(cp) & 0xff) + ((*((cp)+1) & 0xff) << 8))

// Decodes 3 chars of RAD50 into the given buffer
void CSymTable::unrad50(uint16_t word, char *cp)
{
    if (word < 0175000)              /* Is it legal RAD50? */
    {
        cp[0] = radtbl[word / 03100];
        cp[1] = radtbl[(word / 050) % 050];
        cp[2] = radtbl[word % 050];
    }
    else
        cp[0] = cp[1] = cp[2] = ' ';
}

// Decodes 6 chars of RAD50 into the temp buffer and returns buffer address
const char* CSymTable::unrad50(uint16_t loword, uint16_t hiword)
{
    memset(unrad50buffer, 0, sizeof(unrad50buffer));
    unrad50(loword, unrad50buffer);
    unrad50(hiword, unrad50buffer + 3);
    return unrad50buffer;
}

// Decodes 6 chars of RAD50 into the temp buffer and returns buffer address
const char* CSymTable::unrad50(uint32_t data)
{
    memset(unrad50buffer, 0, sizeof(unrad50buffer));
    unrad50(LOWORD(data), unrad50buffer);
    unrad50(HIWORD(data), unrad50buffer + 3);
    return unrad50buffer;
}

int CSymTable::rad50name(char *cp, char *name)
{
    int i = 0;
    if(WORD(cp) != 0xFFFF) {
        // Decode RAD50 symbol
        unrad50(WORD(cp), name);
        unrad50(WORD(cp + 2), name + 3);
        name[6] = 0;
//        trim(name);
        i = 4;
    } else {
        // Decode plain text symbol
        cp+=2;
        while(*cp) {
            name[i] = *cp++;
            i++;
        }
        name[i++] = 0;
        i+=2;
    }
    return i;
}

int CSymTable::process_gsd_item(const uint8_t* itemw)
{
    int i;
    char name[MAX_SYM_LEN];

    i=rad50name((char *)itemw, name);
    uint16_t itemflags = itemw[i++];
    uint16_t itemtype = itemw[i++];
    uint16_t value = (uint16_t)(itemw[i] + (itemw[i+1] << 8));
    i+=2;

    switch (itemtype)
    {
    case 0: // 0 - MODULE NAME FROM .TITLE
        break;
    case 2: // 2 - ISD ENTRY
        printf("Add local symbol %s addr %06ho\n", name, value);
        AddSymbol(value, name);
        break;
    case 3: // 3 - TRANSFER ADDRESS
        break;
    case 4: // 4 - GLOBAL SYMBOL
        printf("Add Global symbol %s addr %06ho\n", name, value);
        AddSymbol(value, name);
        break;
    case 1: // 1 - CSECT NAME
        break;
    case 5: // 5 - PSECT NAME
        break;
    case 6: // 6 - IDENT DEFINITION
        break;
    case 7: // 7 - VIRTUAL SECTION
        break;
    default:
        printf("ERROR: Incorect GSD item type. Possibly the symbol file is damaged\n");
    }
    return i;
}


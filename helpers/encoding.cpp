
#include "encoding.h"


/* utf8_decode.c */

/* 2016-04-05 */

/*
Copyright (c) 2005 JSON.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "encoding.h"

/*
    Very Strict UTF-8 Decoder

    UTF-8 is a multibyte character encoding of Unicode. A character can be
    represented by 1-4 bytes. The bit pattern of the first byte indicates the
    number of continuation bytes.

    Most UTF-8 decoders tend to be lenient, attempting to recover as much
    information as possible, even from badly encoded input. This UTF-8
    decoder is not lenient. It will reject input which does not include
    proper continuation bytes. It will reject aliases (or suboptimal
    codings). It will reject surrogates. (Surrogate encoding should only be
    used with UTF-16.)

    Code     Contination Minimum Maximum
    0xxxxxxx           0       0     127
    10xxxxxx       error
    110xxxxx           1     128    2047
    1110xxxx           2    2048   65535 excluding 55296 - 57343
    11110xxx           3   65536 1114111
    11111xxx       error
*/





/*
    Get the next byte. It returns UTF8_END if there are no more bytes.
*/
int utf8_stream::get()
{
    int c;
    if (m_index >= m_length) {
        return UTF8_END;
    }
    c = m_input[m_index] & 0xFF;
    m_index += 1;
    return c;
}


/*
    Get the 6-bit payload of the next continuation byte.
    Return UTF8_ERROR if it is not a contination byte.
*/
int utf8_stream::cont()
{
    int c = get();
    return ((c & 0xC0) == 0x80)
        ? (c & 0x3F)
        : UTF8_ERROR;
}


/*
    Initialize the UTF-8 decoder. The decoder is not reentrant,
*/
utf8_stream::utf8_stream(const char *p, int length)
{
    m_index = 0;
    m_input = (char *)p;
    m_length = length;
    m_char = 0;
    m_byte = 0;
}


/*
    Get the current byte offset. This is generally used in error reporting.
*/
int utf8_stream::decode_at_byte()
{
    return m_byte;
}


/*
    Get the current character offset. This is generally used in error reporting.
    The character offset matches the byte offset if the text is strictly ASCII.
*/
int utf8_stream::decode_at_character()
{
    return (m_char > 0)
        ? m_char - 1
        : 0;
}


/*
    Extract the next character.
    Returns: the character (between 0 and 1114111)
         or  UTF8_END   (the end)
         or  UTF8_ERROR (error)
*/
int utf8_stream::decode_next()
{
    int c;  /* the first byte of the character */
    int c1; /* the first continuation character */
    int c2; /* the second continuation character */
    int c3; /* the third continuation character */
    int r;  /* the result */

    if (m_index >= m_length) {
        return m_index == m_length ? UTF8_END : UTF8_ERROR;
    }
    m_byte = m_index;
    m_char += 1;
    c = get();
/*
    Zero continuation (0 to 127)
*/
    if ((c & 0x80) == 0) {
        return c;
    }
/*
    One continuation (128 to 2047)
*/
    if ((c & 0xE0) == 0xC0) {
        c1 = cont();
        if (c1 >= 0) {
            r = ((c & 0x1F) << 6) | c1;
            if (r >= 128) {
                return r;
            }
        }

/*
    Two continuations (2048 to 55295 and 57344 to 65535)
*/
    } else if ((c & 0xF0) == 0xE0) {
        c1 = cont();
        c2 = cont();
        if ((c1 | c2) >= 0) {
            r = ((c & 0x0F) << 12) | (c1 << 6) | c2;
            if (r >= 2048 && (r < 55296 || r > 57343)) {
                return r;
            }
        }

/*
    Three continuations (65536 to 1114111)
*/
    } else if ((c & 0xF8) == 0xF0) {
        c1 = cont();
        c2 = cont();
        c3 = cont();
        if ((c1 | c2 | c3) >= 0) {
            r = ((c & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
            if (r >= 65536 && r <= 1114111) {
                return r;
            }
        }
    }
    return UTF8_ERROR;
}

// таблица соответствия верхней половины аскии кодов с 128 по 255, включая псевдографику
static const wchar_t koi8tbl[128] =   // {200..237} этих символов на бк нету.
{
    L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
    L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
    L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
    L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
    // {240..277}
//	0xb6,   0x2534, 0x2665, 0x2510, 0x2561, 0x251c, 0x2514, 0x2550,
//	0x2564, 0x2660, 0x250c, 0x252c, 0x2568, 0x2193, 0x253c, 0x2551,
//	0x2524, 0x2190, 0x256c, 0x2191, 0x2663, 0x2500, 0x256b, 0x2502,
//	0x2666, 0x2518, 0x256a, 0x2565, 0x2567, 0x255e, 0x2192, 0x2593,
    L'¶',   L'┴',   L'♥',   L'┐',   L'╡',   L'├',   L'└',   L'═',
    L'╤',   L'♠',   L'┌',   L'┬',   L'╨',   L'↓',   L'┼',   L'║',
    L'┤',   L'←',   L'╬',   L'↑',   L'♣',   L'─',   L'╫',   L'│',
    L'♦',   L'┘',   L'╪',   L'╥',   L'╧',   L'╞',   L'→',   L'▓',

    // {300..337}
    L'ю',   L'а',   L'б',   L'ц',   L'д',   L'е',   L'ф',   L'г',
    L'х',   L'и',   L'й',   L'к',   L'л',   L'м',   L'н',   L'о',
    L'п',   L'я',   L'р',   L'с',   L'т',   L'у',   L'ж',   L'в',
    L'ь',   L'ы',   L'з',   L'ш',   L'э',   L'щ',   L'ч',   L'ъ',
    // {340..377}
    L'Ю',   L'А',   L'Б',   L'Ц',   L'Д',   L'Е',   L'Ф',   L'Г',
    L'Х',   L'И',   L'Й',   L'К',   L'Л',   L'М',   L'Н',   L'О',
    L'П',   L'Я',   L'Р',   L'С',   L'Т',   L'У',   L'Ж',   L'В',
    L'Ь',   L'Ы',   L'З',   L'Ш',   L'Э',   L'Щ',   L'Ч',   L'Ъ'
};


char utf82koi(int uchr)
{
    char b;

    if (uchr == L'\t')
    {
        b = 9;
    }
    else if (uchr == L'\n')
    {
        b = 10;
    }
    else if (uchr < 32) // если символ меньше пробела,
    {
        b = 32;  // то будет пробел
    }
    else if ((32 <= uchr) && (uchr < 127)) // если буквы-цифры- знаки препинания
    {
        b = char(uchr); // то буквы-цифры- знаки препинания
    }
    else if (uchr == 0x25a0) // если такое
    {
        b = 127; // то это. это единственное исключение в нижней половине аски кодов
    }
    else // если всякие другие символы
    {
        // то ищем в таблице нужный нам символ, а его номер - будет кодом кои8
        b = 32; // если такого символа нету в таблице - будет пробел

        if (uchr == L'ё')
        {
            uchr = L'е'; // меняем ё на е, т.к. на БК нету такой буквы
        }
        else if (uchr == L'Ё')
        {
            uchr = L'Е'; // меняем ё на е, т.к. на БК нету такой буквы
        }

        for (int i = 32; i < 128; ++i)
        {
            if (koi8tbl[i] == uchr)
            {
                b = i + 0200;
                break;
            }
        }
    }

    return b;
}

#ifndef ENCODING_H
#define ENCODING_H

/* utf8_decode.h */

#define UTF8_END   -1
#define UTF8_ERROR -2

class utf8_stream {
    int  m_index = 0;
    int  m_length = 0;
    int  m_char = 0;
    int  m_byte = 0;
    char* m_input;
  public:
    utf8_stream(const char *cp, int len);
    int   decode_at_byte();
    int   decode_at_character();
    int   decode_next();
    char *get_ptr() {return m_input + m_index; }
  private:
    int   get();
    int   cont();
};

char utf82koi(int sym);

#endif // ENCODING_H

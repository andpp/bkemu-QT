#pragma once
#include <qstring.h>
#include <QDir>
#include "res.h"
#include <QRegularExpression>

class CString: public QString {
  public:
    inline CString() : QString() {}
    explicit CString(const QChar *unicode, int size = -1): QString(unicode, size) {}
    CString(QChar c) : QString(c) {}
    CString(int size, QChar c) : QString(size, c) {}
    inline CString(QLatin1String latin1): QString(latin1) {}
    inline CString(const wchar_t *wc) : QString(fromWCharArray(wc)) {}
    inline CString(const char *s) : QString(s) {}
    inline CString(const CString & s) : QString(s) {}
    inline CString(const QString & s) : QString(s) {}
    inline ~CString() {}

    bool IsEmpty() const {return isEmpty(); }
    int GetLength() const { return length(); }

    short operator[](int i) const {return at(i).unicode();}
    CString& operator=(const CString&) = default;

    int Compare(const CString & s) const {return compare(s);}
    int CompareNoCase(const CString & s) const {return compare(s, Qt::CaseInsensitive);}
    int CompareNoCase(const char *c) const { return compare(c, Qt::CaseInsensitive); }
    int CompareNoCase(const wchar_t *c) const { return compare(CString(c), Qt::CaseInsensitive); }

    int CollateNoCase(const CString & s) const {return compare(s, Qt::CaseInsensitive);}
    int CollateNoCase(const char *c) const { return compare(c, Qt::CaseInsensitive); }
    int CollateNoCase(const wchar_t *c) const { return compare(CString(c), Qt::CaseInsensitive); }

    wchar_t GetAt(int i) { return this->at(i).unicode(); }
    void SetAt(int i, char c) { replace(i,1,c); }
    int Find(const wchar_t wc, int from = 0) { return indexOf(QChar(wc), from); }
    int Find(const char *c, int from = 0) { return indexOf(c, from); }
    int Find(const CString &str, int from = 0) { return indexOf(str, from); }
    int ReverseFind(const wchar_t wc, int from = -1) {return lastIndexOf(QChar(wc), from); }
    CString Mid(int start, int end = -1) const { return mid(start, end); }

    int toInt(bool *ok = nullptr, int base = 10) const {
        QString str = this->trimmed();
        str.remove('%');
        return str.toInt(ok, base);
    }

    const char * GetString() const {return toUtf8().data(); }

    bool LoadString(int i) {
        CString str(g_ResourceStrings.GetString(i));
        this->clear(); this->resize(str.size());
        this->replace(0, str.size(), str);
        return true;
    }
    CString &Trim() {
        CString str = this->trimmed();
        str = str.remove(QChar::Null);
        this->clear();
        this->replace(0,str.size(), str);
        return *this;
    }
    CString &Trim(wchar_t c) {
        if (this->GetAt(0) ==  c) this->remove(0,1);
        if (this->GetAt(this->size()-1) == c) this->chop(1);
        return *this;
    }

    CString &TrimLeft(wchar_t c) {
        if (this->GetAt(this->size()-1) == c) this->chop(1);
        return *this;
    }

    void Empty() { this->clear(); };

    CString &Insert(int i, const CString str) {
        CString tmp = this->insert(i, str);  this->clear(); this->resize(str.size()); this->replace(0, tmp.size(), tmp);
        return *this;
    }

    CString Left(int pos) { return this->left(pos); }
    CString Right(int pos) { return this->right(pos); }

    CString &Format(const char *format, ...) {
          va_list args;
          va_start (args, format);
          CString str = CString::vasprintf(format, args);
          va_end(args);
          this->clear(); this->resize(str.size()); this->replace(0, str.size(), str);
          return *this;
    }

    CString &Format(const CString& format, ...) {
          va_list args;
          va_start (args, format);
          CString str = CString::vasprintf(format.toLocal8Bit().data(), args);
          va_end(args);
          this->clear(); this->resize(str.size()); this->replace(0, str.size(), str);
          return *this;
    }

    CString &Format(const int frmt_id, ...) {
          va_list args;
          va_start (args, frmt_id);
          CString str = CString::vasprintf(MAKEINTRESOURCE(frmt_id), args);
          va_end(args);
          this->clear(); this->resize(str.size()); this->replace(0, str.size(), str);
          return *this;
    }

    CString &Format(const int frmt_id, const CString& arg) {
          CString str = CString::asprintf(MAKEINTRESOURCE(frmt_id), arg.toLocal8Bit().data());
          this->clear(); this->resize(str.size()); this->replace(0, str.size(), str);
          return *this;
    }

    CString &Format(const int frmt_id, const int str_id) {
        CString str = CString::asprintf(MAKEINTRESOURCE(frmt_id), MAKEINTRESOURCE(str_id));
          this->clear(); this->resize(str.size()); this->replace(0, str.size(), str);
          return *this;
    }

    CString MakeUpper() {return this->toUpper();}
    CString &SpanExcluding(const char *str) {
        this->remove(str);
        return *this;
    }

    CString &SpanExcluding(const QChar *str) {
        this->remove(CString(str));
        return *this;
    }

    CString &SafeName() {
        CString exp = "[" + QRegularExpression::escape("!@#%^&*(){}[];:\\|/?.,<>`~") + "]";
        //replace(QRegularExpression("\\$"), "S");
        replace(QRegularExpression(exp), "_");
        replace(QRegularExpression("^([0-9]*)$"),"\\1$");
        return *this;
    }


};

Q_DECLARE_METATYPE(CString)


inline int _ttoi(CString &a) {
    return a.toInt();
}

inline double _ttof(CString &a) {
    return a.toDouble();
}

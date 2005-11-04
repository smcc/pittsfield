/* A minimal version */

#ifndef PITTSFIELD_IOSTREAM_H_INCLUDED
#define PITTSFIELD_IOSTREAM_H_INCLUDED

#include <stdio.h>

class ios {
public:
    enum open_mode {
        in = 1,
        out = 2,
        ate = 4,
        app = 8,
        trunc = 16,
        nocreate = 32,
        noreplace = 64,
        bin = 128,
        binary = 128 };
};

class istream;
class ostream;

typedef ostream& (*omanip)(ostream&);

class istream {
public:
    istream() : failed(false) { }
    FILE *fp;
    bool failed;
    istream& operator>>(char*);
    istream& operator>>(char& c);
    istream& operator>>(int&);
    istream& operator>>(float&);
    istream& operator>>(double&);
    istream& get(char& c);
    istream& get(unsigned char& c) { return get((char&)c); }
    operator void*() const { return failed ? (void*)0 : (void*)(-1); }
    int operator!() const { return failed; }
};

class ostream {
public:
    ostream() : failed(false) { }
    FILE *fp;
    bool failed;
    ostream& operator<<(const char *s);
    ostream& operator<<(int n);
    ostream& operator<<(unsigned int n);
    ostream& operator<<(long n);
    ostream& operator<<(unsigned long n);
    ostream& operator<<(short n) {return operator<<((int)n);}
    ostream& operator<<(unsigned short n) {return operator<<((unsigned int)n);}
    ostream& operator<<(bool b) { return operator<<((int)b); }
    ostream& operator<<(double n);
    ostream& operator<<(float f);
    ostream& operator<<(long double n);
    ostream& operator<<(const void *p);
    ostream& operator<<(omanip func) { return (*func)(*this); }
    operator void*() const { return failed ? (void*)0 : (void*)(-1); }
    int operator!() const { return failed; }
};

ostream& endl(ostream& outs);

extern istream cin;
extern ostream cout, cerr, clog;

#endif

/* A minimal version */

#ifndef PITTSFIELD_IOSTREAM_H_INCLUDED
#define PITTSFIELD_IOSTREAM_H_INCLUDED

#ifndef IS_THE_LIBRARY
#include <stdio.h>
#endif

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
    enum { skipws=01 };
};

class istream;
class ostream;

typedef ostream& (*omanip)(ostream&);
typedef ssize_t streamsize;
typedef unsigned long fmtflags;

class istream {
private:
    void skip_ws() {
	for (;;) {
	    int c = getc(fp);
	    if (c == EOF) {
		return;
	    } else if (!isspace(c)) {
		ungetc(c, fp);
		return;
	    }
	}
    }
public:
    istream() : failed(false) { }
    FILE *fp;
    bool failed;
    istream& operator>>(char *cp) {
	fscanf(fp, " %s", cp);
	return *this;
    }
    istream& operator>>(char& cr) {
	char c;
	fscanf(fp, " %c", &c);
	cr = c;
	return *this;
    }
    istream& operator>>(unsigned char& cr) {
	unsigned char c;
	fscanf(fp, " %c", (char *)&c);
	cr = c;
	return *this;
    }
    istream& operator>>(int& v) {
	int x;
	fscanf(fp, " %d", &x);
	v = x;
	return *this;
    }
    istream& operator>>(float& v) {
	float f;
	fscanf(fp, " %f", &f);
	v = f;
	return *this;
    }
    istream& operator>>(double& v) {
	double f;
	fscanf(fp, " %lf", &f);
	v = f;
	return *this;
    }
    int eof() const {
	return feof(fp);
    }
    istream& get(char& c) {
	int ch = fgetc(fp);
	if (ch == EOF) {
	    /* XXX do nothing? */
	} else {
	    c = ch;
	}
	return *this;
    }
    istream& get(unsigned char& c) { return get((char&)c); }
    istream& putback(char ch) {
	ungetc(ch, fp);
	return *this;
    }
    fmtflags unsetf(fmtflags mask) { return 0; /* XXX */ } 
    int good() const { return !failed; }
    int fail() const { return failed; }
    operator void*() const { return failed ? (void*)0 : (void*)(-1); }
    int operator!() const { return failed; }
};

class ostream {
public:
    ostream() : failed(false) { }
    FILE *fp;
    bool failed;
    int good() const { return !failed; }
    int fail() const { return failed; }
    ostream& put(char c) {
	putc(c, fp);
	return *this;
    }
    fmtflags unsetf(fmtflags mask);
    ostream& write(const char *s, streamsize n) {
	fwrite(s, n, 1, fp);
	return *this;
    }
    ostream& operator<<(const char *s) {
	fputs(s, fp);
	return *this;
    }
    ostream& operator<<(int n) {
	fprintf(fp, "%d", n);
	return *this;
    }
    ostream& operator<<(unsigned int n) { 
	fprintf(fp, "%ud", n);
	return *this;
    }
    ostream& operator<<(long n) {
	fprintf(fp, "%ld", n);
	return *this;
    }
    ostream& operator<<(unsigned long n) {
	fprintf(fp, "%uld", n);
	return *this;
    }
    ostream& operator<<(short n) {return operator<<((int)n);}
    ostream& operator<<(unsigned short n) {return operator<<((unsigned int)n);}
    ostream& operator<<(bool b) { return operator<<((int)b); }
    ostream& operator<<(double n) {
	fprintf(fp, "%g", n);
	return *this;
    }
    ostream& operator<<(float f) {
	fprintf(fp, "%g", f);
	return *this;
    }
    ostream& operator<<(long double n) {
	fprintf(fp, "%lg", n);
	return *this;
    }
    ostream& operator<<(const void *p) {
	fprintf(fp, "%p", p);
	return *this;
    }
    ostream& operator<<(omanip func) { return (*func)(*this); }
    operator void*() const { return failed ? (void*)0 : (void*)(-1); }
    int operator!() const { return failed; }
};

ostream& endl(ostream& outs);

extern istream cin;
extern ostream cout, cerr, clog;

#endif

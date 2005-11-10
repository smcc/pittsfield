/* A minimal version */

#ifndef PITTSFIELD_FSTREAM_H_INCLUDED
#define PITTSFIELD_FSTREAM_H_INCLUDED

#ifndef IS_THE_LIBRARY
#include <stdio.h>
#include <iostream.h>
#endif

class ifstream : public istream {
 public:
    ifstream() {}

    ifstream(int fd) {
	fp = fdopen(fd, "r");
    }

    ifstream(const char *name) {
	fp = fopen(name, "r");
    }

    void open(const char *name, int mode=ios::in) {
        fp = fopen(name, "r");
    }

    void close(void) {
	if (fp)
	    fclose(fp);
    }
};

class ofstream : public ostream {
 public:
    ofstream() {}

    ofstream(int fd) {
	fp = fdopen(fd, "w");
    }

    ofstream(const char *name) {
	fp = fopen(name, "w");
    }

    void open(const char *name, int mode=ios::out) {
        fp = fopen(name, "w");
    }

    void close(void) {
	if (fp)
	    fclose(fp);
    }
};

#endif

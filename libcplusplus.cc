#define IS_THE_LIBRARY
#include "libc.h"
#include <iostream.h>
#include <fstream.h>

istream cin = ifstream(0);

ostream cout = ofstream(1);
ostream cerr = ofstream(2);
ostream clog = ofstream(2);

ostream& endl(ostream& outs) {
    outs << "\n";
    return outs;
}

void *operator new(unsigned int size) {
    return malloc(size);
}

void *operator new[](unsigned int size) {
    return malloc(size);
}

void operator delete(void *vp) {
    free(vp);
}

void operator delete[](void *vp) {
    free(vp);
}

extern "C" {
    extern void (*__CTOR_LIST__)(void);

    void global_constructors(void) {
	void (**ctor)(void) = &__CTOR_LIST__;
	ctor++; /* Skip first "0" header */
	while (*(int *)ctor != -1) {
	    (**ctor)();
	    ctor++;
	}
    }

    void __cxa_pure_virtual(void) { abort(); }
}

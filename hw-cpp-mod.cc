#include "libc.h"

#define IS_THE_LIBRARY
#include "fake-libc-inc/iostream.h"

int main(int argc, char **argv) {
    cout << "Hello, world!" << endl;
}

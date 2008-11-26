#include <stdio.h>
#include <string.h>

void bad(char *str) {
    char buf[10], *p;
    strcpy(buf, str);
    for (p = buf; *p; p++) {
	if ((*p & 0xc0) == 0x40)
	    *p ^= 0x20;
    }
    printf("%s\n", buf);
}

int main(int argc, char **argv) {
    if (argc != 2) {
	fprintf(stderr, "Usage: overflow <word>\n");
	return 1;
    }
    bad(argv[1]);
    return 0;
}

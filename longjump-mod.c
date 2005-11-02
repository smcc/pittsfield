#include "libc.h"

jmp_buf escape_buf;

void loop(void) {
    int i;
    for (i = 0; i < 10; i++) {
	printf("Iteration %d\n", i);
	if (i == 5)
	    longjmp(escape_buf, 1);
    }
}

int main(int argc, char **argv) {
    if (!setjmp(escape_buf)) {
	loop();
    }
    printf("Done\n");
}

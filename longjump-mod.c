#include "libc.h"

jmp_buf escape_buf;

void loop(void) {
    int i;
    for (i = 0; i < 10; i++) {
	printf("Iteration %d\n", i);
	if (i == 5)
	    longjmp(escape_buf, i);
    }
}

int main(int argc, char **argv) {
    int ret_val;
    if (!(ret_val = setjmp(escape_buf))) {
	loop();
    }
    printf("Done (%d)\n", ret_val);
    return 0;
}

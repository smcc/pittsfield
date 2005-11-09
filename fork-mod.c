#include "libc.h"

int main(int argc, char **argv) {
    int child = fork();
    if (child == -1) {
	fprintf(stderr, "Fork failed (%d)\n", errno);
    } else if (child == 0) {
	/* In child */
	/* char *kid_argv[] = {"uname", "-a", 0}; */
	printf("In child\n");
	/*execvp("/bin/uname", kid_argv);*/
	execl("/bin/uname", "uname", "-a", (char *)0);
	fprintf(stderr, "Exec failed (%d)\n", errno);
    } else {
	/* In parent */
	printf("In parent\n");
    }
    return 0;
}

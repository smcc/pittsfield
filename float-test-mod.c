/*#include "libc.c"*/
int printf(const char *fmt, ...);

float a[10];
double b[20];

int main() {
    int i;
    for (i = 0; i < 10; i++) {
	b[i] = 10.0 * i;
	b[10 + i] = i + 7;
	a[i] = b[i] / b[10 + i];
    }
    printf("%g\n", a[5] - 42.0);
    return 0;
}

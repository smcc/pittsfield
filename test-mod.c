char ary[10];

void f(char *p) {
    int i;
    for (i = 0; i < 10; i++)
	p[i] = i;
}

int main(void) {
    f(ary);
    return ary[9];
}

void *memcpy(void *dest, const void *src, unsigned int n) {
    int i;
    char *d = dest;
    const char *s = src;
    for (i = 0; i < n; i++) {
	d[i] = s[i];
    }
    return dest;
}

void *memset(void *loc, int c, unsigned int n) {
    int i;
    char *s = loc;
    for (i = 0; i < n; i++) {
	s[i] = c;
    }
    return loc;
}

int outside_access(const char *pathname, int mode);
void abort(void);
double acos(double x);
double atan(double x);
double atof(const char *nptr);
long long atoll(const char *nptr);
double ceil(double x);
int outside_chmod(const char *path, mode_t mode);
clock_t clock(void);
int outside_close(int fd);
int outside_closedir(DIR *dir);
double cos(double x);
int outside_errno(void);
void exit(int exit_val);
double exp(double x);
double fabs(double x);
int outside_fclose(int fi);
int outside_fdopen(int fd, const char *mode);
int outside_feof(int fi);
int outside_ferror(int fi);
int outside_fflush(int fi);
int outside_fileno(int fi);
double floor(double x);
int outside_fgetc(int fi);
char *outside_fgets(char *byf, int size, int fi);
int outside_fopen(const char *path, const char *mode);
int outside_fputc(int c, int fi);
int outside_fputs(const char *s, int fi);
size_t outside_fread(void *ptr, size_t size, size_t num, int fi);
void outside_free(void *ptr);
double frexp(double x, int *exp);
int outside_fseek(int fi, long offset, int whence);
#ifdef NEED_STAT
int outside_fstat(int fd, struct stat *buf);
#endif
long outside_ftell(int fi);
size_t outside_fwrite(const void *ptr, size_t size, size_t num, int fi);
char *outside_getenv(const char *name);
int getrusage(int who, struct rusage *usage);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int outside_isatty(int fd);
double ldexp(double x, int exp);
long int lrint(double x);
long int lrintf(float x);
off_t outside_lseek(int fd, off_t offset, int whence);
#ifdef NEED_STAT
int outside_lstat(const char *fname, struct stat *buf);
#endif
double log(double x);
double log10(double x);
int outside_open(const char *pathname, int flags, int mode);
DIR *outside_opendir(const char *name);
double pow(double x, double y);
int print_int(int i);
int printf(const char *fmt, ...);
int putchar(int c);
int outside_read(int fd, void *buf, size_t count);
struct dirent *outside_readdir(DIR *dir);
void outside_rewind(int fi);
double rint(double x);
void *sbrk(long inc);
int outside_select(int n, fd_set *rfds, fd_set *wfds, fd_set *xfds, struct timeval *tv);
double sin(double x);
int sprintf(char *str, const char *format, ...);
double sqrt(double x);
int sscanf(const char *str, const char *format, ...);
#ifdef NEED_STAT
int outside_stat(const char *fname, struct stat *buf);
#endif
char *outside_strerror(int errnum);
clock_t outside_times(struct tms *buf);
int outside_ungetc(int c, int fi);
int outside_unlink(const char *path);
int outside_vasprintf(char **strp, const char *fmt, va_list ap);
int outside_vfprintf(int fi, const char *fmt, va_list ap);
int outside_vfscanf(int fi, const char *fmt, va_list ap);
int outside_vsnprintf(char *str, size_t size, const char *format, va_list ap);

void fail_check(void);

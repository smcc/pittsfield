int outside_access(const char *pathname, int mode);
void abort(void);
double acos(double x);
double asin(double x);
double atan(double x);
double atan2(double y, double x);
double atof(const char *nptr);
long long atoll(const char *nptr);
double ceil(double x);
int outside_chdir(const char *path);
int outside_chmod(const char *path, mode_t mode);
void outside_clearerr(int fi);
clock_t clock(void);
int outside_close(int fd);
int outside_closedir(DIR *dir);
double cos(double x);
int outside_dup(int oldfd);
int outside_dup2(int oldfd, int newfd);
int outside_errno(void);
int outside_execv(const char *path, char *const argv[]);
int outside_execvp(const char *file, char *const argv[]);
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
pid_t outside_fork(void);
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
int outside_ftruncate(int fd, off_t length);
size_t outside_fwrite(const void *ptr, size_t size, size_t num, int fi);
char *outside_getenv(const char *name);
int getrusage(int who, struct rusage *usage);
int gettimeofday(struct timeval *tv, struct timezone *tz);
struct tm *gmtime(const time_t *timep);
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
int outside_mkdir(const char *pathname, mode_t mode);
double modf(double x, double *iptr);
int outside_open(const char *pathname, int flags, int mode);
DIR *outside_opendir(const char *name);
int outside_pipe(int fds[2]);
double pow(double x, double y);
int print_int(int i);
int printf(const char *fmt, ...);
int putchar(int c);
int outside_read(int fd, void *buf, size_t count);
struct dirent *outside_readdir(DIR *dir);
int outside_rename(const char *oldpath, const char *newpath);
void outside_rewind(int fi);
double rint(double x);
int outside_rmdir(const char *path);
void *sbrk(long inc);
void outside_setbuf(int fi, char *buf);
unsigned int sleep(unsigned int secs);
int outside_select(int n, fd_set *rfds, fd_set *wfds, fd_set *xfds, struct timeval *tv);
double sin(double x);
int sprintf(char *str, const char *format, ...);
double sqrt(double x);
int sscanf(const char *str, const char *format, ...);
#ifdef NEED_STAT
int outside_stat(const char *fname, struct stat *buf);
#endif
char *outside_strerror(int errnum);
double tan(double theta);
clock_t outside_times(struct tms *buf);
int outside_truncate(const char *path, off_t length);
mode_t umask(mode_t mask);
int outside_ungetc(int c, int fi);
int outside_unlink(const char *path);
int outside_vasprintf(char **strp, const char *fmt, va_list ap);
int outside_vfprintf(int fi, const char *fmt, va_list ap);
int outside_vfscanf(int fi, const char *fmt, va_list ap);
int outside_vsnprintf(char *str, size_t size, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
pid_t outside_wait(int *status);
int outside_write(int fd, const void *buf, size_t count);

void fail_check(void);

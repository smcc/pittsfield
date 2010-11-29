#ifndef PITTSFIELD_LIBC_H_INCLUDED
#define PITTSFIELD_LIBC_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif
typedef signed long ptrdiff_t;
typedef unsigned int size_t;
typedef int ssize_t;
#ifndef __cplusplus
typedef long wchar_t;
#endif
typedef long int off_t;
typedef int clock_t;
typedef __builtin_va_list va_list;

typedef short int16_t;
typedef unsigned short u_int16_t;
typedef int int32_t;
typedef unsigned int u_int32_t;
typedef long long int64_t;

typedef struct __dirstream DIR;

typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef int pid_t;

struct timeval {
    long tv_sec;
    long tv_usec;
};

struct timezone {
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

#define RUSAGE_SELF     0
#define RUSAGE_CHILDREN -1

struct rusage {
    struct timeval ru_utime; /* user time used */
    struct timeval ru_stime; /* system time used */
    long   ru_maxrss;        /* maximum resident set size */
    long   ru_ixrss;         /* integral shared memory size */
    long   ru_idrss;         /* integral unshared data size */
    long   ru_isrss;         /* integral unshared stack size */
    long   ru_minflt;        /* page reclaims */
    long   ru_majflt;        /* page faults */
    long   ru_nswap;         /* swaps */
    long   ru_inblock;       /* block input operations */
    long   ru_oublock;       /* block output operations */
    long   ru_msgsnd;        /* messages sent */
    long   ru_msgrcv;        /* messages received */
    long   ru_nsignals;      /* signals received */
    long   ru_nvcsw;         /* voluntary context switches */
    long   ru_nivcsw;        /* involuntary context switches */
};

struct tms {
    clock_t tms_utime;  /* user time */
    clock_t tms_stime;  /* system time */
    clock_t tms_cutime; /* user time of dead children */
    clock_t tms_cstime; /* system time of dead children */
};

typedef unsigned int mode_t;
typedef long int time_t;

struct utimbuf {
    time_t actime;
    time_t modtime;
};

typedef struct {
    long fds_bits[32];
} fd_set;

#include "sizes.h"

#define REPLACEMENT

#ifndef REPLACEMENT_INLINE
#define REPLACEMENT_INLINE extern inline
#endif

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef __STDC__
#define __STDC__
#endif

#define offsetof(type,member) ((size_t) &((type*)0)->member)

#ifdef NO_STUBS
#define FILE myFILE
#define stdin mystdin
#define stdout mystdout
#define stderr mystderr
#define fopen myfopen
#define fdopen myfdopen
#define fclose myfclose
#define ferror myferror
#define fflush myfflush
#define fgetc myfgetc
#define fprintf myfprintf
#define fread myfread
#define fwrite myfwrite
#define rewind myrewind
#define ungetc myungetc
#define memchr mymemchr

#define malloc mymalloc
#define strcpy mystrcpy
#define strdup mystrdup
#define memchr mymemchr
#define memcpy mymemcpy
#define mempcpy mymempcpy
#define memmove mymemmove
#define memset mymemset
#define memcmp mymemcmp
#define strerror mystrerror
#define perror myperror
#define getenv mygetenv
#define strlen mystrlen
#define strcmp mystrcmp
#define strncmp mystrncmp
#define strcpy mystrcpy
#define strncpy mystrncpy
#define strcat mystrcat
#define strncat mystrncat
#define strstr mystrstr
#define strdup mystrdup
#define strchr mystrchr
#define strrchr mystrrchr
#define getpagesize mygetpagesize
#define access myaccess
#define close myclose
#define fstat myfstat
#define lseek mylseek
#define open myopen
#define read myread
#define time mytime
#define times mytimes
#define opendir myopendir
#define closedir myclosedir
#define readdir myreaddir
#define isupper myisupper
#define islower myislower
#define isalpha myisalpha
#define isdigit myisdigit
#define isalnum myisalnum
#define isascii myisascii
#define isblank myisblank
#define iscntrl myiscntrl
#define isgraph myisgraph
#define isprint myisprint
#define ispunct myispunct
#define isspace myisspace
#define isxdigit myisxdigit
#define toupper mytoupper
#define tolower mytolower
#define abs myabs
#define labs mylabs
#define fopen myfopen
#define fdopen myfdopen
#define fclose myfclose
#define ferror myferror
#define fflush myfflush
#define fgetc myfgetc
#define getc mygetc
#define fputc myfputc
#define putc myputc
#define putc_unlocked myputc_unlocked
#define fputc_unlocked myfputc_unlocked
#define fputs myfputs
#define fputs_unlocked myfputs_unlocked
#define puts myputs
#define fileno myfileno
#define fprintf myfprintf
#define vfprintf myvfprintf
#define vasprintf myvasprintf
#define asprintf myasprintf
#define snprintf mysnprintf
#define fread myfread
#define fseek myfseek
#define ftell myftell
#define fwrite myfwrite
#define fwrite_unlocked myfwrite_unlocked
#define rewind myrewind
#define ungetc myungetc
#define gettext mygettext
#define malloc mymalloc
#define free myfree
#define realloc myrealloc
#define calloc mycalloc
#define vmalloc myvmalloc
#define srand mysrand
#define rand myrand
#define qsort myqsort
#define strspn mystrspn
#define strcspn mystrcspn
#define strpbrk mystrpbrk
#define strtoul mystrtoul
#define strtol mystrtol
#define atoi myatoi
#define strcasecmp mystrcasecmp
#define bsearch mybsearch
#define struct_stat struct stat
#endif
typedef int FILE;
extern FILE myfiles[16];
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

REPLACEMENT int fprintf(FILE *stream, const char *format, ...);
#ifndef HIDE_MALLOC_PROTOS
REPLACEMENT void *malloc(size_t nbytes);
#endif
REPLACEMENT_INLINE char *strcpy(char *buf, const char *src);
REPLACEMENT char *strdup(const char *s);

typedef unsigned int nlink_t;
typedef unsigned long ino_t;
typedef unsigned long long dev_t;

#ifdef NEED_STAT
struct stat {
    off_t st_size;
    int st_mode;
    time_t st_mtime;
    nlink_t st_nlink;
    ino_t st_ino;
    dev_t st_dev;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    time_t st_atime;
    time_t st_ctime;
};
#endif

#include "stubs.h"

#ifdef __cplusplus
/* Avoid "`abort()' has type `void' and is not a throw-expression",
   warning, which I don't understand. */
#define assert(e)
#else
/* Calling abort() here has the disadvantage that because it calls
   back outside the sandbox, it makes it hard for GDB to get a good
   backtrace. Let's try making a SIGTRAP instead. */
#define assert(e) ((void)((e) ? 0 : myabort()))
#endif

void myabort(void);

REPLACEMENT_INLINE void *memchr(const void *s, int c, size_t n) {
    unsigned i;
    char *cp = (char *)s;
    for (i = 0; i < n; i++) {
	if (cp[i] == c)
	    return &cp[i];
    }
    return 0;
}

REPLACEMENT_INLINE void *memcpy(void *dest, const void *src, unsigned int n) {
    unsigned i;
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (i = 0; i < n; i++) {
	d[i] = s[i];
    }
    return dest;
}

REPLACEMENT_INLINE void *mempcpy(void *dest, const void *src, unsigned int n) {
    unsigned i;
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (i = 0; i < n; i++) {
	d[i] = s[i];
    }
    return (char *)dest + n;
}

REPLACEMENT_INLINE void *memmove(void *dest, const void *src, unsigned int n) {
    unsigned i;
    char *d = (char *)dest;
    const char *s = (const char *)src;
    if (d < s) {
	for (i = 0; i < n; i++) {
	    d[i] = s[i];
	}
    } else {
	for (i = n-1; i != (unsigned)-1; i--) {
	    d[i] = s[i];
	}
    }
    return dest;
}    

REPLACEMENT_INLINE void *memset(void *loc, int c, unsigned int n) {
    unsigned i;
    char *s = (char *)loc;
    for (i = 0; i < n; i++) {
	s[i] = c;
    }
    return loc;
}

REPLACEMENT_INLINE int memcmp(const void *v1, const void *v2, size_t count) {
    unsigned i;
    const char *s1 = (const char *)v1, *s2 = (const char *)v2;
    for (i = 0; i < count; i++) {
	if (s1[i] != s2[i])
	    return s1[i] - s2[i];
    }
    return 0;
}

extern int errno;

void refresh_errno(void);

REPLACEMENT char *strerror(int errnum);
REPLACEMENT void perror(const char *command);
REPLACEMENT char *getenv(const char *name);

REPLACEMENT_INLINE size_t strlen(const char *s) {
    size_t i;
    for (i = 0; *s; s++)
	i++;
    return i;
}

REPLACEMENT_INLINE int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
	s1++;
	s2++;
    }
    return *s1 - *s2;
}

REPLACEMENT_INLINE int strncmp(const char *s1, const char *s2, size_t limit) {
    unsigned i;
    for (i = 0; i < limit; i++) {
	if (s1[i] != s2[i])
	    return s1[i] - s2[i];
	if (!s1[i])
	    return 0;
    }
    return 0;
}

REPLACEMENT_INLINE char *strcpy(char *buf, const char *src) {
    char *p;
    for (p = buf; *src; src++, p++) {
	*p = *src;
    }
    *p = 0;
    return buf;
}

REPLACEMENT_INLINE char *strncpy(char *buf, const char *src, size_t limit) {
    unsigned i;
    for (i = 0; i < limit; i++) {
	buf[i] = src[i];
	if (!src[i])
	    break;
    }
    for (; i < limit; i++) {
	buf[i] = 0;
    }
    return buf;
}

REPLACEMENT_INLINE char *strcat(char *buf, const char *extra) {
    char *p = buf;
    p += strlen(p);
    strcpy(p, extra);
    return buf;
}

REPLACEMENT_INLINE char *strncat(char *buf, const char *extra, size_t n) {
    char *p = buf;
    p += strlen(p);
    strncpy(p, extra, n);
    return buf;    
}

REPLACEMENT_INLINE const char *strstr(const char *big, const char *small) {
    size_t big_len = strlen(big);
    size_t small_len = strlen(small);
    const char *p = big;
    int count;
    if (big_len < small_len)
	return 0;
    if (!small_len)
	return big;
    count = big_len - small_len + 1;
    for (; count; count--) {
	if (!memcmp(p, small, small_len))
	    return p;
	p++;
    }
    return 0;    
}

REPLACEMENT char *strdup(const char *s);

REPLACEMENT_INLINE char *strchr(const char *s, int c) {
    char *p;
    for (p = (char *)s; *p; p++) {
	if (*p == c)
	    return p;
    }
    return 0;
}

REPLACEMENT_INLINE char *strrchr(const char *s, int c) {
    char *p, *q = 0;
    for (p = (char *)s; *p; p++) {
	if (*p == c)
	    q = p;
    }
    return q;
}

#define alloca __builtin_alloca

/* errno */

/* From linux */
#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define E2BIG            7      /* Argument list too long */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ECHILD          10      /* No child processes */
#define EAGAIN          11      /* Try again */
#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EFAULT          14      /* Bad address */
#define ENOTBLK         15      /* Block device required */
#define EBUSY           16      /* Device or resource busy */
#define EEXIST          17      /* File exists */
#define EXDEV           18      /* Cross-device link */
#define ENODEV          19      /* No such device */
#define ENOTDIR         20      /* Not a directory */
#define EISDIR          21      /* Is a directory */
#define EINVAL          22      /* Invalid argument */
#define ENFILE          23      /* File table overflow */
#define EMFILE          24      /* Too many open files */
#define ENOTTY          25      /* Not a typewriter */
#define ETXTBSY         26      /* Text file busy */
#define EFBIG           27      /* File too large */
#define ENOSPC          28      /* No space left on device */
#define ESPIPE          29      /* Illegal seek */
#define EROFS           30      /* Read-only file system */
#define EMLINK          31      /* Too many links */
#define EPIPE           32      /* Broken pipe */
#define EDOM            33      /* Math argument out of domain of func */
#define ERANGE          34      /* Math result not representable */
#define EDEADLK         35      /* Resource deadlock would occur */
#define ENAMETOOLONG    36      /* File name too long */
#define ENOLCK          37      /* No record locks available */
#define ENOSYS          38      /* Function not implemented */

extern int sys_nerr;

/* limits */

#define LONG_MAX 2147483647L
#define LONG_MIN (-LONG_MAX - 1L)
#define ULONG_MAX 4294967295UL

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)
#define UINT_MAX 4294967295U

/* fcntl */

/* from Linux */
#define O_ACCMODE          0003
#define O_RDONLY             00
#define O_WRONLY             01
#define O_RDWR               02
#define O_CREAT            0100 /* not fcntl */
#define O_EXCL             0200 /* not fcntl */
#define O_NOCTTY           0400 /* not fcntl */
#define O_TRUNC           01000 /* not fcntl */
#define O_APPEND          02000
#define O_NONBLOCK        04000
#define O_NDELAY        O_NONBLOCK
#define O_SYNC           010000
#define O_FSYNC          O_SYNC
#define O_ASYNC          020000

/* system calls */

/* from Linux */
#define S_IFMT        0170000 /* These bits determine file type.  */
#define S_IFDIR       0040000 /* Directory.  */
#define S_IFCHR       0020000 /* Character device.  */
#define S_IFBLK       0060000 /* Block device.  */
#define S_IFREG       0100000 /* Regular file.  */
#define S_IFIFO       0010000 /* FIFO.  */
#define S_IFLNK       0120000 /* Symbolic link.  */
#define S_IFSOCK      0140000 /* Socket.  */
#define S_ISUID       04000   /* Set user ID on execution.  */
#define S_ISGID       02000   /* Set group ID on execution.  */
#define S_ISVTX       01000   /* Save swapped text after use (sticky).  */
#define S_IREAD       0400    /* Read by owner.  */
#define S_IWRITE      0200    /* Write by owner.  */
#define S_IEXEC       0100    /* Execute by owner.  */


/* Signals, from Linux  */
#define SIGHUP          1       /* Hangup (POSIX).  */
#define SIGINT          2       /* Interrupt (ANSI).  */
#define SIGQUIT         3       /* Quit (POSIX).  */
#define SIGILL          4       /* Illegal instruction (ANSI).  */
#define SIGTRAP         5       /* Trace trap (POSIX).  */
#define SIGABRT         6       /* Abort (ANSI).  */
#define SIGIOT          6       /* IOT trap (4.2 BSD).  */
#define SIGBUS          7       /* BUS error (4.2 BSD).  */
#define SIGFPE          8       /* Floating-point exception (ANSI).  */
#define SIGKILL         9       /* Kill, unblockable (POSIX).  */
#define SIGUSR1         10      /* User-defined signal 1 (POSIX).  */
#define SIGSEGV         11      /* Segmentation violation (ANSI).  */
#define SIGUSR2         12      /* User-defined signal 2 (POSIX).  */
#define SIGPIPE         13      /* Broken pipe (POSIX).  */
#define SIGALRM         14      /* Alarm clock (POSIX).  */
#define SIGTERM         15      /* Termination (ANSI).  */
#define SIGSTKFLT       16      /* Stack fault.  */
#define SIGCLD          SIGCHLD /* Same as SIGCHLD (System V).  */
#define SIGCHLD         17      /* Child status has changed (POSIX).  */
#define SIGCONT         18      /* Continue (POSIX).  */
#define SIGSTOP         19      /* Stop, unblockable (POSIX).  */
#define SIGTSTP         20      /* Keyboard stop (POSIX).  */
#define SIGTTIN         21      /* Background read from tty (POSIX).  */
#define SIGTTOU         22      /* Background write to tty (POSIX).  */
#define SIGURG          23      /* Urgent condition on socket (4.2 BSD).  */
#define SIGXCPU         24      /* CPU limit exceeded (4.2 BSD).  */
#define SIGXFSZ         25      /* File size limit exceeded (4.2 BSD).  */
#define SIGVTALRM       26      /* Virtual alarm clock (4.2 BSD).  */
#define SIGPROF         27      /* Profiling alarm clock (4.2 BSD).  */
#define SIGWINCH        28      /* Window size change (4.3 BSD, Sun).  */
#define SIGPOLL         SIGIO   /* Pollable event occurred (System V).  */
#define SIGIO           29      /* I/O now possible (4.2 BSD).  */
#define SIGPWR          30      /* Power failure restart (System V).  */
#define SIGSYS          31      /* Bad system call.  */

struct dirent {
    int d_ino;
    off_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256]; 
};

typedef void (*sighandler_t)(int);
#define SIG_ERR ((sighandler_t)-1)
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)

#define FD_ZERO(set)      memset((set), 0, sizeof(fd_set))
#define FD_SET(d, set)   ((set)->fds_bits[(d)/32] |=  (1 << ((d) % 32)))
#define FD_CLR(d, set)   ((set)->fds_bits[(d)/32] &= ~(1 << ((d) % 32)))
#define FD_ISSET(d, set) ((set)->fds_bits[(d)/32] &   (1 << ((d) % 32)))

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#define _SC_CLK_TCK 2

REPLACEMENT int getpagesize();
REPLACEMENT char *getcwd(char *buf, size_t size);
REPLACEMENT int access(const char *pathname, int mode);
REPLACEMENT int close(int fd);
REPLACEMENT int chdir(const char *path);
REPLACEMENT int chmod(const char *path, mode_t mode);
REPLACEMENT char *ctime(const time_t *timep);
REPLACEMENT int dup(int oldfd);
REPLACEMENT int dup2(int oldfd, int newfd);
REPLACEMENT int execl(const char *path, const char *arg, ...); 
REPLACEMENT int execlp(const char *file, const char *arg,...);
REPLACEMENT int execle(const char *path, const char *arg,...);
REPLACEMENT int execv(const char *path, char *const argv[]);
REPLACEMENT int execvp(const char *file, char *const argv[]);
REPLACEMENT int execve(const char *path, char *const argv[],
		       char *const envp[]);
REPLACEMENT void _exit(int status);
REPLACEMENT int fcntl(int fd, int cmd, ...);
REPLACEMENT pid_t fork(void);
#ifdef NEED_STAT
REPLACEMENT int fstat(int fd, struct stat *buf);
#endif
REPLACEMENT int ftruncate(int fd, off_t length);
REPLACEMENT uid_t getuid(void);
REPLACEMENT uid_t geteuid(void);
REPLACEMENT gid_t getgid(void);
REPLACEMENT gid_t getegid(void);
REPLACEMENT pid_t getpid(void);
REPLACEMENT int ioctl(int fd, int request, ...);
REPLACEMENT int isatty(int fd);
REPLACEMENT int kill(pid_t pid, int sig);
REPLACEMENT off_t lseek(int fd, off_t offset, int whence);
REPLACEMENT struct tm *localtime(const time_t *timep);
REPLACEMENT int mkdir(const char *pathname, mode_t mode);
REPLACEMENT int open(const char *pathname, int flags, ...);
REPLACEMENT int pipe(int fds[2]);
REPLACEMENT int read(int fd, void *buf, size_t count);
REPLACEMENT int rename(const char *oldpath, const char *newpath);
REPLACEMENT int rmdir(const char *path);
REPLACEMENT int select(int n, fd_set *rfds, fd_set *wfds, fd_set *xfds, 
		       struct timeval *tv);
REPLACEMENT int setuid(uid_t uid);
REPLACEMENT int setgid(gid_t gid);
#ifdef NEED_STAT
REPLACEMENT int mystat(const char *file_name, struct stat *buf);
#ifndef NO_STUBS
REPLACEMENT int stat(const char *file_name, struct stat *buf);
#endif
#endif
REPLACEMENT sighandler_t signal(int signum, sighandler_t handler);
REPLACEMENT long sysconf(int name);
REPLACEMENT int system(const char *string);
REPLACEMENT time_t time(time_t *t);
REPLACEMENT clock_t times(struct tms *buf);
REPLACEMENT int truncate(const char *path, off_t length);
REPLACEMENT char *ttyname(int fd);
REPLACEMENT int unlink(const char *path);
REPLACEMENT int utime(const char *filename, const struct utimbuf *buf);
REPLACEMENT pid_t wait(int *status);
REPLACEMENT ssize_t write(int fd, const void *buf, size_t count);

/* directories */

REPLACEMENT DIR *opendir(const char *name);
REPLACEMENT int closedir(DIR *dir);
REPLACEMENT struct dirent *readdir(DIR *dir);

/* ctype */

REPLACEMENT int isupper(int c);
REPLACEMENT int islower(int c);
REPLACEMENT int isalpha(int c);
REPLACEMENT int isdigit(int c);
REPLACEMENT int isalnum(int c);
REPLACEMENT int isascii(int c);
REPLACEMENT int isblank(int c);
REPLACEMENT int iscntrl(int c);
REPLACEMENT int isgraph(int c);
REPLACEMENT int isprint(int c);
REPLACEMENT int ispunct(int c);
REPLACEMENT int isspace(int c);
REPLACEMENT int isxdigit(int c);
REPLACEMENT int toupper(int c);
REPLACEMENT int tolower(int c);

/* math */

#define HUGE_VAL       (__extension__ 0x1.0p2047)

REPLACEMENT int abs(int x);
REPLACEMENT int labs(long x);

/* stdarg */

#define va_start(v,l)   __builtin_va_start((v),l)
#define va_end          __builtin_va_end
#define va_arg          __builtin_va_arg
#define va_copy(d,s)    __builtin_va_copy((d),(s))

/* stdio */

#define EOF (-1)
#define BUFSIZ 8192

REPLACEMENT int atol(const char *nptr);

#define getchar() fgetc(stdin)

REPLACEMENT void clearerr(FILE *stream);
REPLACEMENT FILE *fopen(const char *path, const char *mode);
REPLACEMENT FILE *fdopen(int fd, const char *mode);
REPLACEMENT int fclose(FILE *stream);
REPLACEMENT int ferror(FILE *stream);
REPLACEMENT int fflush(FILE *stream);
REPLACEMENT int fgetc(FILE *stream);
REPLACEMENT int getc(FILE *stream);
REPLACEMENT char *fgets(char *buf, int size, FILE *stream);
REPLACEMENT char *gets(char *buf);
REPLACEMENT int fputc(int c, FILE *stream);
REPLACEMENT int putc(int c, FILE *stream);
REPLACEMENT int putc_unlocked(int c, FILE *stream);
REPLACEMENT int fputc_unlocked(int c, FILE *stream);
REPLACEMENT int fputs(const char *s, FILE *stream);
REPLACEMENT int fputs_unlocked(const char *s, FILE *stream);
REPLACEMENT int puts(const char *s);
REPLACEMENT int fileno(FILE *stream);
REPLACEMENT int feof(FILE *stream);
REPLACEMENT int fprintf(FILE *stream, const char *format, ...);
REPLACEMENT int vfprintf(FILE *stream, const char *format, va_list ap);
REPLACEMENT int vasprintf(char **strp, const char *fmt, va_list ap);
REPLACEMENT void setbuf(FILE *stream, char *buf);
REPLACEMENT int asprintf(char **str, const char *fmt, ...);
REPLACEMENT int snprintf(char *str, size_t size, const char *format, ...);
REPLACEMENT int fscanf(FILE *stream, const char *fmt, ...);
REPLACEMENT int scanf(const char *fmt, ...);
REPLACEMENT size_t fread(void *ptr, size_t size, size_t num, FILE *stream);
REPLACEMENT int remove(const char *path);
REPLACEMENT FILE *tmpfile(void);
REPLACEMENT char *tmpnam(char *s);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

REPLACEMENT int fseek(FILE *fp, long offset, int whence);
REPLACEMENT long ftell(FILE *stream);
REPLACEMENT size_t fwrite(const void *ptr, size_t size, size_t num,
			  FILE *stream);
REPLACEMENT size_t fwrite_unlocked(const void *ptr, size_t size, size_t num,
				   FILE *stream);
REPLACEMENT void rewind(FILE *stream);
REPLACEMENT int ungetc(int c, FILE *stream);

/* internationalization */
REPLACEMENT const char *gettext(const char *msgid);

/* termio(s) */
typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

struct termio {
    tcflag_t c_iflag;
    tcflag_t c_lflag;
    cc_t c_cc[32];
};

#define HZ 100

#define VINTR 0
#define VQUIT 1
#define VERASE 2
#define VKILL 3
#define VEOF 4
#define VTIME 5
#define VMIN 6
#define VSWTC 7
#define VSTART 8
#define VSTOP 9
#define VSUSP 10
#define VEOL 11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE 14
#define VLNEXT 15
#define VEOL2 16
#define IGNBRK  0000001
#define BRKINT  0000002
#define IGNPAR  0000004
#define PARMRK  0000010
#define INPCK   0000020
#define ISTRIP  0000040
#define INLCR   0000100
#define IGNCR   0000200
#define ICRNL   0000400
#define IUCLC   0001000
#define IXON    0002000
#define IXANY   0004000
#define IXOFF   0010000
#define IMAXBEL 0020000
#define ISIG    0000001
#define ICANON  0000002
#define ECHO    0000010
#define ECHOE   0000020
#define ECHOK   0000040
#define ECHONL  0000100
#define NOFLSH  0000200
#define TOSTOP  0000400
#define TCGETA                0x5405
#define TCSETAW               0x5407


/* I couldn't find any documentation for __builtin_setjmp; this layout for
   jmp_buf was determined empirically. */
typedef struct {
    int frame_ptr;
    int return_addr;
    int stack_ptr;
    /* -- Above here is used by __builtin_(set|long)jmp */
    int return_value;
} jmp_buf[1];

#define setjmp(env) ((env)[0].return_value = 0,	\
                     __builtin_setjmp(env),\
                     (env)[0].return_value)

#define longjmp(env,val) ((env)[0].return_value = (val),\
                          __builtin_longjmp((env), 1)) 

extern char **environ;

#ifndef REAL_MALLOC

/* 300.twolf wants to declare these itself, incorrectly. */
#ifndef HIDE_MALLOC_PROTOS
REPLACEMENT void *malloc(unsigned nbytes);
#ifdef TWO_ARGUMENT_FREE
  /* c.f. 300.twolf lists.h line 67 */
REPLACEMENT void free(void *vp, ...);
#else
REPLACEMENT void free(void *vp);
#endif
REPLACEMENT void *realloc(void *vp, unsigned nbytes);
#endif

#else

void free(void *ptr);
void *realloc(void *vp, unsigned nbytes);

#endif

/* calloc, vmalloc by SMcC */
#ifndef HIDE_MALLOC_PROTOS
REPLACEMENT void *calloc(size_t nmemb, size_t size);
REPLACEMENT void *vmalloc(size_t size);
#endif

/* ------- Code below this line came from dietlibc, under the GPL -------- */

/* ------- code below here has been significantly edited  -------- */

REPLACEMENT void srand(unsigned int seed);

/* Knuth's TAOCP section 3.6 */
// FIXME: ISO C/SuS want a longer period
REPLACEMENT int rand(void);

REPLACEMENT void qsort(void *base, size_t nmemb, size_t size,
		       int (*compar)(const void *, const void *));

REPLACEMENT size_t strspn(const char *s, const char *set);

REPLACEMENT size_t strcspn(const char *s, const char *reject);

REPLACEMENT char *strtok(char *s, const char *delim);

REPLACEMENT char *strpbrk(const char *s, const char *accept);
REPLACEMENT unsigned long int strtoul(const char *ptr, char **endptr,
				      int base);
REPLACEMENT long int strtol(const char *nptr, char **endptr, int base);
REPLACEMENT int atoi(const char *nptr);
REPLACEMENT int strcasecmp(const char *s1, const char *s2);
REPLACEMENT void *bsearch(const void *key, const void *base, size_t nmemb,
			  size_t size,
			  int (*compar)(const void*, const void*));

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PITTSFIELD_LIBC_H_INCLUDED */

#ifndef PITTSFIELD_LIBC_H_INCLUDED
#define PITTSFIELD_LIBC_H_INCLUDED
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

struct timeval {
    long tv_sec;
    long tv_usec;
};

struct timezone {
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

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

#include "sizes.h"

#ifdef NO_STUBS
/* Hide these from outside.c, so it can get the real ones */
#define REPLACEMENT
#define REPLACEMENT_STATIC static
#else
#define REPLACEMENT
#define REPLACEMENT_STATIC static
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
#define files myfiles
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
extern FILE files[16];
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

REPLACEMENT int fprintf(FILE *stream, const char *format, ...);
REPLACEMENT void *malloc(size_t nbytes);
REPLACEMENT_STATIC char *strcpy(char *buf, const char *src);
REPLACEMENT char *strdup(const char *s);

typedef unsigned int nlink_t;
typedef unsigned long ino_t;
typedef unsigned long long dev_t;

struct stat {
    off_t st_size;
    int st_mode;
    time_t st_mtime;
    nlink_t st_nlink;
    ino_t st_ino;
    dev_t st_dev;
};

#include "stubs.h"

#define assert(e) ((void)((e) ? 0 : abort()))

REPLACEMENT_STATIC inline void *memchr(const void *s, int c, size_t n) {
    unsigned i;
    char *cp = (char *)s;
    for (i = 0; i < n; i++) {
	if (cp[i] == c)
	    return &cp[i];
    }
    return 0;
}

REPLACEMENT_STATIC inline void *memcpy(void *dest, const void *src, unsigned int n) {
    unsigned i;
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (i = 0; i < n; i++) {
	d[i] = s[i];
    }
    return dest;
}

REPLACEMENT_STATIC inline void *mempcpy(void *dest, const void *src, unsigned int n) {
    unsigned i;
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (i = 0; i < n; i++) {
	d[i] = s[i];
    }
    return (char *)dest + n;
}

REPLACEMENT_STATIC inline void *memmove(void *dest, const void *src, unsigned int n) {
    unsigned i;
    char *d = (char *)dest;
    const char *s = (const char *)src;
    if (d < s) {
	for (i = 0; i < n; i++) {
	    d[i] = s[i];
	}
    } else {
	for (i = n-1; i >= 0; i--) {
	    d[i] = s[i];
	}
    }
    return dest;
}    

REPLACEMENT_STATIC inline void *memset(void *loc, int c, unsigned int n) {
    unsigned i;
    char *s = (char *)loc;
    for (i = 0; i < n; i++) {
	s[i] = c;
    }
    return loc;
}

REPLACEMENT_STATIC inline int memcmp(const void *v1, const void *v2, size_t count) {
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

REPLACEMENT_STATIC inline size_t strlen(const char *s) {
    size_t i;
    for (i = 0; *s; s++)
	i++;
    return i;
}

REPLACEMENT_STATIC inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
	s1++;
	s2++;
    }
    return *s1 - *s2;
}

REPLACEMENT_STATIC inline int strncmp(const char *s1, const char *s2, size_t limit) {
    unsigned i;
    for (i = 0; i < limit; i++) {
	if (s1[i] != s2[i])
	    return s1[i] - s2[i];
	if (!s1[i])
	    return 0;
    }
    return 0;
}

REPLACEMENT_STATIC inline char *strcpy(char *buf, const char *src) {
    char *p;
    for (p = buf; *src; src++, p++) {
	*p = *src;
    }
    *p = 0;
    return buf;
}

REPLACEMENT_STATIC inline char *strncpy(char *buf, const char *src, size_t limit) {
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

REPLACEMENT_STATIC inline char *strcat(char *buf, const char *extra) {
    char *p = buf;
    p += strlen(p);
    strcpy(p, extra);
    return buf;
}

REPLACEMENT_STATIC inline char *strncat(char *buf, const char *extra, size_t n) {
    char *p = buf;
    p += strlen(p);
    strncpy(p, extra, n);
    return buf;    
}

REPLACEMENT_STATIC inline const char *strstr(const char *big, const char *small) {
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

REPLACEMENT_STATIC inline char *strchr(const char *s, int c) {
    char *p;
    for (p = (char *)s; *p; p++) {
	if (*p == c)
	    return p;
    }
    return 0;
}

REPLACEMENT_STATIC inline char *strrchr(const char *s, int c) {
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

struct dirent {
    int d_ino;
    off_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256]; 
};

REPLACEMENT int getpagesize();
REPLACEMENT int access(const char *pathname, int mode);
REPLACEMENT int close(int fd);
REPLACEMENT int chmod(const char *path, mode_t mode);
REPLACEMENT char *ctime(const time_t *timep);
REPLACEMENT int fstat(int fd, struct stat *buf);
REPLACEMENT int isatty(int fd);
REPLACEMENT off_t lseek(int fd, off_t offset, int whence);
REPLACEMENT int open(const char *pathname, int flags, ...);
REPLACEMENT int read(int fd, void *buf, size_t count);
REPLACEMENT int mystat(const char *file_name, struct stat *buf);
#ifndef NO_STUBS
REPLACEMENT int stat(const char *file_name, struct stat *buf);
#endif
REPLACEMENT time_t time(time_t *t);
REPLACEMENT clock_t times(struct tms *buf);
REPLACEMENT int unlink(const char *path);

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

#define va_start(v,l)   __builtin_stdarg_start((v),l)
#define va_end          __builtin_va_end
#define va_arg          __builtin_va_arg
#define va_copy(d,s)    __builtin_va_copy((d),(s))

/* stdio */

#define EOF (-1)
#define BUFSIZ 4096

REPLACEMENT int atol(const char *nptr);

#define getchar() fgetc(stdin)

REPLACEMENT FILE *fopen(const char *path, const char *mode);
REPLACEMENT FILE *fdopen(int fd, const char *mode);
REPLACEMENT int fclose(FILE *stream);
REPLACEMENT int ferror(FILE *stream);
REPLACEMENT int fflush(FILE *stream);
REPLACEMENT int fgetc(FILE *stream);
REPLACEMENT int getc(FILE *stream);
REPLACEMENT char *fgets(char *buf, int size, FILE *stream);
REPLACEMENT int fputc(int c, FILE *stream);
REPLACEMENT int putc(int c, FILE *stream);
REPLACEMENT int putc_unlocked(int c, FILE *stream);
REPLACEMENT int fputc_unlocked(int c, FILE *stream);
REPLACEMENT int fputs(const char *s, FILE *stream);
REPLACEMENT int fputs_unlocked(const char *s, FILE *stream);
REPLACEMENT int puts(const char *s);
REPLACEMENT int fileno(FILE *stream);
REPLACEMENT int fprintf(FILE *stream, const char *format, ...);
REPLACEMENT int vfprintf(FILE *stream, const char *format, va_list ap);
REPLACEMENT int vasprintf(char **strp, const char *fmt, va_list ap);
REPLACEMENT int asprintf(char **str, const char *fmt, ...);
REPLACEMENT int snprintf(char *str, size_t size, const char *format, ...);
REPLACEMENT size_t fread(void *ptr, size_t size, size_t num, FILE *stream);
REPLACEMENT int remove(const char *path);

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

#ifndef REAL_MALLOC

REPLACEMENT void *malloc(unsigned nbytes);

REPLACEMENT void free(void *vp);
REPLACEMENT void *realloc(void *vp, unsigned nbytes);

#else

void free(void *ptr);
void *realloc(void *vp, unsigned nbytes);

#endif

/* calloc, vmalloc by SMcC */
REPLACEMENT void *calloc(size_t nmemb, size_t size);
REPLACEMENT void *vmalloc(size_t size);

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

REPLACEMENT char *strpbrk(const char *s, const char *accept);
REPLACEMENT unsigned long int strtoul(const char *ptr, char **endptr,
				      int base);
REPLACEMENT long int strtol(const char *nptr, char **endptr, int base);
REPLACEMENT int atoi(const char *nptr);
REPLACEMENT int strcasecmp(const char *s1, const char *s2);
REPLACEMENT void *bsearch(const void *key, const void *base, size_t nmemb,
			  size_t size,
			  int (*compar)(const void*, const void*));
#endif /* PITTSFIELD_LIBC_H_INCLUDED */

typedef signed long ptrdiff_t;
typedef unsigned int size_t;
typedef int ssize_t;
typedef long wchar_t;
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

typedef long int time_t;

struct stat {
    off_t st_size;
    int st_mode;
    time_t st_mtime;
};

#include "stubs.h"

#include "sizes.h"

#ifdef NO_STUBS
/* Hide these from outside.c, so it can get the real ones */
#define REPLACEMENT static
#else
#define REPLACEMENT
#endif

#ifndef NULL
#define NULL (void*)0
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
#endif
typedef int FILE;
FILE files[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
FILE *stdin = &files[0];
FILE *stdout = &files[1];
FILE *stderr = &files[2];

REPLACEMENT int fprintf(FILE *stream, const char *format, ...);
REPLACEMENT void *malloc(size_t nbytes);
REPLACEMENT char *strcpy(char *buf, const char *src);
REPLACEMENT char *strdup(const char *s);

#define assert(e) ((void)((e) ? 0 : abort()))

REPLACEMENT void *memchr(const void *s, int c, size_t n) {
    int i;
    char *cp = (char *)s;
    for (i = 0; i < n; i++) {
	if (cp[i] == c)
	    return &cp[i];
    }
    return 0;
}

REPLACEMENT void *memcpy(void *dest, const void *src, unsigned int n) {
    int i;
    char *d = dest;
    const char *s = src;
    for (i = 0; i < n; i++) {
	d[i] = s[i];
    }
    return dest;
}

REPLACEMENT void *mempcpy(void *dest, const void *src, unsigned int n) {
    int i;
    char *d = dest;
    const char *s = src;
    for (i = 0; i < n; i++) {
	d[i] = s[i];
    }
    return dest + n;
}

REPLACEMENT void *memmove(void *dest, const void *src, unsigned int n) {
    int i;
    char *d = dest;
    const char *s = src;
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

REPLACEMENT void *memset(void *loc, int c, unsigned int n) {
    int i;
    char *s = loc;
    for (i = 0; i < n; i++) {
	s[i] = c;
    }
    return loc;
}

REPLACEMENT int memcmp(const void *v1, const void *v2, size_t count) {
    int i;
    const char *s1 = v1, *s2 = v2;
    for (i = 0; i < count; i++) {
	if (s1[i] != s2[i])
	    return s1[i] - s2[i];
    }
    return 0;
}

#if 0
/* The world's smallest malloc implementation (TM).
   Has a minor memory leak problem, though */
REPLACEMENT void *malloc(size_t size) {
    void *new_brk = sbrk(size);
    return new_brk - size;
}

REPLACEMENT void free(void *ptr) {
    return;
}
#endif

int errno;

void refresh_errno(void) {
#ifdef NO_STUBS
#else
    errno = outside_errno();
#endif
}

REPLACEMENT char *strerror(int errnum) {
    /*static char buf[256];
      strcpy(buf, outside_strerror(errnum));
      return buf;*/
    return "an error message";
}
/* extern char *strerror(int errnum);*/

REPLACEMENT void perror(const char *command) {
    fprintf(stderr, "%s: %s\n", command, strerror(errno));
}

REPLACEMENT char *getenv(const char *name) {
    char *v = outside_getenv(name);
    if (v)
	return strdup(v);
    else
	return v;
}

REPLACEMENT size_t strlen(const char *s) {
    size_t i;
    for (i = 0; *s; s++)
	i++;
    return i;
}

REPLACEMENT int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
	s1++;
	s2++;
    }
    return *s1 - *s2;
}

REPLACEMENT int strncmp(const char *s1, const char *s2, size_t limit) {
    int i;
    for (i = 0; i < limit; i++) {
	if (s1[i] != s2[i])
	    return s1[i] - s2[i];
	if (!s1[i])
	    return 0;
    }
    return 0;
}

REPLACEMENT char *strcpy(char *buf, const char *src) {
    char *p;
    for (p = buf; *src; src++, p++) {
	*p = *src;
    }
    *p = 0;
    return buf;
}

REPLACEMENT char *strncpy(char *buf, const char *src, size_t limit) {
    int i;
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

REPLACEMENT char *strcat(char *buf, const char *extra) {
    char *p = buf;
    p += strlen(p);
    strcpy(p, extra);
    return buf;
}

REPLACEMENT char *strncat(char *buf, const char *extra, size_t n) {
    char *p = buf;
    p += strlen(p);
    strncpy(p, extra, n);
    return buf;    
}

REPLACEMENT const char *strstr(const char *big, const char *small) {
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

REPLACEMENT char *strdup(const char *s) {
    char *buf = malloc(strlen(s) + 1);
    if (buf)
	strcpy(buf, s);
    return buf;
}

REPLACEMENT char *strchr(const char *s, int c) {
    char *p;
    for (p = (char *)s; *p; p++) {
	if (*p == c)
	    return p;
    }
    return 0;
}

REPLACEMENT char *strrchr(const char *s, int c) {
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

int sys_nerr = 34;

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

REPLACEMENT int getpagesize() {
    return 8192;
}

REPLACEMENT int access(const char *pathname, int mode) {
    int ret = outside_access(pathname, mode);
    refresh_errno();
    return ret;
}

REPLACEMENT int close(int fd) {
    int ret = outside_close(fd);
    refresh_errno();
    return ret;
}

REPLACEMENT int fstat(int fd, struct stat *buf) {
    int ret = outside_fstat(fd, buf);
    refresh_errno();
    return ret;    
}    

REPLACEMENT off_t lseek(int fd, off_t offset, int whence) {
    int ret = outside_lseek(fd, offset, whence);
    refresh_errno();
    return ret;
}

REPLACEMENT int open(const char *pathname, int flags, int mode) {
    int ret = outside_open(pathname, flags, mode);
    refresh_errno();
    return ret;    
}

REPLACEMENT int read(int fd, void *buf, size_t count) {
    int ret = outside_read(fd, buf, count);
    refresh_errno();
    return ret;    
}

REPLACEMENT int stat(const char *file_name, struct stat *buf) {
    int ret = outside_stat(file_name, buf);
    refresh_errno();
    return ret;    
}    

REPLACEMENT time_t time(time_t *t) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    if (t)
	*t = tv.tv_sec;
    return tv.tv_sec;
}

REPLACEMENT clock_t times(struct tms *buf) {
    clock_t ret = outside_times(buf);
    refresh_errno();
    return ret;    
}	

/* directories */

REPLACEMENT DIR *opendir(const char *name) {
    DIR *ret = outside_opendir(name);
    refresh_errno();
    return ret;
}

REPLACEMENT int closedir(DIR *dir) {
    int ret = outside_closedir(dir);
    refresh_errno();
    return ret;
}

REPLACEMENT struct dirent *readdir(DIR *dir) {
    struct dirent *ret = outside_readdir(dir); 
    refresh_errno();
    return ret;
}

/* ctype */

REPLACEMENT int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

REPLACEMENT int islower(int c) {
    return c >= 'a' && c <= 'z';
}

REPLACEMENT int isalpha(int c) {
    return isupper(c) || islower(c);
}

REPLACEMENT int isdigit(int c) {
    return c >= '0' && c <= '9';
}

REPLACEMENT int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

REPLACEMENT int isascii(int c) {
    return !(c & 0x80);
}

REPLACEMENT int isblank(int c) {
    return c == ' ' || c == '\t';
}

REPLACEMENT int iscntrl(int c) {
    return (c >= '\0' && c < ' ') || c == 0x7f;
}

REPLACEMENT int isgraph(int c) {
    return c >= '!' && c <= '~';
}

REPLACEMENT int isprint(int c) {
    return isgraph(c) || c == ' ';
}

REPLACEMENT int ispunct(int c) {
    return isgraph(c) && !isalnum(c);
}

REPLACEMENT int isspace(int c) {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t'
	|| c == '\v';
}

REPLACEMENT int isxdigit(int c) {
    return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

REPLACEMENT int toupper(int c) {
    if (islower(c))
	return 'c' - 'a' + 'A';
    else
	return c;
}

REPLACEMENT int tolower(int c) {
    if (isupper(c))
	return 'c' - 'A' + 'a';
    else
	return c;
}

/* math */

#define HUGE_VAL       (__extension__ 0x1.0p2047)

REPLACEMENT int abs(int x) {
    return x > 0 ? x : -x;
}

REPLACEMENT int labs(long x) {
    return x > 0 ? x : -x;
}

/* stdarg */

#define va_start(v,l)   __builtin_stdarg_start((v),l)
#define va_end          __builtin_va_end
#define va_arg          __builtin_va_arg
#define va_copy(d,s)    __builtin_va_copy((d),(s))

/* stdio */

#define EOF (-1)
#define BUFSIZ 4096

REPLACEMENT FILE *fopen(const char *path, const char *mode) {
    int fi = outside_fopen(path, mode);
    if (fi == -1) {
	refresh_errno();
	return 0;
    }
    return &files[fi];
}

REPLACEMENT FILE *fdopen(int fd, const char *mode) {
    int fi = outside_fdopen(fd, mode);
    if (fi == -1) {
	refresh_errno();
	return 0;
    }
    return &files[fi];
}

REPLACEMENT int fclose(FILE *stream) {
    int ret = outside_fclose(*stream);
    if (ret)
	refresh_errno();
    return ret;
}

REPLACEMENT int ferror(FILE *stream) {
    return outside_ferror(*stream);
}

REPLACEMENT int fflush(FILE *stream) {
    int ret = outside_fflush(*stream);
    if (ret)
	refresh_errno();
    return ret;
}

REPLACEMENT int fgetc(FILE *stream) {
    int ret = outside_fgetc(*stream);
    if (ret == EOF)
	refresh_errno();
    return ret;
}

REPLACEMENT int getc(FILE *stream) {
    return fgetc(stream);
}

REPLACEMENT int fputc(int c, FILE *stream) {
    int ret = outside_fputc(c, *stream);
    if (ret == EOF)
	refresh_errno();
    return ret;
}

REPLACEMENT int putc(int c, FILE *stream) {
    return fputc(c, stream);
}

REPLACEMENT int putc_unlocked(int c, FILE *stream) {
    return fputc(c, stream);
}

REPLACEMENT int fputc_unlocked(int c, FILE *stream) {
    return fputc(c, stream);
}

REPLACEMENT int fputs(const char *s, FILE *stream) {
    int ret = outside_fputs(s, *stream);
    if (ret == EOF)
	refresh_errno();
    return ret;
}    

REPLACEMENT int fputs_unlocked(const char *s, FILE *stream) {
    return fputs(s, stream);
}
    
REPLACEMENT int puts(const char *s) {
    int ret = 0;
    ret |= fputs(s, stdout);
    ret |= fputc('\n', stdout);
    return ret;
}

REPLACEMENT int fileno(FILE *stream) {
    int ret = outside_fileno(*stream);
    if (ret == -1)
	refresh_errno();
    return ret;
}

REPLACEMENT int fprintf(FILE *stream, const char *format, ...) {
    va_list args;
    int ret;
    va_start(args, format);
    ret = outside_vfprintf(*stream, format, args);
    va_end(args);
    return ret;
}

REPLACEMENT int vfprintf(FILE *stream, const char *format, va_list ap) {
    return outside_vfprintf(*stream, format, ap);
}

REPLACEMENT int vasprintf(char **strp, const char *fmt, va_list ap) {
    char *outside_str;
    int ret = outside_vasprintf(&outside_str, fmt, ap);
    if (ret != -1) {
	*strp = strdup(outside_str);
	outside_free(outside_str);
    }
    return ret;
}

REPLACEMENT int asprintf(char **str, const char *fmt, ...) {
    va_list args;
    int ret;
    va_start(args, fmt);
    ret = vasprintf(str, fmt, args);
    va_end(args);
    return ret;
}

REPLACEMENT int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    int ret;
    va_start(args, format);
    ret = outside_vsnprintf(str, size, format, args);
    va_end(args);
    return ret;
}

REPLACEMENT size_t fread(void *ptr, size_t size, size_t num, FILE *stream) {
    int ret = outside_fread(ptr, size, num, *stream);
    return ret;
}

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

REPLACEMENT int fseek(FILE *fp, long offset, int whence) {
    int ret = outside_fseek(*fp, offset, whence);
    if (ret == -1)
	refresh_errno();
    return ret;
}

REPLACEMENT long ftell(FILE *stream) {
    long ret = outside_ftell(*stream);
    if (ret == -1)
	refresh_errno();
    return ret;
}

REPLACEMENT size_t fwrite(const void *ptr, size_t size, size_t num,
			  FILE *stream) {
    int ret = outside_fwrite(ptr, size, num, *stream);
    return ret;
}

REPLACEMENT size_t fwrite_unlocked(const void *ptr, size_t size, size_t num,
		       FILE *stream) {
    return fwrite(ptr, size, num, stream);
}

REPLACEMENT void rewind(FILE *stream) {
    outside_rewind(*stream);
}

REPLACEMENT int ungetc(int c, FILE *stream) {
    int ret = outside_ungetc(c, *stream);
    if (ret == EOF)
	refresh_errno();
    return ret;
}

/* internationalization */
REPLACEMENT const char *gettext(const char *msgid) {
    return msgid;
}

#ifndef REAL_MALLOC

/* --------- The classic BSD (Kingsley) malloc implementation ----------- */

/* Copied from the version in Perl 1, with some features removed -SMcC */
/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are 2^n-4 (or 2^n-12) bytes long.
 * This is designed for use in a program that uses vast quantities of memory,
 * but bombs when it runs out. 
 */

#define u_char unsigned char
#define u_int unsigned int
#define u_short unsigned short

/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled and the size of the block fits
 * in two bytes, then the top two bytes hold the size of the requested block
 * plus the range checking words, and the header word MINUS ONE.
 */
union   overhead {
        union   overhead *ov_next;      /* when free */
        struct {
                u_char  ovu_magic;      /* magic number */
                u_char  ovu_index;      /* bucket # */
        } ovu;
#define ov_magic        ovu.ovu_magic
#define ov_index        ovu.ovu_index
#define ov_size         ovu.ovu_size
#define ov_rmagic       ovu.ovu_rmagic
};

#define MAGIC           0xff            /* magic # on accounting info */
#define RSLOP           0

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define NBUCKETS 30
static  union overhead *nextf[NBUCKETS];
#define ASSERT(p)

static void morecore(int bucket);

REPLACEMENT void *
malloc(nbytes)
        register unsigned nbytes;
{
        register union overhead *p;
        register int bucket = 0;
        register unsigned shiftr;

        /*
         * Convert amount of memory requested into
         * closest block size stored in hash buckets
         * which satisfies request.  Account for
         * space used per block for accounting.
         */
        nbytes += sizeof (union overhead) + RSLOP;
        nbytes = (nbytes + 3) &~ 3; 
        shiftr = (nbytes - 1) >> 2;
        /* apart from this loop, this is O(1) */
        while (shiftr >>= 1)
                bucket++;
        /*
         * If nothing in hash bucket right now,
         * request more memory from the system.
         */
        if (nextf[bucket] == NULL)    
                morecore(bucket);
        if ((p = (union overhead *)nextf[bucket]) == NULL)
                return (NULL);
        /* remove from linked list */
#ifndef NO_STUBS
        if (*((int*)p) != 0 &&
	    (*((unsigned*)p) < DATA_START ||
	     *((unsigned*)p) >= (unsigned)DATA_START+DATA_SIZE))
            printf("Corrupt malloc ptr 0x%x at %p\n",*((int*)p),p);
#else
        /*if (*((int*)p) != 0 &&
	    (*((int*)p) < 0x08000000 || *((int*)p) >= 0x09000000))
            printf("Corrupt malloc ptr 0x%x at %p\n",*((int*)p),p); */
#endif
        nextf[bucket] = nextf[bucket]->ov_next;
        p->ov_magic = MAGIC;
        p->ov_index= bucket;
	/*printf("Malloc returning %x bytes at %p\n", nbytes, (char *)(p + 1));*/
        return ((char *)(p + 1));
}

/*
 * Allocate more memory to the indicated bucket.
 */
static void
morecore(bucket)
        register int bucket;
{
        register union overhead *op;
        register int rnu;       /* 2^rnu bytes will be requested */
        register int nblks;     /* become nblks blocks of the desired size */
        register int siz;

        if (nextf[bucket])
                return;
        /*
         * Insure memory is allocated
         * on a page boundary.  Should
         * make getpageize call?
         */
        op = (union overhead *)sbrk(0);
        if ((int)op & 0x3ff)
                sbrk(1024 - ((int)op & 0x3ff));
        /* take 2k unless the block is bigger than that */
        rnu = (bucket <= 8) ? 11 : bucket + 3;
        nblks = 1 << (rnu - (bucket + 3));  /* how many blocks to get */
        if (rnu < bucket)
                rnu = bucket;
        op = (union overhead *)sbrk(1 << rnu);
        /* no more room! */
        if ((int)op == -1)
                return;
        /*
         * Round up to minimum allocation size boundary
         * and deduct from block count to reflect.
         */
        if ((int)op & 7) {
                op = (union overhead *)(((int)op + 8) &~ 7);
                nblks--;
        }
	/*printf("%p is %d blocks of size 2**%d\n", op, nblks, bucket + 3);*/
        /*
         * Add new memory allocated to that on
         * free list for this hash bucket.
         */
        nextf[bucket] = op;
        siz = 1 << (bucket + 3);
        while (--nblks > 0) {
                op->ov_next = (union overhead *)((void *)op + siz);
                op = (union overhead *)((void *)op + siz);
        }
}

REPLACEMENT void free(vp)
        void *vp;
{   
        register int size;
        register union overhead *op;
	char *cp = vp;

        if (cp == NULL)
                return;
        op = (union overhead *)((void *)cp - sizeof (union overhead));
        ASSERT(op->ov_magic == MAGIC);          /* make sure it was in use */
        if (op->ov_magic != MAGIC)
                return;                         /* sanity */
        ASSERT(op->ov_index < NBUCKETS);
        size = op->ov_index;
        op->ov_next = nextf[size];
        nextf[size] = op;
}

REPLACEMENT void *
realloc(vp, nbytes)
        void *vp; 
        unsigned nbytes;
{   
        char *cp = vp;
        register u_int onb;
        union overhead *op;
        char *res;
        register int i;
        int was_alloced = 0;

        if (cp == NULL)
                return (malloc(nbytes));
        op = (union overhead *)((void *)cp - sizeof (union overhead));
	ASSERT(op->ov_magic == MAGIC);
	was_alloced++;
	i = op->ov_index;
        onb = (1 << (i + 3)) - sizeof (*op) - RSLOP;
        /* avoid the copy if same size block */
        if (was_alloced &&
            nbytes <= onb && nbytes > (onb >> 1) - sizeof(*op) - RSLOP)
                return(cp);
        if ((res = malloc(nbytes)) == NULL)
                return (NULL);
        if (cp != res)                  /* common optimization */
	    /*bcopy(cp, res, (nbytes < onb) ? nbytes : onb);*/
	    memcpy(res, cp, (nbytes < onb) ? nbytes : onb);
        if (was_alloced)
                free(cp);
        return (res);
}
#else

void free(void *ptr);
void *realloc(void *vp, unsigned nbytes);

#endif

/* calloc, vmalloc by SMcC */
REPLACEMENT void *calloc(size_t nmemb, size_t size) {
    int len = nmemb * size;
    void *buf = malloc(len);
    if (!buf)
	return 0;
    memset(buf, 0, len);
    return buf;
}

REPLACEMENT void *vmalloc(size_t size) {
    void *ptr = malloc(size);
    if (((int)ptr & 8191) == 0) {
	return ptr;
    } else {
	free(ptr);
	return (void *)(((int)malloc(size + 8192)+8191) & (~8191));
    }
}

/* ------- Code below this line came from dietlibc, under the GPL -------- */

/* ------- code below here has been significantly edited  -------- */

unsigned int random_seed;

REPLACEMENT void srand(unsigned int seed) {
    random_seed = seed;
}

/* Knuth's TAOCP section 3.6 */
// FIXME: ISO C/SuS want a longer period
REPLACEMENT int rand(void) {
    int x = random_seed;
    x = 48271 * (x % 44488) - 3399 * (int)(x / 44488);
    if (x < 0)
	x += 48271;
    random_seed = x;
    return x;
}

static void iswap(void *a, void *b, size_t size) {
    char *x = a;
    char *y = b;
    char *z = x + size;
    while (x < z) {
	char tmp = *x;
	*x = *y;
	*y = tmp;
	++x; ++y;
    }
}

static void swap(void *base, size_t size, size_t a, size_t b) {
  iswap((char*)base + a*size, (char*)base + b*size, size);
}

void isort(void *base, size_t nmemb, size_t size,
	   int (*compar)(const void *, const void *)) {
    size_t i;
    while (nmemb > 1) {
	char *min = base;
	char *tmp = min + size;
	for (i = 1; i < nmemb; i++) {
	    if (compar(tmp, min) < 0)
		min = tmp;
	    tmp += size;
	}
	iswap(min, base, size);
	base += size;
	nmemb--;
    }
}

static char *idx(void* base, size_t size, size_t x) {
    return ((char*)base) + x*size;
}

static void Qsort(void *base, size_t nmemb, size_t size, long l, long r,
                  int (*compar)(const void *, const void *)) {
    long i,j,k,p,q;
    char* v;
    if (r - l <10) {
	isort(idx(base,size,l), r - l + 1, size, compar);
	return;
    }
    v = idx(base, size, r);
    i = l - 1;
    j = r;
    p = l - 1;
    q = r;
    for (;;) {
	while (compar(idx(base, size, ++i), v) < 0)
	    ;
	while (compar(idx(base, size, --j), v) > 0)
	    if (j == l)
		break;
	if (i >= j)
	    break;
	swap(base, size, i, j);
	if (compar(idx(base, size, i), v) == 0) {
	    p++;
	    swap(base, size, p, i);
	}
	if (compar(idx(base, size, j), v) == 0) {
	    q--;
	    swap(base, size, q, j);
	}
    }
    swap(base, size, i, r);
    j = i - 1;
    i++;
    for (k = l; k < p; ++k,--j)
	swap(base, size, k, j);
    for (k = r - 1; k > q; --k, ++i)
	swap(base, size, k, i);
    if (j > l)
	Qsort(base, nmemb, size, l, j, compar);
    if (r > i)
	Qsort(base, nmemb, size, i, r, compar);
}

REPLACEMENT void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *)) {
  Qsort(base, nmemb, size, 0, nmemb - 1, compar);
}

REPLACEMENT size_t strspn(const char *s, const char *set) {
  size_t len = 0;
  int a = 1;
  int i;
  int sl = strlen(set);

  while (a && *s) {
      for (a = i = 0; !a && i < sl; i++)
	  if (*s == set[i])
	      a = 1;
      if (a)
	  len++;
      s++;
  }
  return len;
}

REPLACEMENT size_t strcspn(const char *s, const char *reject) {
  size_t len = 0;
  int a = 1;
  int i;
  int rl = strlen(reject);

  while (a && *s) {
      for (i = 0; a && i < rl; i++)
	  if (*s == reject[i])
	      a=0;
      if (a)
	  len++;
      s++;
  }
  return len;
}

REPLACEMENT char *strpbrk(const char *s, const char *accept) {
    int i;
    int len = strlen(accept);
    for (; *s; s++)
	for (i = 0; i < len; i++)
	    if (*s == accept[i])
		return (char*)s;
    return 0;
}

REPLACEMENT unsigned long int strtoul(const char *ptr, char **endptr,
				      int base) {
    int neg = 0, overflow = 0;
    unsigned long int v = 0;
    const char *orig;
    const char *nptr = ptr;
    
    while (isspace(*nptr))
	++nptr;
    
    if (*nptr == '-') {
	neg = 1;
	nptr++;
    } else if (*nptr == '+')
	++nptr;
    orig = nptr;
    
    if (base==16 && nptr[0]=='0') goto skip0x;
    if (base) {
	unsigned int b = base-2;
	if (b>34) {
	    errno = EINVAL;
	    return 0;
	}
    } else {
	if (*nptr=='0') {
	    base=8;
skip0x:
	    if ((nptr[1] == 'x' || nptr[1] == 'X') && isxdigit(nptr[2])) {
		nptr+=2;
		base=16;
	    }
	} else {
	    base = 10;
	}
    }
    
    while (*nptr) {
	unsigned char c = *nptr;
	c = (c >= 'a' ? c - 'a' + 10 : 
	     c >= 'A' ? c - 'A' + 10 :
	     c <= '9' ? c - '0' :
	     0xff);
	if (c>=base)
	    break;     /* out of base */
	
	{
	    unsigned long x = (v & 0xff)*base + c;
	    unsigned long w = (v >> 8)*base + (x >> 8);
	    if (w > (ULONG_MAX >> 8))
		overflow=1;
	    v = (w << 8) + (x & 0xff);
	}
	++nptr;
    }
    
    if (nptr==orig) {         /* no conversion done */
	nptr = ptr;
	errno = EINVAL;
	v = 0;
    }
    if (endptr)
	*endptr=(char *)nptr;
    if (overflow) {
	errno = ERANGE;
	return ULONG_MAX;
    }
    return (neg ? -v : v);
}


REPLACEMENT long int strtol(const char *nptr, char **endptr, int base) {
    int neg=0;
    unsigned long int v;
    const char *orig = nptr;

    while (isspace(*nptr))
	nptr++;
    if (*nptr == '-' && isalnum(nptr[1])) {
	neg = -1;
	++nptr;
    }

    v = strtoul(nptr, endptr, base);

    if (endptr && *endptr == nptr)
	*endptr=(char *)orig;

    if (v >= 2147483648UL) {
	if (v == 2147483648UL && neg) {
	    errno = 0;
	    return v;
	}
	errno = ERANGE;
	return (neg ? LONG_MIN : LONG_MAX);
    }
    return (neg ? -v : v);
}

REPLACEMENT int atoi(const char *nptr) {
    return strtol(nptr, 0, 10);
}

REPLACEMENT int strcasecmp(const char *s1, const char *s2) {
    unsigned int x1, x2;

    while (1) {
        x2 = *s2 - 'A';
	if (x2 < 26u)
	    x2 += 32;
        x1 = *s1 - 'A';
	if (x1 < 26u)
	    x1 += 32;
        s1++; s2++;
        if (x2 != x1)
            break;
        if (x1 == (unsigned int) - 'A')
            break;
    }

    return x1 - x2;
}

REPLACEMENT void *bsearch(const void *key, const void *base, size_t nmemb,
			  size_t size,
			  int (*compar)(const void*, const void*)) {
    size_t m;
    while (nmemb) {
	int tmp;
	void *p;
	m = nmemb / 2;
	p = (void *) (((const char *) base) + (m * size));
	if ((tmp = (*compar)(key, p)) < 0) {
	    nmemb = m;
	} else if (tmp > 0) {
	    base = p + size;
	    nmemb -= m + 1;
	} else {
	    return p;
	}
    }
    return 0;
}


#define NEED_STAT
#define REPLACEMENT_INLINE
#include "libc.h"
#include "sizes.h"

FILE myfiles[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
FILE *stdin = &myfiles[0];
FILE *stdout = &myfiles[1];
FILE *stderr = &myfiles[2];

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

REPLACEMENT char *strdup(const char *s) {
    char *buf = (char *)malloc(strlen(s) + 1);
    if (buf)
	strcpy(buf, s);
    return buf;
}

int sys_nerr = 34;

void myabort(void) {
    asm("int3");
}

REPLACEMENT int getpagesize() {
    return 8192;
}

REPLACEMENT char *getcwd(char *buf, size_t size) {
    if (size >= 4) {
	strcpy(buf, "/tmp");
	return buf;
    } else {
	errno = ERANGE;
	return 0;
    }
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

REPLACEMENT int chdir(const char *path) {
    int ret = outside_chdir(path);
    refresh_errno();
    return ret;
}

REPLACEMENT int chmod(const char *path, mode_t mode) {
    int ret = outside_chmod(path, mode);
    refresh_errno();
    return ret;
}    

REPLACEMENT char *ctime(const time_t *timep) {
    return "Wed Jun 30 21:49:08 1993\n";
}

REPLACEMENT int dup2(int oldfd, int newfd) {
    int ret = outside_dup2(oldfd, newfd);
    refresh_errno();
    return ret;
}

REPLACEMENT int dup(int oldfd) {
    int ret = outside_dup(oldfd);
    refresh_errno();
    return ret;
}

REPLACEMENT int execl(const char *path, const char *arg0,...) {
    va_list ap;
    const char *argv[256]; /* should really be dynamically allocated */
    int i = 0, ret;
    char *arg;

    argv[i++] = arg0;
    va_start(ap, arg0);
    while ((arg = va_arg(ap, char *))) {
	argv[i++] = arg;
	if (i > 250) {
	    errno = E2BIG;
	    return -1;
	}
    }
    argv[i] = 0;
    va_end(ap);
    
    ret = outside_execv(path, (char *const *)argv);
    refresh_errno();
    return ret;
}

REPLACEMENT int execlp(const char *file, const char *arg, ...) {
    myabort();
    errno = ENOSYS;
    return -1;
}

REPLACEMENT int execle(const char *path, const char *arg, ...) {
    myabort();
    errno = ENOSYS;
    return -1;
}

REPLACEMENT int execv(const char *path, char *const argv[]) { 
    int ret = outside_execv(path, argv);
    refresh_errno();
    return ret;
}

REPLACEMENT int execvp(const char *file, char *const argv[]) {
    int ret = outside_execvp(file, argv);
    refresh_errno();
    return ret;
}

REPLACEMENT int execve(const char *path, char *const argv[],
		       char *const envp[]) {
    myabort();
    errno = ENOSYS;
    return -1;
}

/* As long as we don't have atexit(), these are the same */
REPLACEMENT void _exit(int status) {
    exit(status);
}

void __stack_chk_fail(void) {
    fprintf(stderr, "Stack smashing detected, exiting\n");
    exit(-1);
}

REPLACEMENT int fcntl(int fd, int cmd, ...) {
    errno = ENOSYS;
    return -1;
}

REPLACEMENT pid_t fork(void) {
    int ret = outside_fork();
    refresh_errno();
    return ret;
}

REPLACEMENT uid_t getuid(void)  { return 15168; }
REPLACEMENT uid_t geteuid(void) { return 15168; }
REPLACEMENT gid_t getgid(void)  { return 15168; }
REPLACEMENT gid_t getegid(void) { return 15168; }
REPLACEMENT pid_t getpid(void)  { return 2163;  }

REPLACEMENT int fstat(int fd, struct stat *buf) {
    int ret = outside_fstat(fd, buf);
    refresh_errno();
    return ret;    
}    

REPLACEMENT int ftruncate(int fd, off_t length) {
    int ret = outside_ftruncate(fd, length);
    refresh_errno();
    return ret;
}

REPLACEMENT int ioctl(int fd, int request, ...) {
    errno = ENOSYS;
    return -1;
}

REPLACEMENT int isatty(int fd) {
    int ret = outside_isatty(fd);
    refresh_errno();
    return ret;
}

REPLACEMENT int kill(pid_t pid, int sig) {
    errno = EPERM;
    return -1;
}

REPLACEMENT off_t lseek(int fd, off_t offset, int whence) {
    int ret = outside_lseek(fd, offset, whence);
    refresh_errno();
    return ret;
}

REPLACEMENT struct tm *localtime(const time_t *timep) {
    return gmtime(timep);
}

REPLACEMENT int mkdir(const char *pathname, mode_t mode) {
    int ret = outside_mkdir(pathname, mode);
    refresh_errno();
    return ret;
}

REPLACEMENT int open(const char *pathname, int flags, ...) {
    va_list vl;
    va_start(vl, flags);
    
    int ret = outside_open(pathname, flags, va_arg(vl, int));
    refresh_errno();
    va_end(vl);
    return ret;    
}

REPLACEMENT int pipe(int fds[2]) {
    int ret = outside_pipe(fds);
    refresh_errno();
    return ret;
}

REPLACEMENT int read(int fd, void *buf, size_t count) {
    int ret = outside_read(fd, buf, count);
    refresh_errno();
    return ret;    
}

REPLACEMENT int rename(const char *oldpath, const char *newpath) {
    int ret = outside_rename(oldpath, newpath);
    refresh_errno();
    return ret;
}

REPLACEMENT int rmdir(const char *path) {
    int ret = outside_rmdir(path);
    refresh_errno();
    return ret;
}

/* 186.crafty uses this, believe it or not. */
REPLACEMENT int select(int n, fd_set *rfds, fd_set *wfds, fd_set *xfds, 
		       struct timeval *tv) {
    int ret = outside_select(n, rfds, wfds, xfds, tv);
    refresh_errno();
    return ret;
}

REPLACEMENT int setuid(uid_t uid) {
    errno = EPERM;
    return -1;
}

REPLACEMENT int setgid(gid_t gid) {
    errno = EPERM;
    return -1;
}

REPLACEMENT int mystat(const char *file_name, struct stat *buf) {
    int ret = outside_stat(file_name, buf);
    refresh_errno();
    return ret;    
}    

#ifndef NO_STUBS
REPLACEMENT int stat(const char *file_name, struct stat *buf) {
    int ret = outside_stat(file_name, buf);
    refresh_errno();
    return ret;    
}
#endif

/* We don't actually deliver any signals */
REPLACEMENT sighandler_t signal(int signum, sighandler_t handler) {
    return 0;
}

REPLACEMENT long sysconf(int name) {
    if (name == _SC_CLK_TCK) {
	return 1000;
    } else {
	return -1;
    }
}

/* Not implemented, for obvious reasons. But SPECint's gcc tries to
   call it. */
REPLACEMENT int system(const char *string) {
    return -1;
}

REPLACEMENT int lstat(const char *file_name, struct stat *buf) {
    int ret = outside_lstat(file_name, buf);
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

REPLACEMENT int truncate(const char *path, off_t length) {
    int ret = outside_truncate(path, length);
    refresh_errno();
    return ret;
}

REPLACEMENT char *ttyname(int fd) {
    if (isatty(fd)) {
	return "/dev/tty";
    } else {
	errno = ENOTTY;
	return 0;
    }
}

REPLACEMENT int unlink(const char *path) {
    int ret = outside_unlink(path);
    refresh_errno();
    return ret;
}	

REPLACEMENT int utime(const char *filename, const struct utimbuf *buf) {
    return 0;
}

REPLACEMENT pid_t wait(int *status) {
    int ret = outside_wait(status);
    refresh_errno();
    return ret;
}

REPLACEMENT ssize_t write(int fd, const void *buf, size_t count) {
    int ret = outside_write(fd, buf, count);
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
	return c - 'a' + 'A';
    else
	return c;
}

REPLACEMENT int tolower(int c) {
    if (isupper(c))
	return c - 'A' + 'a';
    else
	return c;
}

/* math */

REPLACEMENT int abs(int x) {
    return x > 0 ? x : -x;
}

REPLACEMENT int labs(long x) {
    return x > 0 ? x : -x;
}

/* stdio */

REPLACEMENT int atol(const char *nptr) {
    return atoi(nptr);
}

REPLACEMENT void clearerr(FILE *stream) {
    outside_clearerr(*stream);
}

REPLACEMENT FILE *fopen(const char *path, const char *mode) {
    int fi = outside_fopen(path, mode);
    if (fi == -1) {
	refresh_errno();
	return 0;
    }
    return &myfiles[fi];
}

REPLACEMENT FILE *fdopen(int fd, const char *mode) {
    int fi = outside_fdopen(fd, mode);
    if (fi == -1) {
	refresh_errno();
	return 0;
    }
    return &myfiles[fi];
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

REPLACEMENT char *fgets(char *buf, int size, FILE *stream) {
    char *ret = outside_fgets(buf, size, *stream);
    if (!ret) {
	refresh_errno();
    }
    return ret;
}

REPLACEMENT char *gets(char *buf) {
    for (;;) {
	int c = getchar();
	if (c == EOF || c == '\n') {
	    *buf = '\0';
	    return buf;
	}
	*buf++ = c;
    }
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

REPLACEMENT int feof(FILE *stream) {
    return outside_feof(*stream);
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

REPLACEMENT int fscanf(FILE *stream, const char *fmt, ...) {
    va_list args;
    int ret;
    va_start(args, fmt);
    ret = outside_vfscanf(*stream, fmt, args);
    va_end(args);
    return ret;
}

REPLACEMENT int scanf(const char *fmt, ...) {
    va_list args;
    int ret;
    va_start(args, fmt);
    ret = outside_vfscanf(0, fmt, args);
    va_end(args);
    return ret;
}

REPLACEMENT size_t fread(void *ptr, size_t size, size_t num, FILE *stream) {
    int ret = outside_fread(ptr, size, num, *stream);
    return ret;
}

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

REPLACEMENT int remove(const char *path) {
    return unlink(path);
}

REPLACEMENT void setbuf(FILE *stream, char *buf) {
    outside_setbuf(*stream, buf);
}

REPLACEMENT FILE *tmpfile(void) {
    static int count = 0;
    char filename[14];
    strcpy(filename, "/tmp/temp0000");
    filename[12] =  count         % 10;
    filename[11] = (count / 10)   % 10;
    filename[10] = (count / 100)  % 10;
    filename [9] = (count / 1000) % 10;
    count++;
    return fopen(filename, "w+b");
}

REPLACEMENT char *tmpnam(char *s) {
    if (s) {
	strcpy(s, "/tmp/foo");
	return s;
    } else {
	return "/tmp/bar";
    }
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

#ifndef NO_STUBS
static char *empty_environment[1] = {0};
char **environ = empty_environment;

/* s/b unique per shared library. Since we don't have them, that's not
   a problem. */
void* __dso_handle = (void*)0;

/* Let's see if we can get away without running any global destructors */
int __cxa_atexit(void (*func) (void *), void *arg, void *d) {
    return 0;
}

/* Overriden by global_constructors() in libcplusplus in C++ */
void global_constructors(void) __attribute__ ((weak));

void global_constructors(void) {}

int main(int argc, char **argv);
int _start(int argc, char **argv) {
    global_constructors();
    return main(argc, argv);
}
#endif /* !NO_STUBS */

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
malloc(unsigned nbytes)
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
#ifdef DATA_START
        if (*((int*)p) != 0 &&
	    (*((unsigned*)p) < DATA_START ||
	     *((unsigned*)p) >= (unsigned)DATA_START+DATA_SIZE))
            printf("Corrupt malloc ptr 0x%x at %p\n",*((int*)p),p);
#endif
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
morecore(int bucket)
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
                op->ov_next = (union overhead *)((char *)op + siz);
                op = (union overhead *)((char *)op + siz);
        }
}

REPLACEMENT void free(void *vp)
{   
        register int size;
        register union overhead *op;
	char *cp = (char *)vp;

        if (cp == NULL)
                return;
        op = (union overhead *)(cp - sizeof (union overhead));
        ASSERT(op->ov_magic == MAGIC);          /* make sure it was in use */
        if (op->ov_magic != MAGIC)
                return;                         /* sanity */
        ASSERT(op->ov_index < NBUCKETS);
        size = op->ov_index;
        op->ov_next = nextf[size];
        nextf[size] = op;
}

REPLACEMENT void *
realloc(void *vp, unsigned nbytes)
{   
        char *cp = (char *)vp;
        register u_int onb;
        union overhead *op;
        char *res;
        register int i;
        int was_alloced = 0;

        if (cp == NULL)
                return (malloc(nbytes));
        op = (union overhead *)(cp - sizeof (union overhead));
	ASSERT(op->ov_magic == MAGIC);
	was_alloced++;
	i = op->ov_index;
        onb = (1 << (i + 3)) - sizeof (*op) - RSLOP;
        /* avoid the copy if same size block */
        if (was_alloced &&
            nbytes <= onb && nbytes > (onb >> 1) - sizeof(*op) - RSLOP)
                return(cp);
        if ((res = (char *)malloc(nbytes)) == NULL)
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
    char *x = (char *)a;
    char *y = (char *)b;
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
	char *min = (char *)base;
	char *tmp = min + size;
	for (i = 1; i < nmemb; i++) {
	    if (compar(tmp, min) < 0)
		min = tmp;
	    tmp += size;
	}
	iswap(min, base, size);
	base = (char *)base + size;
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

static char* strtok_r(char *s, const char *delim, char **ptrptr) {
  char *tmp = 0;

  if (!s)
    s = *ptrptr;
  s += strspn(s, delim);           /* overread leading delimiter */
  if (*s) {
    tmp = s;
    s += strcspn(s, delim);
    if (*s) *s++ = 0;   /* not the end ? => terminate it */
  }
  *ptrptr = s;
  return tmp;
}

static char *strtok_pos;

REPLACEMENT char *strtok(char *s, const char *delim)
{
  return strtok_r(s, delim, &strtok_pos);
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
	    base = (char *)p + size;
	    nmemb -= m + 1;
	} else {
	    return p;
	}
    }
    return 0;
}

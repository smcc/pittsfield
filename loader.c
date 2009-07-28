#undef _GNU_SOURCE
#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <asm/ldt.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <libelf/libelf.h>
#include <assert.h>

#include "sizes.h"

#ifdef VERIFY
#include "libdis.h"
#endif

#define CODE_END CODE_START + CODE_SIZE
#define DATA_END DATA_START + DATA_SIZE


int outside_esp;
int inside_esp;

int outside_ebp;
int inside_ebp;

/* It seems better from an isolation standpoint to use a separate
   %gs for stack checks inside the sandbox, but that would require
   switching the outside one back before calling any libc functions,
   since it also expects to access thread information in that segment.
   So for the moment we share the %gs segment, but check that only
   offset 0x14 is accessed. */
/* short outside_gs;
   short inside_gs; */

int outside_ebx;
int outside_esi;
int outside_edi;

jmp_buf exit_buf;
int exit_value;

int call_in(void *addr, int argc, char **argv) {
    int ret;
    if (!setjmp(exit_buf)) {
	/* XXX This is brittle wrt. GCC's code generation strategy. */
	asm("movl  %0, %%ecx" :: "r" (argc));
	asm("movl  %0, %%edx" :: "r" (argv));
	asm("movl  %0, %%eax" :: "r" (addr));
	asm("movl  %esp, outside_esp");
	asm("movl  %ebp, outside_ebp");
	asm("movl  %ebx, outside_ebx");
	asm("movl  %esi, outside_esi");
	asm("movl  %edi, outside_edi");
	/* asm("movw  %gs, outside_gs"); */
	asm("movl  inside_esp, %esp");
	asm("movl  inside_ebp, %ebp");
	/* asm("movw  inside_gs, %gs"); */
	asm("call  *%eax");
	asm("movl  %esp, inside_esp");
	asm("movl  %ebp, inside_ebp");
	/* asm("movw  %gs, inside_gs"); */
	asm("movl  outside_esp, %esp");
	asm("movl  outside_ebp, %ebp");
	asm("movl  outside_ebx, %ebx");
	asm("movl  outside_esi, %esi");
	asm("movl  outside_edi, %edi");
	/* asm("movw  outside_gs, %gs"); */
	asm("movl  %%eax, %0" : "=r" (ret));
	return ret;
    }
    return exit_value;
}

/* Is this declared anywhere? */
extern int modify_ldt(int func, void *ptr, unsigned long bytecount);

/* Set up a segment descriptor for a one-page segment starting at the
   given linear address, and return its selector. */
short setup_gs_ldt(unsigned addr) {
    struct user_desc desc;
    short selector = 16 + 4 + 3; /* 4=LDT, 3=RPL */
    memset(&desc, 0, sizeof(desc));
    desc.seg_32bit = 1;
    desc.read_exec_only = 0;
    desc.limit_in_pages = 1;
    desc.seg_not_present = 0;
    desc.useable = 1;
    desc.entry_number = selector / 8;
    desc.base_addr = addr;
    desc.limit = 1;
    desc.contents = MODIFY_LDT_CONTENTS_DATA;
    if (modify_ldt(1, &desc, sizeof(desc)) < 0) {
	fprintf(stderr, "Segment descriptor setup failed\n");
	exit(-1);
    }
    return selector;
}

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long long
__udivdi3(unsigned long long a, unsigned long long b);
extern long long
__divdi3(long long a, long long b);
extern unsigned long long
__umoddi3(unsigned long long a, unsigned long long b);
extern long long
__moddi3(long long a, long long b);
extern double
__floatdidf(long long a);
extern long long
__fixdfdi(double a);
#ifdef __cplusplus
} /* extern "C" */
#endif

unsigned long long wrap___udivdi3(unsigned long long a, unsigned long long b) {
    return __udivdi3(a, b);
}

long long wrap___divdi3(long long a, long long b) {
    return __divdi3(a, b);
}

unsigned long long wrap___umoddi3(unsigned long long a, unsigned long long b) {
    return __umoddi3(a, b);
}

long long wrap___moddi3(long long a, long long b) {
    return __moddi3(a, b);
}

double wrap___floatdidf(long long a) {
    return __floatdidf(a);
}

long long wrap___fixdfdi(double a) {
    return __fixdfdi(a);
}

int wrap_printf(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    return vprintf(fmt, ap);
}

int wrap_print_int(int i) {
    return printf("%d\n", i);
}

long long wrap_atoll(const char *nptr) {
    return atoll(nptr);
}

int wrap_putchar(int c) {
    return putchar(c);
}

void *data_break;

void *wrap_sbrk(long incr) {
    void *new_brk = (void *)((long)data_break + incr);
    if ((unsigned)new_brk > (unsigned)DATA_START &&
	(unsigned)new_brk < (unsigned)DATA_END) {
	void *old_brk = data_break;
	/* printf("Moving break by 0x%lx to %p\n", incr, new_brk); */
	data_break = new_brk;
	return old_brk;
    } else {
	/* printf("Rejecting sbrk by 0x%lx to %p\n", incr, new_brk); */
	return (void *)-1;
    }
}

char *wrap_outside_strerror(int errnum) {
    return strerror(errnum);
}

void wrap_exit(int val) {
    exit_value = val;
    longjmp(exit_buf, 1);
}

FILE *files[16] = {/*stdin, stdout, stderr,*/ 0, 0, 0,
		   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int wrap_outside_fopen(const char *path, const char *mode) {
    int i;
    for (i = 0; i < 16; i++)
	if (!files[i])
	    break;
    if (i == 16)
	abort();
    files[i] = fopen(path, mode);
    if (!files[i])
	return -1;
    return i;
}

int wrap_outside_fdopen(int fd, const char *mode) {
    int i;
    for (i = 0; i < 16; i++)
	if (!files[i])
	    break;
    if (i == 16)
	abort();
    files[i] = fdopen(fd, mode);
    if (!files[i])
	return -1;
    return i;
}

int wrap_outside_fclose(int fi) {
    int ret = fclose(files[fi]);
    files[fi] = 0;
    return ret;
}

int wrap_outside_errno(void) {
    return errno;
}

int wrap_outside_ferror(int fi) {
    return ferror(files[fi]);
}

int wrap_outside_fflush(int fi) {
    return fflush(files[fi]);
}

int wrap_outside_fileno(int fi) {
    return fileno(files[fi]);
}

int wrap_outside_feof(int fi) {
    return feof(files[fi]);
}

int wrap_outside_fgetc(int fi) {
    return fgetc(files[fi]);
}

int wrap_outside_vfprintf(int fi, const char *fmt, va_list ap) {
    return vfprintf(files[fi], fmt, ap);
}

int wrap_outside_vfscanf(int fi, const char *fmt, va_list ap) {
    return vfscanf(files[fi], fmt, ap);
}

size_t wrap_outside_fread(void *ptr, size_t size, size_t num, int fi) {
    return fread(ptr, size, num, files[fi]);
}

size_t wrap_outside_fwrite(const void *ptr, size_t size, size_t num, int fi) {
    return fwrite(ptr, size, num, files[fi]);
}

void wrap_outside_rewind(int fi) {
    rewind(files[fi]);
}

int wrap_outside_ungetc(int c, int fi) {
    return ungetc(c, files[fi]);
}

int wrap_gettimeofday(struct timeval *tv, struct timezone *tz) {
    return gettimeofday(tv, tz);
}

int wrap_sscanf(const char *str, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    return vsscanf(str, fmt, ap);
}

double wrap_atof(const char *nptr) {
    return atof(nptr);
}

int wrap_outside_fseek(int fi, long offset, int whence) {
    return fseek(files[fi], offset, whence);
}

double wrap_ldexp(double x, int exp) {
    return ldexp(x, exp);
}

long wrap_outside_ftell(int fi) {
    return ftell(files[fi]);
}

double wrap_floor(double x) {
    return floor(x);
}

float wrap_floorf(float x) {
    return floorf(x);
}

double wrap_sin(double x) {
    return sin(x);
}

double wrap_sqrt(double x) {
    return sqrt(x);
}

float wrap_sqrtf(float x) {
    return sqrtf(x);
}

void wrap_abort(void) {
    abort();
}

double wrap_rint(double x) {
    return rint(x);
}

long wrap_lrint(double x) {
    return lrint(x);
}

long wrap_lrintf(float x) {
    return lrintf(x);
}

double wrap_acos(double x) {
    return acos(x);
}

double wrap_asin(double x) {
    return asin(x);
}

double wrap_atan(double x) {
    return atan(x);
}

double wrap_cos(double x) {
    return cos(x);
}

double wrap_exp(double x) {
    return exp(x);
}

float wrap_expf(float x) {
    return expf(x);
}

double wrap_fabs(double x) {
    return fabs(x);
}

double wrap_log(double x) {
    return log(x);
}

double wrap_log10(double x) {
    return log10(x);
}

double wrap_ceil(double x) {
    return ceil(x);
}

float wrap_ceilf(float x) {
    return ceilf(x);
}

double wrap_pow(double x, double y) {
    return pow(x, y);
}

double wrap_frexp(double x, int *exp) {
    return frexp(x, exp);
}

int wrap_sprintf(char *str, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    return vsprintf(str, fmt, ap);
}

char *wrap_outside_getenv(const char *name) {
    return getenv(name);
}

int wrap_outside_access(const char *pathname, int mode) {
    return access(pathname, mode);
}

int wrap_getrusage(int who, struct rusage *usage) {
    return getrusage(who, usage);
}

int wrap_outside_close(int fd) {
    return close(fd);
}

char *wrap_outside_fgets(char *buf, int size, int fi) {
    return fgets(buf, size, files[fi]);
}

int wrap_outside_fputc(int c, int fi) {
    return fputc(c, files[fi]);
}

int wrap_outside_fputs(const char *str, int fi) {
    return fputs(str, files[fi]);
}

int wrap_outside_open(const char *pathname, int flags, int mode) {
    return open(pathname, flags, mode);
}

off_t wrap_outside_lseek(int fd, off_t offset, int whence) {
    return lseek(fd, offset, whence);
}

int wrap_outside_read(int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}

DIR *wrap_outside_opendir(const char *name) {
    return opendir(name);
}

int wrap_outside_closedir(DIR *dir) {
    return closedir(dir);
}

struct dirent *wrap_outside_readdir(DIR *dir) {
    return readdir(dir);
}

struct inside_stat {
    off_t inside_st_size;
    int inside_st_mode;
    time_t inside_st_mtime;
    nlink_t inside_st_nlink;
    ino_t inside_st_ino;
    dev_t inside_st_dev;
    uid_t inside_st_uid;
    gid_t inside_st_gid;
    dev_t inside_st_rdev;
    time_t inside_st_atime;
    time_t inside_st_ctime;
};

int wrap_outside_stat(const char *fname, struct inside_stat *in_buf) {
    struct stat buf;
    int ret = stat(fname, &buf);
    if (!ret) {
	in_buf->inside_st_size = buf.st_size;
	in_buf->inside_st_mode = buf.st_mode;
	in_buf->inside_st_mtime = buf.st_mtime;
	in_buf->inside_st_nlink = buf.st_nlink;
	in_buf->inside_st_ino = buf.st_ino;
	in_buf->inside_st_dev = buf.st_dev;
	in_buf->inside_st_uid = buf.st_uid;
	in_buf->inside_st_gid = buf.st_gid;
	in_buf->inside_st_rdev = buf.st_rdev;
	in_buf->inside_st_atime = buf.st_atime;
	in_buf->inside_st_ctime = buf.st_ctime;
    }
    return ret;
}

int wrap_outside_lstat(const char *fname, struct inside_stat *in_buf) {
    struct stat buf;
    int ret = lstat(fname, &buf);
    if (!ret) {
	in_buf->inside_st_size = buf.st_size;
	in_buf->inside_st_mode = buf.st_mode;
	in_buf->inside_st_mtime = buf.st_mtime;
	in_buf->inside_st_nlink = buf.st_nlink;
	in_buf->inside_st_ino = buf.st_ino;
	in_buf->inside_st_dev = buf.st_dev;
	in_buf->inside_st_uid = buf.st_uid;
	in_buf->inside_st_gid = buf.st_gid;
	in_buf->inside_st_rdev = buf.st_rdev;
	in_buf->inside_st_atime = buf.st_atime;
	in_buf->inside_st_ctime = buf.st_ctime;
    }
    return ret;
}

int wrap_outside_fstat(int fd, struct inside_stat *in_buf) {
    struct stat buf;
    int ret = fstat(fd, &buf);
    if (!ret) {
	in_buf->inside_st_size = buf.st_size;
	in_buf->inside_st_mode = buf.st_mode;
	in_buf->inside_st_mtime = buf.st_mtime;
	in_buf->inside_st_nlink = buf.st_nlink;
	in_buf->inside_st_ino = buf.st_ino;
	in_buf->inside_st_dev = buf.st_dev;
	in_buf->inside_st_uid = buf.st_uid;
	in_buf->inside_st_gid = buf.st_gid;
	in_buf->inside_st_rdev = buf.st_rdev;
	in_buf->inside_st_atime = buf.st_atime;
	in_buf->inside_st_ctime = buf.st_ctime;
    }
    return ret;
}

void wrap_outside_free(void *buf) {
    free(buf);
}

int wrap_outside_vasprintf(char **strp, const char *fmt, va_list ap) {
    return vasprintf(strp, fmt, ap);
}

int wrap_outside_vsnprintf(char *str, size_t size, const char *format,
			   va_list ap) {
    return vsnprintf(str, size, format, ap);
}

clock_t wrap_clock(void) {
    return clock();
}

clock_t wrap_outside_times(struct tms *buf) {
    return times(buf);
}

int wrap_outside_unlink(const char *path) {
    return unlink(path);
}

int wrap_outside_chmod(const char *path, mode_t mode) {
    return chmod(path, mode);
}

int wrap_outside_isatty(int fd) {
    return isatty(fd);
}

int wrap_outside_select(int n, fd_set *rfds, fd_set *wfds, fd_set *xfds, 
			struct timeval *tv) {
    return select(n, rfds, wfds, xfds, tv);
}

int wrap_outside_dup(int oldfd) {
    return dup(oldfd);
}

int wrap_outside_dup2(int oldfd, int newfd) {
    return dup2(oldfd, newfd);
}

void wrap_outside_clearerr(int fi) {
    clearerr(files[fi]);
}

int wrap_outside_rename(const char *oldpath, const char *newpath) {
    return rename(oldpath, newpath);
}

int wrap_outside_chdir(const char *path) {
    return chdir(path);
}

int wrap_vsprintf(char *str, const char *format, va_list ap) {
    return vsprintf(str, format, ap);
}

double wrap_atan2(double y, double x) {
    return atan2(y, x);
}

double wrap_modf(double x, double *iptr) {
    return modf(x, iptr);
}

unsigned int wrap_sleep(unsigned int secs) {
    return sleep(secs);
}

pid_t wrap_outside_wait(int *status) {
    return wait(status);
}

mode_t wrap_umask(mode_t mask) {
    return umask(mask);
}

int wrap_outside_write(int fd, const void *buf, size_t count) {
    return write(fd, buf, count);
}

int wrap_outside_truncate(const char *path, off_t length) {
    return truncate(path, length);
}

int wrap_outside_ftruncate(int fd, off_t length) {
    return ftruncate(fd, length);
}

int wrap_outside_mkdir(const char *pathname, mode_t mode) {
    return mkdir(pathname, mode);
}

int wrap_outside_rmdir(const char *path) {
    return rmdir(path);
}

void wrap_outside_setbuf(int fi, char *buf) {
    setbuf(files[fi], buf);
}

double wrap_tan(double theta) {
    return tan(theta);
}

pid_t wrap_outside_fork(void) {
    return fork();
}

int wrap_outside_execvp(const char *file, char *const argv[]) {
    return execvp(file, argv);
}

int wrap_outside_execv(const char *file, char *const argv[]) {
    return execvp(file, argv);
}

int wrap_outside_pipe(int fds[2]) {
    return pipe(fds);
}

struct tm *wrap_gmtime(const time_t *timep) {
    return gmtime(timep);
}



void wrap_fail_check(void) {
    printf("Program has performed an illegal operation "
	   "and will be terminated\n");
    exit(1);
}


static int errno_to_vx32(int host_errno) {
    switch (host_errno) {
    case EINVAL: return 1;
    case ENOMEM: return 2;
    case EDOM:   return 3;
    case ERANGE: return	4;
    case EBADF:	 return 5;
    case EPROTO: return 6;
    case EIO:    return 7;
    case ENOENT: return 8;
    case EPIPE:	 return 9;
    case EAGAIN: return 10;

    default: return 1;
    }
}

void wrap_vx32__exit(int status) {
    exit_value = status;
    longjmp(exit_buf, 1);
}

int wrap_vx32_read(int fd, void *buf, size_t count) {
    int rc = read(fd, buf, count);
    if (rc == -1) {
	return -errno_to_vx32(errno);
    } else {
	return rc;
    }
}

void *wrap_vx32_sbrk(long inc) {
    return wrap_sbrk(inc);
}

int wrap_vx32_write(int fd, const void *buf, size_t count) {
    int rc = write(fd, buf, count);
    if (rc == -1) {
	return -errno_to_vx32(errno);
    } else {
	return rc;
    }
}



void init_wrappers() {
    files[0] = stdin;
    files[1] = stdout;
    files[2] = stderr;
}

int (*stubs[])() = {
#include "wrappers.h"
    0};

void install_stubs() {
    int addr = CODE_START + 2*CHUNK_SIZE;
    int (**p)();
    for (p = stubs; *p; p++) {
	/* jmp is PC relative; specificially, relative to the PC after
	   the end of the jump instruction. */
	*((unsigned char *)addr) = 0xe9;
	int offset = (int)*p - (addr + 5);
	*((int *)(addr + 1)) = offset;
	addr += STUB_SIZE;
    }
}

#ifdef VERIFY
void fail_verify(const char *msg, int offset) {
    printf("Verification failed: %s at offset 0x%08x\n", msg, offset);
    exit(1);
}

void fail_verify_int(const char *msg, int arg, int offset) {
    printf("Verification failed: %s (0x%x) at offset 0x%08x\n",
	   msg, arg, offset);
    exit(1);
}

void fail_verify_insn(const char *msg, x86_insn_t *insn, int arg, int offset) 
{
    char buf[100];
    x86_format_insn(insn, buf, sizeof(buf), att_syntax);
    printf("Verification failed: %s (0x%x) at offset 0x%08x\n",
	   msg, arg, offset);
    printf("Bad instruction %s\n", buf);
    exit(1);
}

#define REGISTER_EAX 1
#define REGISTER_ECX 2
#define REGISTER_EDX 3
#define REGISTER_EBX 4
#define REGISTER_ESP 5
#define REGISTER_EBP 6
#define REGISTER_ESI 7
#define REGISTER_EDI 8

enum eff_addr_type {
    EA_EBX_EXACT,  /* (%ebx) */
    EA_ESP_EXACT,  /* (%esp), sometimes disassembled as (%esp,1) to
		     match encoding */
    EA_EBP_OFFSET, /* x(%ebp), where |x| < 2**16 */
    EA_ESP_OFFSET, /* x(%esp), where |x| < 2**8 */
    EA_DIRECT,     /* 0x...., but in ModRM encoding */
    EA_OTHER,       /* anything else */
};

enum eff_addr_type classify_ea(x86_ea_t ea) {
    if (ea.base.id == REGISTER_EBX &&
	ea.index.id == 0 && ea.disp_size == 0)
	return EA_EBX_EXACT;
    else if (ea.base.id == REGISTER_ESP &&
	     ea.index.id == 0 && ea.disp_size == 0) 
	return EA_ESP_EXACT;
    else if (ea.base.id == REGISTER_EBP &&
	ea.index.id == 0 &&
	(ea.disp_size <= 2 ||
	 (ea.disp_sign && ea.disp > -32768) ||
	 ((unsigned)ea.disp < 32768)))
	return EA_EBP_OFFSET;
    else if (ea.base.id == REGISTER_ESP &&
	     ea.index.id == 0 &&
	     ea.disp_size <= 1)
	return EA_ESP_OFFSET;
    else if (ea.base.id == 0 && ea.index.id == 0 &&
	     ea.disp_size == 4)
	return EA_DIRECT;
    else
	return EA_OTHER;
}

#define BUMP_ESP            0x1
#define CHANGE_EBP          0x2
#define CHANGE_ESP          0x4
#define EBP_DATA_SAFE       0x8
#define EBX_CODE_SAFE      0x10
#define EBX_DATA_SAFE      0x20
#define IJUMP              0x40
#define IREAD              0x80
#define IWRITE            0x100
#define JUMP              0x200
#define STACK_TOP_SAFE    0x400
#define USE_ESP           0x800
#define USE_EBP          0x1000

#define DATA_SAFETY      EBX_DATA_SAFE
#define CODE_SAFETY      EBX_CODE_SAFE
#define STACK_TOP_SAFETY STACK_TOP_SAFE
#define EBP_SAFETY       EBP_DATA_SAFE
#define ESP_SAFETY       USE_ESP

void verify(int code_len) {
    int offset = 0;
    int next_aligned = offset;
    int safety = 0, unsafety = 0;
    int bump_count = 0;
    x86_init(opt_none, 0, 0);
    while (offset < code_len) {
	x86_insn_t insn;
	int len, i, my_explicit_count;
	int arg_types;
	int flags;
	x86_oplist_t args[10], *arg;
	if (offset == next_aligned) {
	    next_aligned += CHUNK_SIZE;
	} else if (offset > next_aligned) {
	    fail_verify("Bad chunk alignment", offset);
	}
	len = x86_disasm((unsigned char *)CODE_START, code_len,
			 (unsigned long)CODE_START, offset, &insn);
	if (len <= 0) {
	    fail_verify("Illegal instruction", offset);
	}
	switch (insn.group) {
	case insn_controlflow:
	case insn_arithmetic:
	case insn_logic:
	case insn_stack:
	case insn_comparison:
	case insn_move:
	case insn_bit_manip:
	case insn_fpu:
	case insn_interrupt:
	case insn_other:
	    break;
	default:
	    fail_verify_int("Illegal insn group", insn.group, offset);
	}
	switch (insn.type) {
	    /* insn_controlflow */
	case insn_jmp:
	case insn_jcc:
	case insn_call:
	case insn_return:
	    /* insn_arithmetic */
	case insn_add:
	case insn_sub:
	case insn_mul:
	case insn_div:
	case insn_inc:
	case insn_dec:
	case insn_shl:
	case insn_shr:
	case insn_rol:
	case insn_ror:
	    /* insn_logic */
	case insn_and:
	case insn_or:
	case insn_xor:
	case insn_not:
	case insn_neg:
	    /* insn_stack */
	case insn_push:
	case insn_pop:
	case insn_pushflags:
	case insn_popflags:
	case insn_leave:
	    /* insn_comparison */
	case insn_test:
	case insn_cmp:
	    /* insn_move */
	case insn_mov:
	case insn_movcc:
	    /* insn_bit_manip */
	case insn_bittest:
	    /* insn_fpu */
	case 0xa000:  /* XXX all FPU */
	    /* insn_interrupt */
	case insn_debug: /* int3 */
	    /* insn_other */
	case insn_nop:
	case insn_szconv:
	    break;
	default:
	    {
		char buf[100];
		x86_format_mnemonic(&insn, buf, sizeof(buf), att_syntax);
		printf("Unknown instruction %s\n", buf);
		fail_verify_int("Illegal insn type", insn.type, offset);
	    }
	}

	/* We do all of our calculation based on the explicit operands
	   to make the parallel with verify.pl clearer, even though in
	   a clean-slate implementation, it might be better to treat
	   the implicit operands uniformly. */
	assert(insn.explicit_count <= 3);
	i = 0;
	for (arg = insn.operands; arg; arg = arg->next) {
	    if (!(arg->op.flags & op_implied)) {
		args[i] = *arg;
		args[i].next = &args[i+1];
		i++;
	    }
	}
	if (i > 0)
	    args[i-1].next = 0;
	/* The insn.explicit_count field seems to be broken in
	   libdisas 0.23, giving 2 rather than 1 for "imul %ecx". */
	/* assert(i == insn.explicit_count); */
	my_explicit_count = i;

#define UNARY(op1)               ((op1) << 4 | 1)
#define BINARY(op1, op2)         ((op1) << 8 | (op2) << 4 | 2)
#define TERNARY(op1, op2, op3)   ((op1) << 12 | (op2) << 8 | (op1) << 4 | 3)
	if (my_explicit_count == 0) {
	    arg_types = 0;
	} else if (my_explicit_count == 1) {
	    arg_types = UNARY(args[0].op.type);
	} else if (my_explicit_count == 2) {
	    arg_types = BINARY(args[0].op.type, args[1].op.type);
	} else if (my_explicit_count == 3) {
	    arg_types = TERNARY(args[0].op.type, args[1].op.type,
				args[2].op.type);
	} else {
	    printf("%d operands!\n", my_explicit_count);
	    arg_types = -1;
	}
	switch (arg_types) {
	case 0:
	    switch (insn.type) {
	    case insn_nop:
		flags = 0;
		break;
	    case insn_return:
		if (safety & STACK_TOP_SAFETY)
		    flags = JUMP|USE_ESP|CHANGE_ESP;
		else
		    flags = JUMP|IJUMP|USE_ESP|CHANGE_ESP;
		break;
	    case insn_leave:
		if (!(unsafety & CHANGE_EBP))
		    flags = USE_ESP|CHANGE_EBP;
		else
		    flags = USE_ESP|CHANGE_EBP|USE_ESP;
		break;
	    case insn_pushflags: case insn_popflags:
		flags = USE_ESP|CHANGE_ESP;
		break;
	    case insn_szconv: /* e.g., cdq */
		flags = 0;
		break;
	    case 0xa000: /* e.g., fldz */
		flags = 0;
		break;
	    case insn_debug: /* int3 */
		flags = 0;
		break;
	    case insn_mov: /* e.g., sahf */
		flags = 0;
		break;
	    default:
		fail_verify_insn("Unknown no-argument insn", &insn,
				 insn.type, offset);
		assert(0); flags = -1;
	    }
	    break;
	case UNARY(op_relative_near):
	case UNARY(op_relative_far):	    
	    {
		unsigned long target;
		target = CODE_START + offset + insn.size;
		if (args[0].op.type == op_relative_near) {
		    target += args[0].op.data.relative_near;
		} else if (args[0].op.type == op_relative_far) {
		    target += args[0].op.data.relative_far;
		} else {
		    assert(0);
		}
		switch (insn.type) {
		case insn_jcc:
		case insn_jmp:
		case insn_call:
		    if (target & (CHUNK_SIZE - 1))
			fail_verify_insn("Unaligned literal target", &insn,
					 target, offset);
		    if (target < CODE_START || target >= CODE_END)
			fail_verify_insn("Literal target out of range", &insn,
					 target, offset);
		    flags = JUMP;
		    if (insn.type == insn_call)
			flags |= USE_ESP;
		    break;
		default:
		    fail_verify_insn("Unknown relative insn", &insn,
				     insn.type, offset);
		    assert(0); flags = -1;
		}
	    }
	    break;
	case UNARY(op_register):
	    flags = 0;
	    if (args[0].op.data.reg.id    == REGISTER_EBP ||
		args[0].op.data.reg.alias == REGISTER_EBP)
		flags |= CHANGE_EBP;
	    if (args[0].op.data.reg.id    == REGISTER_ESP ||
		args[0].op.data.reg.alias == REGISTER_ESP)
		flags |= CHANGE_ESP;
	    switch (insn.type) {
	    case insn_push:
		flags = USE_ESP|CHANGE_ESP;
		break;
	    case insn_pop:
		flags |= USE_ESP|CHANGE_ESP;
		break;
	    case insn_inc: case insn_dec:
		break;
	    case insn_movcc: /* e.g., setne */
		break;
	    case 0xa000: /* e.g., fstp %st(0) */
		break;
	    case insn_neg: case insn_not:
		break;
	    case insn_mul: case insn_div:
		break;
	    case insn_call: 
		flags |= USE_ESP;
		/* fall through */
	    case insn_jmp: /* i.e., jmp *%reg */
		flags |= JUMP;
		if (!(args[0].op.data.reg.id == REGISTER_EBX &&
		      safety & CODE_SAFETY))
		    flags |= IJUMP;
		break;
	    default:
		fail_verify_insn("Unknown one-reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case UNARY(op_immediate):
	    switch (insn.type) {
	    case insn_push:
		flags = USE_ESP|CHANGE_ESP;
		break;
	    case insn_return:
		flags = USE_ESP|CHANGE_ESP;
		break;
	    default:
		fail_verify_insn("Unknown one-immediate insn", &insn,
				 insn.type, offset);
		assert(0); flags = -1;
	    }
	    break;
	case BINARY(op_register, op_immediate):
	    flags = 0;
	    if (args[0].op.data.reg.id    == REGISTER_EBP ||
		args[0].op.data.reg.alias == REGISTER_EBP)
		flags |= CHANGE_EBP;
	    if (args[0].op.data.reg.id == REGISTER_ESP) {
		if ((insn.type == insn_add || insn.type == insn_sub) &&
		    args[1].op.datatype == op_byte) {
		    flags |= BUMP_ESP;
		} else if (insn.type == insn_and
			   && args[1].op.datatype == op_byte
			   && args[1].op.data.dword == 0xf0) {
		    flags |= BUMP_ESP;
		} else {
		    flags |= CHANGE_ESP;
		}
		    
	    }
	    switch (insn.type) {
	    case insn_and:
		if (args[0].op.data.reg.id == REGISTER_EBX &&
		    args[1].op.datatype == op_dword &&
		    args[1].op.data.dword == DATA_MASK) {
		    flags = EBX_DATA_SAFE;
		} else if (args[0].op.data.reg.id == REGISTER_EBX &&
			   args[1].op.datatype == op_dword &&
			   args[1].op.data.dword == JUMP_MASK) {
		    flags = EBX_CODE_SAFE;
		} else if (args[0].op.data.reg.id == REGISTER_EBP &&
			   args[1].op.datatype == op_dword &&
			   args[1].op.data.dword == DATA_MASK) {
		    flags = EBP_DATA_SAFE;
		} else if (args[0].op.data.reg.id == REGISTER_ESP &&
			   args[1].op.datatype == op_dword &&
			   args[1].op.data.dword == DATA_MASK) {
		    flags = ESP_SAFETY;
		}
		break;
	    case insn_add: case insn_sub:
	    case insn_mov:
	    case insn_shr: case insn_shl: case insn_rol: case insn_ror:
	    case insn_cmp: case insn_test:
	    case insn_or: case insn_xor:
		break;
	    default:
		fail_verify_insn("Unknown imm,reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case BINARY(op_register, op_register):
	    flags = 0;
	    if (args[1].op.data.reg.id    == REGISTER_EBP ||
		args[1].op.data.reg.alias == REGISTER_EBP)
		flags |= CHANGE_EBP;
	    if (args[1].op.data.reg.id    == REGISTER_ESP ||
		args[1].op.data.reg.alias == REGISTER_ESP)
		flags |= CHANGE_ESP;
	    switch (insn.type) {
	    case insn_mov:
		if (args[0].op.data.reg.id == REGISTER_ESP &&
		    args[1].op.data.reg.id == REGISTER_EBP &&
		    !(unsafety & (CHANGE_ESP|BUMP_ESP))) {
		    flags = EBP_SAFETY;
		} else if (args[0].op.data.reg.id == REGISTER_EBP &&
			   args[1].op.data.reg.id == REGISTER_ESP &&
			   !(unsafety & CHANGE_EBP)) {
		    flags = USE_ESP;
		}
		break;
	    case insn_add: case insn_sub:
	    case insn_mul: case insn_div:
	    case insn_shr: case insn_shl: case insn_rol: case insn_ror:
	    case insn_cmp: case insn_test:
	    case insn_and:
	    case insn_or: case insn_xor:
	    case insn_bittest:
	    case 0xa000: /* e.g., fxch */
		break;
	    default:
		fail_verify_insn("Unknown reg,reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case BINARY(op_register, op_expression):
	    flags = 0;
	    if (args[0].op.data.reg.id    == REGISTER_EBP ||
		args[0].op.data.reg.alias == REGISTER_EBP)
		flags |= CHANGE_EBP;
	    if (args[0].op.data.reg.id    == REGISTER_ESP ||
		args[0].op.data.reg.alias == REGISTER_ESP) {
		/* XXX ESP bump via lea */
		flags |= CHANGE_ESP;
	    }
	    switch (insn.type) {
	    case insn_mov: /* incl. lea */
	    case insn_cmp: case insn_test:
	    case insn_add: case insn_sub:
	    case insn_and:
	    case insn_or: case insn_xor:
	    case insn_mul: case insn_div:
	    case insn_bittest:
		break;
	    default:
		fail_verify_insn("Unknown (expr),reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case UNARY(op_expression):
	    switch (insn.type) {
	    case insn_push:
		flags = USE_ESP|CHANGE_ESP;
		break;
	    case 0xa000: /* e.g., fstpq */
		flags = 0; /* XXX floating stores */
		break;
	    case insn_mul: case insn_div:
		/* Read from memory, implicitly write to a reg */
		flags = 0;
		break;
	    case insn_inc: case insn_dec:
	    case insn_not: case insn_neg:
	    case insn_movcc: /* e.g., setnz */
		{
		    x86_ea_t ea = args[0].op.data.expression;
		    enum eff_addr_type ea_type = classify_ea(ea);
		    if (ea_type == EA_EBX_EXACT && safety & DATA_SAFETY)
			flags = 0;
		    else if (ea_type == EA_EBP_OFFSET
			     && !(unsafety & CHANGE_EBP))
			flags = 0;
		    else if ((ea_type == EA_ESP_OFFSET ||
			      ea_type == EA_ESP_EXACT) &&
			     !(unsafety & CHANGE_ESP))
			flags = USE_ESP;
		    else if (ea_type == EA_DIRECT &&
			     ea.disp >= DATA_START && ea.disp < DATA_END)
			flags = 0;
		    else
			flags = IWRITE;
		    break;
		}
	    default:
		fail_verify_insn("Unknown unary memory insn", &insn,
				 insn.type, offset);
		assert(0); flags = -1;
	    }
	    break;
	case BINARY(op_expression, op_register):
	    {
		int simple = 0;
		x86_ea_t ea = args[0].op.data.expression;
		enum eff_addr_type ea_type = classify_ea(ea);
		if (ea_type == EA_EBX_EXACT && safety & DATA_SAFETY) {
		    flags = 0;
		    break;
		} else if (ea_type == EA_EBP_OFFSET
			   && !(unsafety & CHANGE_EBP)) {
		    simple = USE_EBP;
		} else if ((ea_type == EA_ESP_OFFSET ||
			    ea_type == EA_ESP_EXACT) &&
			   !(unsafety & CHANGE_ESP)) {
		    simple = USE_ESP;
		} else if (ea_type == EA_DIRECT &&
			   ea.disp >= DATA_START && ea.disp < DATA_END) {
		    flags = 0;
		    break;
		}
		switch (insn.type) {
		case insn_cmp: case insn_test:
		    flags = (simple ? simple : IREAD);
		    break;
		case insn_mov:
		case insn_add: case insn_sub:
		case insn_and:
		case insn_or: case insn_xor:
		case insn_shr: case insn_shl: case insn_rol: case insn_ror:
		    flags = (simple ? simple : IWRITE);
		    break;
		default:
		    fail_verify_insn("Unknown reg,(expr) insn", &insn,
				     insn.type, offset);
		    assert(0); flags = -1;
		}
	    }
	    break;
	case BINARY(op_expression, op_immediate):
	    {
		int simple = 0;
		x86_ea_t ea = args[0].op.data.expression;
		enum eff_addr_type ea_type = classify_ea(ea);
		if (ea_type == EA_EBX_EXACT && safety & DATA_SAFETY) {
		    flags = 0;
		    break;
		} else if (ea_type == EA_EBP_OFFSET
			   && !(unsafety & CHANGE_EBP)) {
		    simple = USE_EBP;
		} else if (ea_type == EA_ESP_EXACT &&
			   insn.type == insn_and &&
			   args[1].op.datatype == op_dword &&
			   args[1].op.data.dword == JUMP_MASK) {
		    flags = USE_ESP|STACK_TOP_SAFE;
		    break;
		} else if ((ea_type == EA_ESP_OFFSET ||
			    ea_type == EA_ESP_EXACT) &&
			   !(unsafety & CHANGE_ESP)) {
		    simple = USE_ESP;
		} else if (ea_type == EA_DIRECT &&
			   ea.disp >= DATA_START && ea.disp < DATA_END) {
		    flags = 0;
		    break;
		}
		switch (insn.type) {
		case insn_and:
		case insn_mov:
		case insn_add: case insn_sub:
		case insn_or: case insn_xor:	    
		case insn_shr: case insn_shl: case insn_rol: case insn_ror:
		    flags = (simple ? simple : IWRITE);
		    break;
		case insn_cmp: case insn_test:
		    flags = (simple ? simple : IREAD);
		    break;
		default:
		    fail_verify_insn("Unknown imm,(expr) insn", &insn,
				     insn.type, offset);
		    assert(0); flags = -1;
		}
	    }
	    break;
	case BINARY(op_offset, op_register):
	    if (insn.type == insn_mov) {
		unsigned long d_offset = args[0].op.data.offset;
		if (d_offset >= DATA_START && d_offset < DATA_END) {
		    flags = 0;
		} else {
		    fail_verify_insn("Direct store outside data region", &insn,
				     d_offset, offset);
		    assert(0); flags = -1;
		}    
	    } else {
		fail_verify_insn("Unknown direct store insn", &insn,
				 insn.type, offset);
		assert(0); flags = -1;
	    }
	    break;
	case BINARY(op_register, op_offset):
	    if (insn.type == insn_mov) {
		flags = 0;
	    } else {
		fail_verify_insn("Unknown direct load insn", &insn,
				 insn.type, offset);
		assert(0); flags = -1;
	    }
	    break;
	case TERNARY(op_register, op_register, op_register):
	    flags = 0;
	    if (args[1].op.data.reg.id    == REGISTER_EBP ||
		args[1].op.data.reg.alias == REGISTER_EBP)
		flags |= CHANGE_EBP;
	    if (args[1].op.data.reg.id    == REGISTER_ESP ||
		args[1].op.data.reg.alias == REGISTER_ESP)
		flags |= CHANGE_ESP;
	    if (args[2].op.data.reg.id    == REGISTER_EBP ||
		args[2].op.data.reg.alias == REGISTER_EBP)
		flags |= CHANGE_EBP;
	    if (args[2].op.data.reg.id    == REGISTER_ESP ||
		args[2].op.data.reg.alias == REGISTER_ESP)
		flags |= CHANGE_ESP;
	    switch (insn.type) {
	    case 0xa000: /* e.g., fcom */
	    case insn_shr: case insn_shl: case insn_rol: case insn_ror:
	    case insn_inc: case insn_dec:
	    case insn_mul: case insn_div:
		break;
	    default:
		fail_verify_insn("Unknown 3-reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case TERNARY(op_register, op_expression, op_register):
	    switch (insn.type) {
	    case insn_mul: case insn_div:
		flags = 0;
		break;
	    default:
		fail_verify_insn("Unknown (expr),reg,reg insn", &insn,
				 insn.type, offset);
		assert(0); flags = -1;
	    }
	    break;
	    
	case TERNARY(op_relative_far, op_register, op_relative_far):
	    fail_verify_insn("Unknown 3-arg relative insn", &insn,
			     insn.type, offset);
	    assert(0); flags = -1;
	    break;
/* 		    x86_oplist_t *op = insn.operands; */
/* 		    int i; */
/* 		    printf("Args are:"); */
/* 		    for (i = 0; i < insn.operand_count; i++) { */
/* 			char buf[100]; */
/* 			x86_format_operand(&op->op, &insn, buf, sizeof(buf), */
/* 					   att_syntax); */
/* 			printf(" %s", buf); */
/* 			if (op->op.flags & op_implied) { */
/* 			    printf("[I]"); */
/* 			} */
/* 			printf(";"); */
/* 			op = op->next; */
/* 		    } */
/* 		    printf("\n"); */
/* 		    fail_verify_insn("Unknown many-arg insn", &insn, */
/* 				     insn.type, offset); */
	default:
	    fail_verify_insn("Illegal arg configuration", &insn,
			     arg_types, offset);
	    assert(0); flags = -1;
	}
#undef UNARY
#undef BINARY
#undef TERNARY

	if (flags & IJUMP) {
	    fail_verify_insn("Unsafe indirect jump", &insn, flags, offset);
	} else if (flags & IWRITE) {
	    fail_verify_insn("Unsafe indirect write", &insn, flags, offset);
	} else if (flags & JUMP && unsafety & (CHANGE_ESP|BUMP_ESP)) {
	    fail_verify_insn("Unsafe %esp escapes by jump", &insn,
			     flags, offset);
	} else if (flags & JUMP && unsafety & CHANGE_EBP) {
	    fail_verify_insn("Unsafe %ebp escapes by jump", &insn,
			     flags, offset);
	}
	if (insn.type == insn_popflags)
	    safety = safety & (EBX_DATA_SAFE|EBX_CODE_SAFE);
	else
	    safety = 0;
	safety |= flags & (EBX_DATA_SAFE|EBX_CODE_SAFE|STACK_TOP_SAFE|
			   EBP_DATA_SAFE);
	unsafety |= flags & (CHANGE_ESP|CHANGE_EBP|BUMP_ESP);
	if (flags & BUMP_ESP)
	    bump_count++;
	if (bump_count >= 250)
	    unsafety |= CHANGE_ESP;
	if (flags & USE_ESP) {
	    unsafety &= ~(CHANGE_ESP|BUMP_ESP);
	    bump_count = 0;
	}
	if (flags & EBP_SAFETY)
	    unsafety &= ~CHANGE_EBP;
	/*printf("After %08x: ESP: %x    EBP: %x    EBX: %x\n", offset,
	       unsafety & (CHANGE_ESP|BUMP_ESP), unsafety & CHANGE_EBP,
	       safety & (EBX_DATA_SAFE|EBX_CODE_SAFE));*/

	x86_oplist_free(&insn);
	offset += len;
    }
    fprintf(stderr, "Verification finished at 0x%08x\n", CODE_START + offset);
    x86_cleanup();
}
#endif

int main(int argc, char **argv) {
    Elf *elf;
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdr;
    Elf32_Shdr *shdr;
    Elf_Scn *scn;
    int fd;
    int ph_count;
    int data_size = 0;
    int data_len = 0, data_offset = 0;
    int code_len = 0, code_offset = 0;
    int rodata_len = 0, rodata_offset = 0;
    int ret, count;
    void *retp;
    unsigned version;
    const char *fio_name;

    /* Do this early, before libc tries to use the space */
    retp = mmap((void *)DATA_START, DATA_SIZE, PROT_READ | PROT_WRITE,
		MAP_FIXED | MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS, 0, 0);
    if (retp == MAP_FAILED) {
	perror("data mmap:");
    }

#if 0
#ifndef LOADER_FIO
    fprintf(stderr, "Loader running with main at %p, malloc arena around %p\n",
	   (void *)main, malloc(1));
#endif    
#endif

#ifdef LOADER_FIO
    fio_name = LOADER_FIO;
#else
    fio_name = argv[1];
    argc--;
    argv++;
#endif
    fd = open(fio_name, O_RDONLY);
    if (fd == -1) {
	perror("Load fio");
    }
    version = elf_version(EV_CURRENT);
    assert(version != EV_NONE);
    elf = elf_begin(fd, ELF_C_READ, 0);
    ehdr = elf32_getehdr(elf);
    ph_count = ehdr->e_phnum;
    /*assert(ph_count == 2);*/
    phdr = elf32_getphdr(elf);
#ifdef CODE_IS_LOWER
    assert(phdr[0].p_type == PT_LOAD);
    assert(phdr[0].p_vaddr == CODE_START);
    assert(phdr[0].p_paddr == CODE_START);
    assert(phdr[0].p_memsz <= CODE_SIZE);
    assert(phdr[0].p_flags == (PF_R | PF_X));

    assert(phdr[1].p_type == PT_LOAD);
    assert(phdr[1].p_vaddr == DATA_START);
    assert(phdr[1].p_paddr == DATA_START);
    assert(phdr[1].p_memsz <= DATA_SIZE);
    assert(phdr[1].p_flags == (PF_R | PF_W));
#else
    assert(phdr[0].p_type == PT_LOAD);
    assert(phdr[0].p_vaddr == DATA_START);
    assert(phdr[0].p_paddr == DATA_START);
    assert(phdr[0].p_memsz <= DATA_SIZE);
    assert(phdr[0].p_flags == (PF_R | PF_W));

    assert(phdr[1].p_type == PT_LOAD);
    assert(phdr[1].p_vaddr == CODE_START);
    assert(phdr[1].p_paddr == CODE_START);
    assert(phdr[1].p_memsz <= CODE_SIZE);
    assert(phdr[1].p_flags == (PF_R | PF_X));
#endif
    
    scn = 0;
    while ((scn = elf_nextscn(elf, scn)) != 0) {
	char *name;
	shdr = elf32_getshdr(scn);
	name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
	if (!name)
	    continue;
	/*printf("%s\n", name); */
	if (!strcmp(name, ".text")) {
	    void *ret;
	    assert(shdr->sh_size <= CODE_SIZE);
	    assert(shdr->sh_addr == CODE_START);
	    assert(shdr->sh_type == SHT_PROGBITS);
	    assert(shdr->sh_flags == (SHF_ALLOC | SHF_EXECINSTR));
	    assert(!(shdr->sh_offset & 0xfff));
/* 	    mmap((void *)0x10000000, shdr->sh_size, PROT_EXEC | PROT_READ, */
/* 		 MAP_FIXED | MAP_SHARED, fd, shdr->sh_offset); */
	    ret = mmap((void *)CODE_START, CODE_SIZE /*shdr->sh_size*/,
		       PROT_EXEC | PROT_READ | PROT_WRITE,
		       MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	    if (ret == MAP_FAILED)
		perror("mmap:");
	    code_len = shdr->sh_size;
	    code_offset = shdr->sh_offset;
	} else if (!strcmp(name, ".rodata")) {
	    rodata_len = shdr->sh_size;
	    rodata_offset = shdr->sh_offset;
	    abort();
	} else if (!strcmp(name, ".data")) {
	    assert(shdr->sh_size <= DATA_SIZE);
	    assert(shdr->sh_addr == DATA_START);
	    assert(shdr->sh_type == SHT_PROGBITS);
	    assert(shdr->sh_flags == (SHF_WRITE | SHF_ALLOC));
	    assert(!(shdr->sh_offset & 0xfff));
	    data_len = shdr->sh_size;
	    data_offset = shdr->sh_offset;
	    data_size += shdr->sh_size;
	} else if (!strcmp(name, ".bss")) {
	    data_size += shdr->sh_size;
	}
    }
    assert(data_size <= DATA_SIZE);
    elf_end(elf);
    lseek(fd, code_offset, SEEK_SET);
    count = read(fd, (void *)CODE_START, code_len);
    assert(count == code_len);
    /*lseek(fd, rodata_offset, SEEK_SET);
    printf("Loading %d bytes of read-only data at %x\n",
	   rodata_len, (0x10000000 + code_len));
	   read(fd, (void *)(0x10000000 + code_len), rodata_len); */
    lseek(fd, data_offset, SEEK_SET);
    count = read(fd, (void *)DATA_START, data_len);
    assert(count == data_len);
    close(fd);
    init_wrappers();
#ifdef VERIFY
    verify(code_len);
#endif
    install_stubs();
    data_break = (void *)(DATA_START + data_size);
    /* Align the start of the heap to a page boundary. This should
       also compensate for any padding of the .bss section that wasn't
       accounted for above. */
    data_break = (void *)((unsigned)(data_break + 2*4096) & ~4095);
#if 0
    {
	/* Set up %gs to point to a page at the top of the data
	   sandbox region. */
	unsigned gs_ptr = (unsigned)DATA_END - 4096;
	inside_gs = setup_gs_ldt(gs_ptr);
	inside_ebp = inside_esp = gs_ptr - 4;
    }
#else
    inside_ebp = inside_esp = (unsigned)DATA_END - 4;
#endif
    ret = call_in((void*)CODE_START, argc, argv);
#ifndef LOADER_FIO
    fprintf(stderr, "Module returned %d\n", ret);
#endif
    return ret;
}

#undef _GNU_SOURCE
#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
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
	asm("movl  inside_esp, %esp");
	asm("movl  inside_ebp, %ebp");
	asm("call  *%eax");
	asm("movl  %esp, inside_esp");
	asm("movl  %ebp, inside_ebp");
	asm("movl  outside_esp, %esp");
	asm("movl  outside_ebp, %ebp");
	asm("movl  outside_ebx, %ebx");
	asm("movl  outside_esi, %esi");
	asm("movl  outside_edi, %edi");
	asm("movl  %%eax, %0" : "=r" (ret));
	return ret;
    }
    return exit_value;
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

double wrap_sin(double x) {
    return sin(x);
}

double wrap_sqrt(double x) {
    return sqrt(x);
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

void fail_verify_insn(const char *msg, x86_insn_t *insn, int arg, int offset) {
    char buf[100];
    x86_format_insn(insn, buf, sizeof(buf), att_syntax);
    printf("Verification failed: %s (0x%x) at offset 0x%08x\n",
	   msg, arg, offset);
    printf("Bad instruction %s\n", buf);
    exit(1);
}

void verify(int code_len) {
    int offset = 0;
    int next_aligned = offset;
    x86_init(opt_none, 0);
    while (offset < code_len) {
	x86_insn_t insn;
	int len, i;
	int arg_types;
	x86_oplist_t args[4], *arg;
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
	assert(insn.explicit_count <= 3);
	arg = insn.operands;
	i = 0;
	for (arg = insn.operands; arg; arg = arg->next) {
	    if (!(arg->op.flags & op_implied)) {
		args[i] = *arg;
		args[i].next = &args[i+1];
		i++;
	    }
	}
	args[i-1].next = 0;
	assert(i == insn.explicit_count);

#define UNARY(op1)               ((op1) << 4 | 1)
#define BINARY(op1, op2)         ((op1) << 8 | (op2) << 4 | 2)
#define TERNARY(op1, op2, op3)   ((op1) << 12 | (op2) << 8 | (op1) << 4 | 3)
	if (insn.explicit_count == 0) {
	    arg_types = 0;
	} else if (insn.explicit_count == 1) {
	    arg_types = UNARY(args[0].op.type);
	} else if (insn.explicit_count == 2) {
	    arg_types = BINARY(args[0].op.type, args[1].op.type);
	} else if (insn.explicit_count == 3) {
	    arg_types = TERNARY(args[0].op.type, args[1].op.type,
				args[2].op.type);
	} else {
	    printf("%d operands!\n", insn.explicit_count);
	    arg_types = -1;
	}
	switch (arg_types) {
	case 0:
	    switch (insn.type) {
	    case insn_return:
	    case insn_nop:
	    case insn_debug: /* int3 */
	    case insn_szconv: /* e.g., cdq */
	    case 0xa000: /* e.g., fldz */
	    case insn_pushflags:
	    case insn_popflags:
	    case insn_mov: /* e.g., sahf */
	    case insn_leave:
		break;
	    default:
		fail_verify_insn("Unknown no-argument insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case UNARY(op_relative_near):
	case UNARY(op_relative_far):
	    switch (insn.type) {
	    case insn_jcc:
	    case insn_jmp:
	    case insn_call:
		break;
	    default:
		fail_verify_insn("Unknown relative insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case UNARY(op_register):
	    switch (insn.type) {
	    case insn_push:
	    case insn_pop:
	    case insn_jmp:
	    case insn_call:
	    case insn_inc: case insn_dec:
	    case insn_neg: case insn_not:
	    case insn_movcc: /* e.g., setne */
	    case 0xa000: /* e.g., fstp %st(0) */
		break;
	    default:
		fail_verify_insn("Unknown one-reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case UNARY(op_immediate):
	    switch (insn.type) {
	    case insn_push:
	    case insn_return:
	      break;
	    default:
	      fail_verify_insn("Unknown one-immediate insn", &insn,
			       insn.type, offset);
	    }
	    break;
	case BINARY(op_register, op_immediate):
	    switch (insn.type) {
	    case insn_add: case insn_sub:
	    case insn_and:
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
	    switch (insn.type) {
	    case insn_add: case insn_sub:
	    case insn_mul: case insn_div:
	    case insn_mov:
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
	    switch (insn.type) {
	    case insn_mov: /* incl. lea */
	    case insn_cmp: case insn_test:
	    case insn_add: case insn_sub:
	    case insn_and:
	    case insn_or: case insn_xor:
	    case insn_mul: case insn_div:
		break;
	    default:
		fail_verify_insn("Unknown (expr),reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case UNARY(op_expression):
	    switch (insn.type) {
	    case insn_inc: case insn_dec:
	    case insn_not: case insn_neg:
	    case insn_movcc: /* e.g., setnz */
	    case insn_push:
	    case 0xa000: /* e.g., fstpq */
		break;
	    default:
		fail_verify_insn("Unknown unary memory insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case BINARY(op_expression, op_register):
	    switch (insn.type) {
	    case insn_mov:
	    case insn_add: case insn_sub:

	    case insn_cmp: case insn_test:

	    case insn_and:
	    case insn_or: case insn_xor:
	    case insn_shr: case insn_shl: case insn_rol: case insn_ror:
		break;
	    default:
		fail_verify_insn("Unknown reg,(expr) insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case BINARY(op_expression, op_immediate):
	    switch (insn.type) {
	    case insn_and: /* incl. and $mask, (%esp) */

	    case insn_mov:
	    case insn_add: case insn_sub:
	    case insn_or: case insn_xor:	    
	    case insn_shr: case insn_shl: case insn_rol: case insn_ror:

	    case insn_cmp: case insn_test:
		break;
	    default:
		fail_verify_insn("Unknown imm,(expr) insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case BINARY(op_offset, op_register):
	    if (insn.type != insn_mov) {
		fail_verify_insn("Unknown direct store insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case BINARY(op_register, op_offset):
	    if (insn.type != insn_mov) {
		fail_verify_insn("Unknown direct load insn", &insn,
				 insn.type, offset);
	    }
	    break;
	case TERNARY(op_register, op_register, op_register):
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
		break;
	    default:
		fail_verify_insn("Unknown (expr),reg,reg insn", &insn,
				 insn.type, offset);
	    }
	    break;
	    
	case TERNARY(op_relative_far, op_register, op_relative_far):
	    fail_verify_insn("Unknown 3-arg relative insn", &insn,
			     insn.type, offset);
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
	}
#undef UNARY
#undef BINARY
#undef TERNARY

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
    int data_len, data_offset;
    int code_len, code_offset;
    int rodata_len = 0, rodata_offset = 0;
    int ret;
    void *retp;
    unsigned version;
    const char *fio_name;

    /* Do this early, before libc tries to use the space */
    retp = mmap((void *)DATA_START, DATA_SIZE, PROT_READ | PROT_WRITE,
		MAP_FIXED | MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS, 0, 0);
    if (retp == MAP_FAILED) {
	perror("data mmap:");
    }

#ifndef LOADER_FIO
    fprintf(stderr, "Loader running with main at %p, malloc arena around %p\n",
	   (void *)main, malloc(1));
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
    read(fd, (void *)CODE_START, code_len);
    /*lseek(fd, rodata_offset, SEEK_SET);
    printf("Loading %d bytes of read-only data at %x\n",
	   rodata_len, (0x10000000 + code_len));
	   read(fd, (void *)(0x10000000 + code_len), rodata_len); */
    lseek(fd, data_offset, SEEK_SET);
    read(fd, (void *)DATA_START, data_len);
    close(fd);
    init_wrappers();
    install_stubs();
#ifdef VERIFY
    verify(code_len);
#endif
    data_break = (void *)(DATA_START + data_size);
    /* Align the start of the heap to a page boundary. This should
       also compensate for any padding of the .bss section that wasn't
       accounted for above. */
    data_break = (void *)((unsigned)(data_break + 2*4096) & ~4095);
    inside_ebp = inside_esp = (unsigned)DATA_END - 4;
    ret = call_in((void*)CODE_START, argc, argv);
#ifndef LOADER_FIO
    fprintf(stderr, "Module returned %d\n", ret);
#endif
    return ret;
}

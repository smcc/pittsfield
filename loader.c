#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/resource.h>
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

int call_in(void *addr) {
    int ret;
    if (!setjmp(exit_buf)) {
	asm("movl %0, %%eax" :: "r" (addr));
	asm("movl %esp, outside_esp");
	asm("movl %ebp, outside_ebp");
	asm("movl %ebx, outside_ebx");
	asm("movl %esi, outside_esi");
	asm("movl %edi, outside_edi");
	asm("movl inside_esp, %esp");
	asm("movl inside_ebp, %ebp");
	asm("call *%eax");
	asm("movl %esp, inside_esp");
	asm("movl %ebp, inside_ebp");
	asm("movl outside_esp, %esp");
	asm("movl outside_ebp, %ebp");
	asm("movl outside_ebx, %ebx");
	asm("movl outside_esi, %esi");
	asm("movl outside_edi, %edi");
	asm("movl %%eax, %0" : "=r" (ret));
	return ret;
    }
    return exit_value;
}

extern unsigned long long
__udivdi3(unsigned long long a, unsigned long long b);
unsigned long long wrap___udivdi3(unsigned long long a, unsigned long long b) {
    return __udivdi3(a, b);
}

extern long long
__divdi3(long long a, long long b);
long long wrap___divdi3(long long a, long long b) {
    return __divdi3(a, b);
}

extern unsigned long long
__umoddi3(unsigned long long a, unsigned long long b);
unsigned long long wrap___umoddi3(unsigned long long a, unsigned long long b) {
    return __umoddi3(a, b);
}

extern long long
__moddi3(long long a, long long b);
long long wrap___moddi3(long long a, long long b) {
    return __moddi3(a, b);
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
    void *new_brk = data_break + incr;
    if ((unsigned)new_brk > (unsigned)DATA_START &&
	(unsigned)new_brk < (unsigned)DATA_END) {
	void *old_brk = data_break;
	/*printf("Moving break by %lx to %p\n", incr, new_brk);*/
	data_break = new_brk;
	return old_brk;
    } else {
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

int wrap_outside_fgetc(int fi) {
    return fgetc(files[fi]);
}

int wrap_outside_vfprintf(int fi, const char *fmt, va_list ap) {
    return vfprintf(files[fi], fmt, ap);
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
    return sscanf(str, fmt, ap);
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

double wrap_acos(double x) {
    return acos(x);
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
};

int wrap_outside_stat(const char *fname, struct inside_stat *in_buf) {
    struct stat buf;
    int ret = stat(fname, &buf);
    if (!ret) {
	in_buf->inside_st_size = buf.st_size;
	in_buf->inside_st_mode = buf.st_mode;
	in_buf->inside_st_mtime = buf.st_mtime;
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

void wrap_fail_check(void) {
    printf("Program has performed an illegal operation "
	   "and will be terminated\n");
    exit(1);
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
    int addr = CODE_START + 2*CHUNK_SIZE + 1;
    int (**p)();
    for (p = stubs; *p; p++) {
	/* jmp is PC relative; specificially, relative to the PC after
	   the end of the jump instruction. */
	int offset = (int)*p - (addr + 4);
	*((int *)addr) = offset;
	addr += STUB_SIZE;
    }
}

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

    /* Do this early, before libc tries to use the space */
    retp = mmap((void *)DATA_START, DATA_SIZE, PROT_READ | PROT_WRITE,
		MAP_FIXED | MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS, 0, 0);
    if (retp == MAP_FAILED) {
	perror("data mmap:");
    }

    printf("Loader running with main at %p, malloc arena around %p\n",
	   (void *)main, malloc(1));
    
    fd = open(argv[1], O_RDONLY);
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
    data_break = (void *)(DATA_START + data_size);
    inside_ebp = inside_esp = (unsigned)DATA_END - 4;
    ret = call_in((void*)CODE_START);
    printf("%d\n", ret);
    return ret;
}

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

FILE *outside_files[16] = {/*stdin, stdout, stderr,*/ 0, 0, 0,
			   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int outside_fopen(const char *path, const char *mode) {
    int i;
    for (i = 0; i < 16; i++)
	if (!outside_files[i])
	    break;
    if (i == 16)
	abort();
    outside_files[i] = fopen(path, mode);
    if (!outside_files[i])
	return -1;
    return i;
}

int outside_fdopen(int fd, const char *mode) {
    int i;
    for (i = 0; i < 16; i++)
	if (!outside_files[i])
	    break;
    if (i == 16)
	abort();
    outside_files[i] = fdopen(fd, mode);
    if (!outside_files[i])
	return -1;
    return i;
}

int outside_fclose(int fi) {
    int ret = fclose(outside_files[fi]);
    outside_files[fi] = 0;
    return ret;
}

int outside_errno(void) {
    return errno;
}

int outside_ferror(int fi) {
    return ferror(outside_files[fi]);
}

int outside_fflush(int fi) {
    return fflush(outside_files[fi]);
}

int outside_fgetc(int fi) {
    return fgetc(outside_files[fi]);
}

int outside_vfprintf(int fi, const char *fmt, va_list ap) {
    return vfprintf(outside_files[fi], fmt, ap);
}

size_t outside_fread(void *ptr, size_t size, size_t num, int fi) {
    return fread(ptr, size, num, outside_files[fi]);
}

size_t outside_fwrite(const void *ptr, size_t size, size_t num, int fi) {
    return fwrite(ptr, size, num, outside_files[fi]);
}

void outside_rewind(int fi) {
    rewind(outside_files[fi]);
}

int outside_ungetc(int c, int fi) {
    return ungetc(c, outside_files[fi]);
}

char *outside_getenv(const char *name) {
    return getenv(name);
}

int outside_access(const char *pathname, int mode) {
    return access(pathname, mode);
}

int outside_close(int fd) {
    return close(fd);
}

int outside_fseek(int fi, long offset, int whence) {
    return fseek(outside_files[fi], offset, whence);
}

long outside_ftell(int fi) {
    return ftell(outside_files[fi]);
}

struct inside_stat {
    off_t inside_st_size;
    int inside_st_mode;
    time_t inside_st_mtime;
};

int outside_stat(const char *fname, struct inside_stat *in_buf) {
    struct stat buf;
    int ret = stat(fname, &buf);
    if (!ret) {
	in_buf->inside_st_size = buf.st_size;
	in_buf->inside_st_mode = buf.st_mode;
	in_buf->inside_st_mtime = buf.st_mtime;
    }
    return ret;
}

int outside_fstat(int fd, struct inside_stat *in_buf) {
    struct stat buf;
    int ret = fstat(fd, &buf);
    if (!ret) {
	in_buf->inside_st_size = buf.st_size;
	in_buf->inside_st_mode = buf.st_mode;
	in_buf->inside_st_mtime = buf.st_mtime;
    }
    return ret;
}

off_t outside_lseek(int fd, off_t offset, int whence) {
    return lseek(fd, offset, whence);
}

int outside_open(const char *pathname, int flags, int mode) {
    return open(pathname, flags, mode);
}

int outside_read(int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}

clock_t outside_times(struct tms *buf) {
    return times(buf);
}

DIR *outside_opendir(const char *name) {
    return opendir(name);
}

int outside_closedir(DIR *dir) {
    return closedir(dir);
}

struct dirent *outside_readdir(DIR *dir) {
    return readdir(dir);
}

int outside_fputc(int c, int fi) {
    return fputc(c, outside_files[fi]);
}

int outside_fputs(const char *s, int fi) {
    return fputs(s, outside_files[fi]);
}

int outside_fileno(int fi) {
    return fileno(outside_files[fi]);
}

int outside_vasprintf(char **strp, const char *fmt, va_list ap) {
    return vasprintf(strp, fmt, ap);
}

void outside_free(void *ptr) {
    free(ptr);
}

int outside_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    return vsnprintf(str, size, format, ap);
}

void init_wrappers() {
    outside_files[0] = stdin;
    outside_files[1] = stdout;
    outside_files[2] = stderr;
}

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

int outside_execv(const char *path, char *const argv[]) {
    return execv(path, argv);
}

int outside_vfscanf(int fi, const char *fmt, va_list ap) {
    return vfscanf(outside_files[fi], fmt, ap);
}

int outside_isatty(int fd) {
    return isatty(fd);
}

char *outside_fgets(char *buf, int size, int fi) {
    return fgets(buf, size, outside_files[fi]);
}

int outside_chdir(const char *path) {
    return chdir(path);
}

int outside_chmod(const char *path, mode_t mode) {
    return chmod(path, mode);
}

int outside_dup2(int oldfd, int newfd) {
    return dup2(oldfd, newfd);
}

int outside_dup(int oldfd) {
    return dup(oldfd);
}

int outside_execvp(const char *file, char *const argv[]) {
    return execvp(file, argv);
}

pid_t outside_fork(void) {
    return fork();
}

int outside_ftruncate(int fd, off_t length) {
    return ftruncate(fd, length);
}

int outside_mkdir(const char *pathname, mode_t mode) {
    return mkdir(pathname, mode);
}

int outside_rename(const char *oldpath, const char *newpath) {
    return rename(oldpath, newpath);
}

pid_t outside_wait(int *status) {
    return wait(status);
}

int outside_pipe(int fds[2]) {
    return pipe(fds);
}

int outside_rmdir(const char *path) {
    return rmdir(path);
}

int outside_select(int n, fd_set *rfds, fd_set *wfds, fd_set *xfds,
		   struct timeval *tv) {
    return select(n, rfds, wfds, xfds, tv);
}

int outside_lstat(const char *fname, struct stat *buf) {
    return lstat(fname, buf);
}

int outside_truncate(const char *path, off_t length) {
    return truncate(path, length);
}

int outside_unlink(const char *path) {
    return unlink(path);
}

int outside_write(int fd, const void *buf, size_t count) {
    return write(fd, buf, count);
}

void outside_clearerr(int fi) {
    clearerr(outside_files[fi]);
}

int outside_feof(int fi) {
    return feof(outside_files[fi]);
}

void outside_setbuf(int fi, char *buf) {
    setbuf(outside_files[fi], buf);
}


void init_wrappers() {
    outside_files[0] = stdin;
    outside_files[1] = stdout;
    outside_files[2] = stderr;
}

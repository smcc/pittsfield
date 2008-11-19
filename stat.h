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

REPLACEMENT int fstat(int fd, struct stat *buf);
REPLACEMENT int stat(const char *file_name, struct stat *buf);
REPLACEMENT int lstat(const char *file_name, struct stat *buf);

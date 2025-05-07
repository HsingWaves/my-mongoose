#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <windows.h>  // For directory listing and mkdir

#define MG_FS_READ  0
#define MG_FS_WRITE 1
#define MG_PATH_MAX 256

struct mg_str {
    char *buf;
    size_t len;
};

struct mg_fd {
    void *fd;
    struct mg_fs *fs;
};

struct mg_fs {
    int (*st)(const char *, size_t *, time_t *);
    void (*ls)(const char *, void (*)(const char *, void *), void *);
    void *(*op)(const char *, int);
    void (*cl)(void *);
    size_t (*rd)(void *, void *, size_t);
    size_t (*wr)(void *, const void *, size_t);
    size_t (*sk)(void *, size_t);
    bool (*mv)(const char *, const char *);
    bool (*rm)(const char *);
    bool (*mkd)(const char *);
};

// ======== POSIX-compatible file operations ========

static int win_stat(const char *path, size_t *size, time_t *mtime) {
    struct _stat st;
    if (_stat(path, &st) != 0) return -1;
    if (size) *size = (size_t) st.st_size;
    if (mtime) *mtime = st.st_mtime;
    return 0;
}

static void win_ls(const char *path, void (*fn)(const char *, void *), void *userdata) {
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFile(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, ".."))
            fn(find_data.cFileName, userdata);
    } while (FindNextFile(hFind, &find_data));
    FindClose(hFind);
}

static void *win_open(const char *path, int flags) {
    return fopen(path, flags == MG_FS_READ ? "rb" : "wb");
}

static void win_close(void *fd) {
    fclose((FILE *) fd);
}

static size_t win_read(void *fd, void *buf, size_t len) {
    return fread(buf, 1, len, (FILE *) fd);
}

static size_t win_write(void *fd, const void *buf, size_t len) {
    return fwrite(buf, 1, len, (FILE *) fd);
}

static size_t win_seek(void *fd, size_t offset) {
    fseek((FILE *) fd, offset, SEEK_SET);
    return (size_t) ftell((FILE *) fd);
}

static bool win_rename(const char *from, const char *to) {
    return MoveFileEx(from, to, MOVEFILE_REPLACE_EXISTING) != 0;
}

static bool win_remove(const char *path) {
    return remove(path) == 0;
}

static bool win_mkdir(const char *path) {
    return _mkdir(path) == 0;
}

struct mg_fs mg_fs_win = {
    win_stat, win_ls, win_open, win_close, win_read, win_write,
    win_seek, win_rename, win_remove, win_mkdir
};

// ======== mg_fs helpers (same as before) ========

struct mg_fd *mg_fs_open(struct mg_fs *fs, const char *path, int flags) {
    struct mg_fd *fd = calloc(1, sizeof(*fd));
    if (fd != NULL) {
        fd->fd = fs->op(path, flags);
        fd->fs = fs;
        if (fd->fd == NULL) {
            free(fd);
            fd = NULL;
        }
    }
    return fd;
}

void mg_fs_close(struct mg_fd *fd) {
    if (fd != NULL) {
        fd->fs->cl(fd->fd);
        free(fd);
    }
}

struct mg_str mg_file_read(struct mg_fs *fs, const char *path) {
    struct mg_str result = {NULL, 0};
    void *fp;
    fs->st(path, &result.len, NULL);
    if ((fp = fs->op(path, MG_FS_READ)) != NULL) {
        result.buf = calloc(1, result.len + 1);
        if (result.buf != NULL &&
            fs->rd(fp, result.buf, result.len) != result.len) {
            free(result.buf);
            result.buf = NULL;
        }
        fs->cl(fp);
    }
    if (result.buf == NULL) result.len = 0;
    return result;
}

bool mg_file_write(struct mg_fs *fs, const char *path, const void *buf, size_t len) {
    bool result = false;
    struct mg_fd *fd;
    char tmp[MG_PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    if ((fd = mg_fs_open(fs, tmp, MG_FS_WRITE)) != NULL) {
        result = fs->wr(fd->fd, buf, len) == len;
        mg_fs_close(fd);
        if (result) {
            fs->rm(path);
            fs->mv(tmp, path);
        } else {
            fs->rm(tmp);
        }
    }
    return result;
}

char *mg_vmprintf(const char *fmt, va_list *ap) {
    va_list ap_copy;
    va_copy(ap_copy, *ap);
    int len = _vscprintf(fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0) return NULL;

    char *buf = (char *) malloc(len + 1);
    if (buf == NULL) return NULL;

    vsnprintf(buf, len + 1, fmt, *ap);
    return buf;
}

bool mg_file_printf(struct mg_fs *fs, const char *path, const char *fmt, ...) {
    va_list ap;
    char *data;
    bool result = false;
    va_start(ap, fmt);
    data = mg_vmprintf(fmt, &ap);
    va_end(ap);
    result = mg_file_write(fs, path, data, strlen(data));
    free(data);
    return result;
}

static void mg_fs_ls_fn(const char *filename, void *param) {
    printf(" - %s\n", filename);
}

bool mg_fs_ls(struct mg_fs *fs, const char *path, char *buf, size_t len) {
    fs->ls(path, mg_fs_ls_fn, buf);
    return true;
}

// ==== TEST MAIN ====

int main() {
    const char *filename = "test.txt";

    printf("Writing file...\n");
    mg_file_printf(&mg_fs_win, filename, "Hello, %s! Number: %d\n", "world", 42);

    printf("Reading file...\n");
    struct mg_str content = mg_file_read(&mg_fs_win, filename);
    if (content.buf) {
        printf("File content (%zu bytes):\n%s", content.len, content.buf);
        free(content.buf);
    } else {
        printf("Failed to read file.\n");
    }

    printf("Listing directory...\n");
    char buf[1] = {0};
    mg_fs_ls(&mg_fs_win, ".", buf, sizeof(buf));

    printf("Removing file...\n");
    mg_fs_win.rm(filename);

    printf("Done.\n");
    return 0;
}

#include "Utilities.h"

// return the mode, in failure return something < 0
mode_t path_get_mode(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        return -1;
    }
    return buf.st_mode;
}

// return 1 if the path is a directory, else 0
int is_dir(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        return 0;
    }
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

// return 1 if files differ, 0 if they don't
int files_diff(char *in_path, char *out_path) {
    struct stat out_buf, in_buf;
    memset(&in_buf, 0, sizeof(in_buf));
    if (lstat(in_path, &in_buf) < 0) {
        return 1;
    }
    memset(&out_buf, 0, sizeof(out_buf));
    if (lstat(out_path, &out_buf) < 0) {
        return 1;
    }

    return (in_buf.st_mode & S_IFMT) == (out_buf.st_mode & S_IFMT) &&  // check if the files are of the same type
                   in_buf.st_size == out_buf.st_size &&  // check if the files are of the same size
                   in_buf.st_mtime <= out_buf.st_mtime 
                   ? 0 : 1;  // check if the src is modified later than it's relative copy
}

// return 1 if the path is symbolic link, else 0
int is_sym(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        return 0;
    }
    return (buf.st_mode & S_IFMT) == S_IFLNK;
}

// int get_hardlinks(char *path) {
//     // TODO
// }

// small procedure that takes as input the __opened__ fd's
// of one input and one output file and copies the containings of
// the first to the second. Returns 1 in success, otherwise 0
int copy_file(int in_fd, int out_fd, int BUFFSIZE) {
    ssize_t bytes = 0;
    char buf[BUFFSIZE];
    memset(buf, 0, BUFFSIZE);
    // start copying the file
    while ((bytes = read(in_fd, buf, BUFFSIZE)) > 0) {
        if (write(out_fd, buf, bytes) < bytes) {
            // error in writing
            close(in_fd);
            close(out_fd);
            return FILE_CP_FAIL;
        }
    }

    close(in_fd);
    close(out_fd);
    if (bytes < 0)
        return FILE_CP_FAIL;
    else
        return FILE_CP_SUCC;
}

// function that creates a dir with perms 0777
DIR *create_dir(char *path) {
    if (mkdir(path, 0755) < 0) {
        fprintf(stderr, "Cannot create dir '%s'.\n", path);
    }
    return opendir(path);
}

void delete_element(char *path) {
    if(unlink(path) != 0) {
        char b[BUFSIZ];
        sprintf(b, "Unlinking '%s'", path);
        perror(b);
        exit(1);
    }
}
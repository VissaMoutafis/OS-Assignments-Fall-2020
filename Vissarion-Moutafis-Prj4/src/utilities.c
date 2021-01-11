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

    if ((in_buf.st_mode & S_IFMT) == S_IFLNK)
        return 0;

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
    printf("Trying to delete '%s'\n", path);
    if(unlink(path) != 0) {
        char b[BUFSIZ];
        sprintf(b, "Unlinking '%s'", path);
        perror(b);
        exit(1);
    }
}

void delete_dir(char *path) {
    DIR *dp;
    struct dirent *dir;

    // first open the src directory
    if ((dp = opendir(path)) == NULL) {
        fprintf(stderr, "Trouble opening directory '%s'.\n", path);
        exit(1);
    }

    // Now we are ready to check the containings of the  target directory
    while ((dir = readdir(dp)) != NULL) {
        if (strcmp(".", dir->d_name) == 0 ||
            strcmp("..", dir->d_name) == 0)
            continue;

        // reconstruct the suppossed path as it would be in the original
        // directory
        char *element_path = calloc(strlen(path) + 1 + strlen(dir->d_name) + 1, sizeof(char));
        strcpy(element_path, path);
        strcat(element_path, "/");
        strcat(element_path, dir->d_name);

        if (is_dir(element_path))
            delete_dir(element_path);
        else
            delete_element(element_path);

        // free the allocated memory
        free(element_path);
        element_path = NULL;
    }
    printf("Trying to delete '%s'\n", path);
    rmdir(path);
}


int check_deleted(char *src_dir, char *trg_dir) {
    DIR *trg_dp;
    struct dirent *dirent_trg;

    // first open the src directory 
    if ((trg_dp = opendir(trg_dir)) == NULL) {
        fprintf(stderr, "Trouble opening directory '%s'.\n", src_dir);
        return FAIL;
    }

    // Now we are ready to check the containings of the  target directory
    while ((dirent_trg = readdir(trg_dp)) != NULL) {
        if (strcmp(".", dirent_trg->d_name) == 0 || strcmp("..", dirent_trg->d_name) == 0)
            continue;

        // reconstruct the suppossed path as it would be in the original directory
        char *src_path = calloc(strlen(src_dir) + 1 + strlen(dirent_trg->d_name) + 1, sizeof(char));
        strcpy(src_path, src_dir);
        strcat(src_path, "/");
        strcat(src_path, dirent_trg->d_name);
        // construct the path on the trg dir
        char *out_path = calloc(strlen(trg_dir) + 1 + strlen(dirent_trg->d_name) + 1, sizeof(char));
        strcpy(out_path, trg_dir);
        strcat(out_path, "/");
        strcat(out_path, dirent_trg->d_name);

        // Now check if the element exists in the src directory
        struct stat buf;
        memset(&buf, 0, sizeof(buf));
        int exists_in_src = !(lstat(src_path, &buf) < 0 || buf.st_ino == 0);
        int exists_in_trg = !(lstat(out_path, &buf) < 0 || buf.st_ino == 0);

        if (exists_in_trg && !exists_in_src) {
            if (is_dir(out_path)) 
                delete_dir(out_path);
            else 
                delete_element(out_path);
        }

        // free the allocated memory
        free(src_path);
        src_path = NULL;
        free(out_path);
        out_path = NULL;
    }
    
    return SUCC;
}


int detect_cycle(char *src_dir, char* trg_path) {
    DIR *src_dp;
    struct dirent *dirent_src;

    // first open the src directory 
    if ((src_dp = opendir(src_dir)) == NULL) {
        fprintf(stderr, "Trouble opening directory '%s'.\n", src_dir);
        return FAIL;
    }

    // Now we are ready to check the containings of the  target directory
    while ((dirent_src = readdir(src_dp)) != NULL) {
        if (strcmp(".", dirent_src->d_name) == 0 || strcmp("..", dirent_src->d_name) == 0)
            continue;

        // reconstruct the suppossed path as it would be in the original directory
        char *src_path = calloc(strlen(src_dir) + 1 + strlen(dirent_src->d_name) + 1, sizeof(char));
        strcpy(src_path, src_dir);
        strcat(src_path, "/");
        strcat(src_path, dirent_src->d_name);
        
        printf("%s\n%s\n\n", trg_path, src_path);
        // Now check if the target exists in the src directory after traversing at some depth
        if (strcmp(src_path, trg_path) == 0)
            return SUCC;
        
        if (is_dir(src_path)) 
            if (detect_cycle(src_path, trg_path) == SUCC)
                return SUCC;

        // free the allocated memory
        free(src_path);
        src_path = NULL;
    }
    
    return FAIL;
}
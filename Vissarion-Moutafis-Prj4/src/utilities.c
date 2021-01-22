#include "Utilities.h"

// return the mode, in failure return something < 0
mode_t path_get_mode(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        exit(1);
    }
    return buf.st_mode;
}

// return 1 if the path is a directory, else 0
int is_dir(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        exit(1);
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
    
    if (manage_links)
        return (in_buf.st_mode & S_IFMT) == (out_buf.st_mode & S_IFMT) &&  // check if the files are of the same type
                   in_buf.st_size == out_buf.st_size &&  // check if the files are of the same size
                   in_buf.st_mtime <= out_buf.st_mtime &&
                   in_buf.st_nlink == out_buf.st_nlink
                   ? 0 : 1;  // check if the src is modified later than it's relative copy
    
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
        exit(1);
    }
    return S_ISLNK(buf.st_mode);
}

int create_symlink(char *src_path, char *trg_path, char *trg_root_path) {
    // create the symlink, by using the symlink string according to the src_path symlink

    // in case the symlink name is already taken then overwrite it
    unlink(trg_path);
    
    char dest_file[BUFSIZ];
    memset(dest_file, 0, BUFSIZ);
    int bytes_read = readlink(src_path, dest_file, BUFSIZ);
    if (bytes_read == -1) {
        if (verbose)
            fprintf(stderr, "Link '%s' is dangling, we skip the creation\n", src_path);
        return FILE_CP_SUCC;
    }

    // try to create the symlink
    if (symlink(dest_file, trg_path) == 0){
        items_copied += 1;
        struct stat buf;
        lstat(trg_path, &buf);
        bytes_copied += buf.st_size;
        if (verbose)
            print_copy_element(trg_path, trg_root_path);

        return FILE_CP_SUCC;
    }
    // return failure
    return FILE_CP_FAIL;
}

// return 1 if the path is symbolic link, else 0
int  number_of_links(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        exit(1);
    }
    return buf.st_nlink;
}

Pointer create_inode_pair(ino_t ino, char *path, int deep_copy) {
    inodePair p = calloc(1, sizeof(*p));
    p->src_ino = ino;
    if (deep_copy) {
        p->trg_path = calloc(strlen(path) + 1, sizeof(char));
        strcpy(p->trg_path, path);
    } else 
        p->trg_path = path;

    return (Pointer) p;
}

int cmp_inode_pair(void *a, void *b) {
    inodePair p1 = (inodePair)a;
    inodePair p2 = (inodePair)b;

    return p1->src_ino - p2->src_ino;
}

size_t hash_inode_pair(void *_p) {
    inodePair p = (inodePair)_p;
    // Knuth's multiplicative method. Check link below:
    // (https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key)
    return (((size_t)p->src_ino) * 2654435761) % (((size_t)2)<<32);
}

void destroy_inode_pair(void *a) {
    inodePair p = (inodePair)a;
    free(p->trg_path);
    free(a);
}

int create_link(char *src_path, char *trg_path, char *trg_root_path, HT inode_table) {
    
    // The inode table is a table of {src inode number (int), target path to the specific inode}
    struct stat buf_src;
    if (lstat(src_path, &buf_src) < 0) {
        perror("lstat at create link");
        exit(1);
    }
    
    ino_t src_ino = buf_src.st_ino;
    inodePair dummy = create_inode_pair(src_ino, NULL, 0);
    Pointer pair;
    if (ht_contains(inode_table, dummy, &pair) == true) {
        inodePair p = (inodePair)pair;
        // at this point there is already a record of the inode 
        // and we have to create a new link to the respective inode in target file system
        if (link(p->trg_path, trg_path) != 0) {
            unlink(trg_path);
            if (link(p->trg_path, trg_path) != 0) {
                perror("creating hard link in create_link()");
                exit(1);
            }
        }
        free(dummy);
        items_copied += 1;
        struct stat buf;
        lstat(trg_path, &buf);
        bytes_copied += buf.st_size;
        if (verbose)
            print_copy_element(trg_path, trg_root_path);
        return FILE_CP_SUCC;
    }

    // at this point we have to create an entry to the hastable
    ht_insert(inode_table, create_inode_pair(src_ino, trg_path, 1));

    // since no link created, return failure
    free(dummy);
    return FILE_CP_FAIL;
}


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
        exit(1);
    }
    return opendir(path);
}

void delete_file(char *path, char *trg_root_path) {
    if(unlink(path) != 0) {
        char b[BUFSIZ];
        sprintf(b, "Unlinking '%s'", path);
        perror(b);
        exit(1);
    }
    if (verbose)
        print_remove_element(path, trg_root_path);
}

void delete_dir(char *path, char *trg_root_path) {
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

        delete_element(element_path, trg_root_path);

        // free the allocated memory
        free(element_path);
        element_path = NULL;
    }

    closedir(dp);
    rmdir(path);

    if (verbose)
        print_remove_element(path, trg_root_path);
}

void delete_element(char *path, char *trg_root_path) {
    items_deleted += 1;
    if (is_dir(path))
        delete_dir(path, trg_root_path);
    else
        delete_file(path, trg_root_path);
}

int check_deleted(char *src_dir, char *trg_dir, char *trg_root_path) {
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
            delete_element(out_path, trg_root_path);
        }

        // free the allocated memory
        free(src_path);
        src_path = NULL;
        free(out_path);
        out_path = NULL;
    }
    closedir(trg_dp);
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
        
        // Now check if the target exists in the src directory after traversing at some depth
        if (strcmp(src_path, trg_path) == 0){
            free(src_path);
            closedir(src_dp);
            return SUCC;
        }
        
        if (is_dir(src_path)) 
            if (detect_cycle(src_path, trg_path) == SUCC){
                free(src_path);
                closedir(src_dp);
                return SUCC;
            }
        // free the allocated memory
        free(src_path);
        src_path = NULL;
    }
    closedir(src_dp);
    return FAIL;
}

void print_copy_element(char *path, char *trg_root_path) {
    int start = strlen(trg_root_path);
    printf("%s\n", path+start+1);
}

void print_remove_element(char *path, char *trg_root_path) {
    int start = strlen(trg_root_path);
    printf("removing '%s'...\n", path + start + 1);
}

// return 1 if element exists, 0 otherwise
int element_exists(char *path) {
    struct stat buf;
    if (lstat(path, &buf) == -1)
        return 0;
    
    return 1;
}

int is_same_type(char *path1, char *path2) {
    struct stat buf1, buf2;
    if (lstat(path1, &buf1) == -1) return 0;
    if (lstat(path2, &buf2) == -1) return 0;
    return (buf1.st_mode & S_IFMT) == (buf2.st_mode & S_IFMT);
}

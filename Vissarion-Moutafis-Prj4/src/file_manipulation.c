#include "FileManipulation.h"

int copy_symlinks = 0;

// return the mode, in failure return something < 0
static mode_t path_get_mode(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat"); 
        return -1;
    }
    return buf.st_mode;
}

//return 1 if the path is a directory, else 0
static int is_dir(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        return 0;
    }
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

// return 1 if files differ, 0 if they don't 
static int files_diff(char *in_path, char *out_path) {
    struct stat out_buf, in_buf;
    memset(&in_buf, 0, sizeof(in_buf));
    if (lstat(in_path, &in_buf) < 0) {
        return 1;
    }
    memset(&out_buf, 0, sizeof(out_buf));
    if (lstat(out_path, &out_buf) < 0) {
        return 1;
    }

    return (in_buf.st_mode & S_IFMT) == (out_buf.st_mode & S_IFMT) &&   // check if the files are of the same type
            in_buf.st_size == out_buf.st_size                      &&   // check if the files are of the same size    
            in_buf.st_mtime <= out_buf.st_mtime ? 0 : 1;                         // check if the src is modified later than it's relative copy
}

// return 1 if the path is symbolic link, else 0
static int is_sym(char *path) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        return 0;
    }
    return (buf.st_mode & S_IFMT) == S_IFLNK;
}

// small procedure that takes as input the open fd's 
// of one input and one output file and copies the containings of 
// the first to the second. Returns 1 in success, otherwise 0
static int copy_file(int in_fd, int out_fd, int BUFFSIZE) {
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

int clean_copy_file(char *in_path, char *out_path, int BUFFSIZE) {
    int in, out;
    
    if ((in = open(in_path, O_RDONLY)) == -1) {
        // the reading of the input file failed print error message
        fprintf(stderr, "%s: Cannot open input file.\n", in_path);
        return FILE_CP_FAIL;
    }
    if ((out = open(out_path, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
        fprintf(stderr, "%s: Cannot open output file.\n", in_path);
        return FILE_CP_FAIL;
    }
    // copy the mode to the out
    if (chmod(out_path, path_get_mode(in_path)) < 0) {
        perror("chmod");
        return FILE_CP_FAIL;
    }

    // copy the file
    return copy_file(in, out, BUFFSIZE);
}

// create a dir
static DIR* create_dir(char *path) {
    if (mkdir(path, 0755) < 0) {
        fprintf(stderr, "Cannot create dir '%s'.\n", path);
    }
    return opendir(path);
}

int copy_dir(char *in_path, char *out_path, int BUFFSIZE) {
    DIR *in_dp, *out_dp;
    struct dirent *dirent_in;

    // first open the directory 
    if ((in_dp = opendir(in_path)) == NULL) {
        fprintf(stderr, "Trouble opening directory '%s'.\n", in_path);
        return DIR_CP_FAIL;
    }
    // Now try to open the target dir
    if ((out_dp = opendir(out_path)) == NULL) {
        // at this case the dir does not exist so we must create it
        out_dp = create_dir(out_path);
        if (!out_dp) {
            fprintf(stderr, "Trouble creating directory '%s'.\n", out_path);
            return DIR_CP_FAIL;
        }
    }

    // At this point we must copy the mode of the in dir to the out dir
    // copy the mode to the out
    if (chmod(out_path, path_get_mode(in_path)) < 0) {
        perror("chmod");
        return DIR_CP_FAIL;
    }

    // Now we are ready to copy the containings of the directory
    while ((dirent_in = readdir(in_dp)) != NULL) {
        if (strcmp(".", dirent_in->d_name) == 0 || strcmp("..", dirent_in->d_name) == 0)
            continue;
        char *element_path = calloc(strlen(in_path) + 1 + strlen(dirent_in->d_name) + 1, sizeof(char));
        char *new_out_path = calloc(strlen(out_path) + 1 + strlen(dirent_in->d_name) + 1, sizeof(char));

        // now we have to create the new in_path 
        strcpy(element_path, in_path);
        strcat(element_path, "/");
        strcat(element_path, dirent_in->d_name);

        // now we have to create the new out_path
        strcpy(new_out_path, out_path);
        strcat(new_out_path, "/");
        strcat(new_out_path, dirent_in->d_name);
        printf("Trying to create '%s' from '%s'.\n", new_out_path, element_path);
        // // copy every element of the directory
        if (copy_element(element_path, new_out_path) == FAIL)
            return DIR_CP_FAIL;

        // free the allocated memory
        free(element_path);
        element_path = NULL;
    }
    
    return DIR_CP_SUCC;
}


// The in path is the src file/dir/sym path. 
// The out_path is the new path that the last notion
//  is the name that you give to the deep copy you make.  
int copy_element(char *in_path, char *out_path) {
    // logic:
    // if the in_path its a file:
    //      - check if it already exist in target
    //          - if exists check if it is altered and if it is then rewrite it
    //      - if it doesn't exist then just write it
    // if the path is a dir then:
    //      - if it does not exist then try to copy its containings
    // if the path is a symlink then just make a pure copy (only if the proper flag is set)

    if (is_dir(in_path)) {
        return copy_dir(in_path, out_path, CP_BUFFSIZE);
    } else {
        // if it is a symlink and we haven't set the flag then just skip it
        if (is_sym(in_path) && !copy_symLinks)
            return FILE_CP_SUCC;

        // in case the src and dest files are not different then there is no
        // reason to copy again
        if (files_diff(in_path, out_path) == 0)
            return FILE_CP_SUCC;


        return clean_copy_file(in_path, out_path, CP_BUFFSIZE);
    }

    return FAIL;
}


int main(int argc, char *argv[]) {
    char *in = argv[1];
    char *target_media = argv[2];

    copy_element(in, target_media);

    return 0;
}
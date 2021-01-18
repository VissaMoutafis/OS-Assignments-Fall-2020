#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>

#include "FileManipulation.h"
#include "Utilities.h"

// flag that determines if we should handle links or not (1 or 0 respectively)
int manage_links = 0;
// flag that determines whether to check for deleted files in the src dir that don't exist in out dir.
int check_for_deleted = 0;
// flag that defines the verbosity level of the output
int verbose = 0;

// total bytes copied
unsigned int bytes_copied = 0;
// total number of new elements copied
unsigned int items_copied = 0;
// total number of elements checked for copying
unsigned int items_detected = 0;
//total time the whole copy process lasted
float total_time = 0.0;

// flag that is used to check if a directory changed after recursive calls to its contents
short dir_changed = 0;

// this is the map of {key:item} pairs that 
// "key = src file system file inode" and 
// "item = trg file system respective path after first copy"
// it will be used to properly detect and copy hard links
HT inodes_map = NULL;

// function that copies a file ( with name in_path) to the file with name out_path. 
// If the latter doesn't exist create it.
int clean_copy_file(char *in_path, char *out_path, int BUFFSIZE, char *out_root_path) {
    int in, out;
    // first we must check if the in_path is a hard link and if it is then we must 
    // decide if it is the first one. To that case we copy it. If it is already copied then just 
    // use link to make a hard link in the new media
    
    // if it is a symlink and we don't want to consider the links then just skip it
    if (manage_links && is_sym(in_path))
        return create_symlink(in_path, out_path, out_root_path);
        
    // check for hard links and if they exist try to create one. 
    // If the respective inode in trg dir's file system is not yet created, then 
    // return FILE_CP_FAIL and proceeed to the normal copy of the file
    if (manage_links && number_of_links(in_path) > 1 && create_link(in_path, out_path, out_root_path, inodes_map) == FILE_CP_SUCC)
        return FILE_CP_SUCC;

    // open input file
    if ((in = open(in_path, O_RDONLY)) == -1) {
        // the reading of the input file failed print error message
        fprintf(stderr, "%s: Cannot open input file.\n", in_path);
        return FILE_CP_FAIL;
    }

    // open out put for writting, if it exists then truncate and rewrite
    if ((out = open(out_path, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
        fprintf(stderr, "%s: Cannot open output file.\n", in_path);
        return FILE_CP_FAIL;
    }

    // copy the mode of in at the out's inode
    if (chmod(out_path, path_get_mode(in_path)) < 0) {
        perror("chmod");
        return FILE_CP_FAIL;
    }

    // copy the contents of input file to out put and return 
    int ret = copy_file(in, out, BUFFSIZE);
    if (ret == FILE_CP_SUCC) {
        // add the size of the file to the copied file
        struct stat buf;
        memset(&buf, 0, sizeof(buf));
        if (lstat(out_path, &buf) < 0) {
            fprintf(stderr, "%s: Cannot open input file to acquire size.\n",
                    out_path);
            return FILE_CP_FAIL;
        }
        bytes_copied += buf.st_size;
        // and increase the copied files counter
        items_copied += 1;
    }
    if (verbose)
        print_copy_element(out_path, out_root_path);

    return ret;
}


// function that copies a whole directory recursively (deep copy the whole dir to out path)
int copy_dir(char *in_path, char *out_path, int BUFFSIZE, char *out_root_path) {
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
            closedir(in_dp);
            return DIR_CP_FAIL;
        }
        // add the size of the dir to the bytes copied counter
        struct stat buf;
        memset(&buf, 0, sizeof(buf));
        if (lstat(out_path, &buf) < 0) {
            fprintf(stderr, "%s: Cannot open input file to acquire size.\n",
                    out_path);
            exit(1);
        }
        bytes_copied += buf.st_size;
        items_copied += 1;
        if (verbose)
            print_copy_element(out_path, out_root_path);
    }

    // At this point we must copy the mode of the in dir to the out dir
    // copy the mode to the out
    if (chmod(out_path, path_get_mode(in_path)) < 0) {
        perror("chmod");
        exit(1);
    }
    // set the flag
    dir_changed = 0;

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

        // // copy every element of the directory
        if (copy_element(element_path, new_out_path, out_root_path) == FAIL) {
            closedir(in_dp);
            closedir(out_dp);
            return DIR_CP_FAIL;
        }

        // free the allocated memory
        free(element_path);
        element_path = NULL;
        free(new_out_path);
        new_out_path = NULL;
    }

    if (check_for_deleted && check_deleted(in_path, out_path, out_root_path) == FAIL) {
        closedir(in_dp);
        closedir(out_dp);
        return DIR_CP_FAIL;
    }

    closedir(in_dp);
    closedir(out_dp);

    return DIR_CP_SUCC;
}

// The in path is the src file/dir/symlink's path. 
// The out_path is the new pathname that you give to the deep copy you make.
// The out root path, is the actual root directory that you want to copy things to
int copy_element(char *in_path, char *out_path, char *out_root_path) {
    // logic:
    // if the in_path its a file:
    //      - check if it already exist in target
    //          - if exists check if it is altered and if it is then rewrite it
    //      - if it doesn't exist then just write it
    // if the path is a dir then:
    //      - if it does not exist then try to copy its containings
    // if the path is a symlink then just make a pure copy (only if the proper flag is set)
    
    //every time we copy an element we increase the detected elements counts
    items_detected += 1;

    if (is_dir(in_path)) {
        return copy_dir(in_path, out_path, CP_BUFFSIZE, out_root_path);
    } else {
        // in case the src and dest files are not different then there is no
        // reason to copy again
        if (files_diff(in_path, out_path) == 0)
            return FILE_CP_SUCC;
        // printf("copy %s\n", in_path);
        return clean_copy_file(in_path, out_path, CP_BUFFSIZE, out_root_path);
    }

    return FAIL;
}

void print_usage(void) {
    fprintf(stderr, "\nUsage:\n\t ~$ quic [-v] [-l] [-d] origin_dir target_dir\n"
    "Flags must be given all together, either in the beginning or the end of\n"
    "the command and\ndo not interfere between origin and target paths.\n");
}

char **set_args(int argc, char *argv[],  int min_args) {
    if (argc-1 < min_args) {
        fprintf(stderr, "You have to provide at least the origin and dest directory");
        print_usage();
        exit(1);
    }

    char **args = calloc(2, sizeof(char *));

    // we already know that the origin and target path must be given in a consecutive way
    // i.e. origin -l target is not permitted
    // the flags will be given consecutively as well

    // flag that notes how many path arguments we have yet encountered
    short int path_files = 0; 
    for (int i = 1; i < argc; i++) {
        // if this is one of the flag arguments
        if (strstr(argv[i], "-") != NULL) {
            // error checking
            if (path_files % 2) {
                free(args);
                fprintf(stderr, "\nError: Flags should not be between compulsory args.\n");
                print_usage();
                exit(1);
            }

            // now we have to check if it is one of the available flags
            if (!strcmp(argv[i], "-l"))
                manage_links = 1;
            else if (!strcmp(argv[i], "-v"))
                verbose = 1;
            else if (!strcmp(argv[i], "-d"))
                check_for_deleted = 1;
            else {
                free(args);
                fprintf(stderr, "\nError: Uknown flag.\n");
                print_usage();
                exit(1);
            }
        } else {
            path_files++;
            args[path_files-1] = argv[i];
        }
    }

    if (path_files < 2) {
        free(args);
        fprintf(stderr, "\nError: Argument(s) missing.\n");
        print_usage();
        exit(1);
    }

    return args;
}

void print_statistics(void) {
    printf("\nThere are %u files/directories in the hierarchy.\n", items_detected);
    printf("Number of entities copied is %u.\n", items_copied);
    printf("Copied %u bytes in %.3f seconds at %.2f bytes/sec.\n", bytes_copied, total_time, bytes_copied ? ((float)bytes_copied)/total_time:0.0 );

}

void set_up_trg_dir(char *target) {
    struct stat buf;
    int exists =  !(lstat(target, &buf) < 0 || buf.st_ino == 0);
    
    if (!exists) {
        DIR *new_dir = create_dir(target);
        items_copied += 1;
        lstat(target, &buf);
        bytes_copied += buf.st_size;
        closedir(new_dir);
        if (verbose) {
            printf("Target directory does not exist.\nCreated directory %s.\n", target);
            printf("./\n");
        }
    }
}

int main(int argc, char *argv[]) {
    
    manage_links = 0;
    check_for_deleted = 0;
    verbose = 0;
    bytes_copied = 0;
    items_copied = 0;
    items_detected = 0;
    total_time = 0.0;
    dir_changed = 0;

    // return the origin and target directory. Also set the proper flags during searching the args.
    char** args = set_args(argc, argv, 2);
    char *src = realpath(args[0], NULL);
    char *target = realpath(args[1], NULL);

    double t1, t2;
    struct tms tb1, tb2;
    double ticspersec;

    inodes_map = ht_create(cmp_inode_pair, hash_inode_pair, destroy_inode_pair);

    ticspersec = (double)sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);

    // check for cyclic path (if the target does not exists then there is no need for checking cycles)
    if (src && (!target || detect_cycle(src, target) == FAIL)) {
        if (!target){
            set_up_trg_dir(args[1]);
            target = realpath(args[1], NULL);
        }
        int ret = copy_element(src, target, target);
        if (ret == FAIL) {
            fprintf(stderr, "Failed to execute copy command.\n");
        }
    } else {
        fprintf(stderr, "Error: Cannot copy path '%s' in '%s'\n", args[0], args[1]);
        if (args)
            free(args);
        if (src)
            free(src);
        if (target)
            free(target);
        ht_destroy(inodes_map);
        exit(1);
    }

    t2 = times(&tb2);
    total_time = (float)(t2-t1)/(float)ticspersec;
    // print statistics
    if (verbose)
        print_statistics();

    free(args);
    free(src);
    free(target);
    ht_destroy(inodes_map);
    return 0;
}
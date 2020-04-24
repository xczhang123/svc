#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "svc.h"

/* Define and initialize all the structure we need to use

    active_branch: struct Branch

    struct Branch:
        committed: struct Commit* (double-linked list)
        cursor: struct Commit (current commit)

    struct Commit:
        commit_id: char* (6 digits)
        message: char*
        added_files: struct File* (array/single-linked list)
        removed_files: struct File* (array/single-linked list)
        files_hash: int* (array/single_linked_list)

    struct File:
        file: FILE*
        file_path: char*
        file_hash: int

*/
// void *svc_init(void) {
//     // TODO: Implement
//     return NULL;
// }

// void cleanup(void *helper) {
//     // TODO: Implement
// }

int hash_file(void *helper, char *file_path) {

    free(helper);

    //file_path is NULL
    if (file_path == NULL) {
        return -1;
    }

    FILE *fp;
    //File not found
    if ((fp = fopen(file_path, "r")) == NULL) {
        return -2;
    }

//     function file_hash(file_name):
        // file_contents = read(file_name)
        // file_length = num_bytes(file_contents)
        // hash = 0
        // for unsigned byte in file_name:
        // hash = (hash + byte) % 1000
        // for unsigned byte in file_contents:
        // hash = (hash + byte) % 2000000000
        // return hash
    //Get the file length
    long file_length = ftell(fp);
    
    //Read the file contents
    fseek(fp, 0, SEEK_END);
    fseek(fp, 0, SEEK_SET);
    char *file_contents = malloc(file_length *sizeof(char));
    fread(file_contents, sizeof(char), file_length, fp);
    fclose(fp);

    //Get the file name
    char *file_name = basename(file_path);

    //Hash number
    long hash = 0;

    //Compute file name hash
    for (unsigned int i = 0; i < strlen(file_name); i++) {
        hash = (hash + (unsigned char)file_name[i]) % 1000;
    }

    //Compute file contents hash
    for (long i = 0; i < file_length; i++) {
        hash = (hash + (unsigned char)file_contents[i]) % 2000000000;
    }

    free(file_contents);
    return hash;
}

// char *svc_commit(void *helper, char *message) {
//     // TODO: Implement
//     return NULL;
// }

// void *get_commit(void *helper, char *commit_id) {
//     // TODO: Implement
//     return NULL;
// }

// char **get_prev_commits(void *helper, void *commit, int *n_prev) {
//     // TODO: Implement
//     return NULL;
// }

// void print_commit(void *helper, char *commit_id) {
//     // TODO: Implement
// }

// int svc_branch(void *helper, char *branch_name) {
//     // TODO: Implement
//     return 0;
// }

// int svc_checkout(void *helper, char *branch_name) {
//     // TODO: Implement
//     return 0;
// }

// char **list_branches(void *helper, int *n_branches) {
//     // TODO: Implement
//     return NULL;
// }

// int svc_add(void *helper, char *file_name) {
//     // TODO: Implement
//     return 0;
// }

// int svc_rm(void *helper, char *file_name) {
//     // TODO: Implement
//     return 0;
// }

// int svc_reset(void *helper, char *commit_id) {
//     // TODO: Implement
//     return 0;
// }

// char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
//     // TODO: Implement
//     return NULL;
// }


#ifndef svc_h
#define svc_h

#include <stdlib.h>
#define DEFAULT 0
#define ADDED 1
#define REMOVED 2
#define CHANGED 3

/** 
 * Struct to represent files in the SVC
 * 
 * state: DEFAULT, ADDED, DELETED, CHANGED
 * file_path: path to the file
 * file_content: content of the file
 * hash: hash value of the file
*/
typedef struct file {
    int state;
    char *file_path;
    char *file_content;
    int hash;
} file_t;

/**
 * Struct to represent the staging before committing to SVC
 * 
 * is_commited: if it has been commited
 * staged_file: file staged before committed
 * size: size of the staged file
*/
typedef struct stage {
    int is_commited;
    file_t *staged_file;
    size_t size;
} stage_t;

/**
 * Struct to represent one commit object in SVC
 * 
 * commit_id: identifier for a commit
 * message: commit message
 * committed_file: committed file(s)
 * prev: previous commit (at most 2)
 * n_prev: number of previous commit
 * size: number of commited files
*/
typedef struct commit {
    char commit_id[6];
    char *message;
    file_t *added_file;
    file_t *removed_file;
    file_t *changed_file;
    file_t *commited_file;
    struct commit *prev[2];
    size_t n_prev;
    size_t size;
} commit_t;

/**
 * Struct to represent a branch
 * 
 * name: name of the branch
 * last_commit_index: index of the last commit
 * commit: all commits in the current branch
 * size: number of commits
*/ 
typedef struct branch {
    char *name;
    size_t last_commit_index;
    commit_t *commit;
    size_t size;
} branch_t;


/** 
 * Struct to represent the whole SVC system
 * 
 * head: current branch
 * branch: all branches in the system
 * size: number of branches
 * stage: containing files before commit
*/
typedef struct svc {
    branch_t *head;
    branch_t *branch;
    size_t size;
    stage_t *stage;
} svc_t;


typedef struct resolution {
    // NOTE: DO NOT MODIFY THIS STRUCT
    char *file_name;
    char *resolved_file;
} resolution;

void *svc_init(void);

void cleanup(void *helper);

int hash_file(void *helper, char *file_path);

char *svc_commit(void *helper, char *message);

void *get_commit(void *helper, char *commit_id);

char **get_prev_commits(void *helper, void *commit, int *n_prev);

void print_commit(void *helper, char *commit_id);

int svc_branch(void *helper, char *branch_name);

int svc_checkout(void *helper, char *branch_name);

char **list_branches(void *helper, int *n_branches);

int svc_add(void *helper, char *file_name);

int svc_rm(void *helper, char *file_name);

int svc_reset(void *helper, char *commit_id);

char *svc_merge(void *helper, char *branch_name, resolution *resolutions, int n_resolutions);

#endif


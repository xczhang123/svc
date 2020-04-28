#ifndef commit_t_dyn_array_h
#define commit_t_dyn_array_h

#include "file_t_dyn_array.h"
#include "svc.h"

/**
 * Struct to represent one commit object in SVC
 * 
 * commit_id: identifier for a commit
 * message: commit message
 * committed_file: committed file(s)
 * prev: previous commit (at most 2)
 * n_prev: number of previous commit
 * committed_size: number of commited files
*/

typedef struct commit {
    char commit_id[6];
    char *message;
    struct file_t_dyn_array *commited_file;
    struct commit *prev[2];
    size_t n_prev;
} commit_t;

struct commit_t_dyn_array {
    int capacity;
    int size;
    int last_commit_index;
    commit_t **commit;
};

/**
 * Struct to represent the staging before committing to SVC
 * 
 * is_commited: if it has been commited
 * staged_file: file staged before committed
 * size: size of the staged file
*/
typedef struct stage {
    int not_changed;
    struct file_t_dyn_array *tracked_file;
} stage_t;

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
    struct commit_t_dyn_array *commit;
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
    branch_t **branch;
    int size;
    stage_t *stage;
} svc_t;

void commit_t_dyn_array_resize(struct commit_t_dyn_array *d);

struct commit_t_dyn_array* commit_t_dyn_array_init();

void commit_t_dyn_array_add(struct commit_t_dyn_array *dyn, stage_t *stage, char *message, int n_prev, commit_t *prev[2]);

void commit_t_dyn_array_add_commit(struct commit_t_dyn_array *dyn, commit_t *commit);

commit_t* commit_t_dyn_array_get(struct commit_t_dyn_array *dyn, int index);

void commit_t_dyn_array_free(struct commit_t_dyn_array *dyn);

#endif
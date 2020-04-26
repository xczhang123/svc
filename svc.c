#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svc.h"

//DONE
void *svc_init(void) {
    // TODO: Implement
    svc_t *svc = calloc(1, sizeof(svc_t));
    return svc;
}

void cleanup(void *helper) {
    //Get the SVC system 
    svc_t *svc = (struct svc*)helper;

    for (size_t i = 0; i < svc->size; i++) {
        branch_t *branch = &svc->branch[i];
        
        for (size_t j = 0; j < branch->size; j++) {
            commit_t *commit = &branch->commit[i];

            for (size_t z = 0; z < commit->size; z++) {
                file_t *file = &commit->commited_file[z];
                if (file->file_content != NULL) {
                    free(file->file_content);
                    file->file_content = NULL;
                }
                if (file->file_path != NULL) {
                    free(file->file_path);
                    file->file_path = NULL;   
                }
            }
            // ******** free commit
        }
        if (branch->name != NULL) {
            free(branch->name);
            branch->name = NULL;
        }
        if (branch->commit != NULL) {
            free(branch->commit);
            branch->commit = NULL;  
        }
    }
    if (svc->branch != NULL) {
        free(svc->branch);
        svc->branch = NULL;
    }
    if (svc->stage != NULL) {
        free(svc->stage);
        svc->stage = NULL;
    }
    free(svc);
}

//DONE
int hash_file(void *helper, char *file_path) {

    //file_path is NULL
    if (file_path == NULL) {
        return -1;
    }

    FILE *fp;
    //File not found
    if ((fp = fopen(file_path, "r")) == NULL) {
        return -2;
    }

    //Get the file length && the file contents
    fseek(fp, 0, SEEK_END);
    long file_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char file_contents[file_length+1];
    file_contents[file_length] = '\0';
    fread(file_contents, sizeof(char), file_length, fp);
    fclose(fp);

    //Hash number
    int hash = 0;

    //Compute file name hash
    for (unsigned int i = 0; i < strlen(file_path); i++) {
        hash = (hash + (unsigned char)file_path[i]) % 1000;
    }

    //Compute file contents hash
    for (long i = 0; i < file_length; i++) {
        hash = (hash + (unsigned char)file_contents[i]) % 2000000000;
    }

    return hash;
}

char *svc_commit(void *helper, char *message) {
    // TODO: Implement
    stage_t *stage = (((struct svc*)helper)->stage);
    //If there are no changes since the last commit or message is NULL: return NULL
    if (stage->size == 0 || message == NULL) {
        return NULL;
    }

    //We assume there are at least one branch created
    //As a result of the svc_add() method

    // branch_t *branch = ((struct svc*)helper)->head; //Get current branch

    // //If it is the very first commit
    // if (branch->commit == NULL) {
    //     branch->commit = calloc(1, sizeof(commit_t)); //Set branch commit field
    //     branch->last_commit_index = 0; //Set branch last_commit_index field to be the first commit
    //     branch->size++; //Set branch size field
    // }

    // commit_t *last_commit = &branch->commit[branch->last_commit_index]; //Get last commit(current commit)





    return NULL;
}

//DONE
void *get_commit(void *helper, char *commit_id) {

    if (commit_id == NULL) {
        return NULL;
    }
    
    //Get the SVC system 
    svc_t *svc = (struct svc*)helper;

    //Loop through all the branches
    for (size_t i = 0; i < svc->size; i++) {
        branch_t *branch = &svc->branch[i];
        //Loop through all the commits in one branch
        for (size_t j = 0; j < branch->size; j++) {
            commit_t *commit = &branch->commit[j];
            if (strcmp(commit->commit_id, commit_id) == 0) {
                return commit;
            }
        }
    }

    return NULL;
}

//DONE
char **get_prev_commits(void *helper, void *commit, int *n_prev) {

    if (n_prev == NULL) {
        return NULL;
    }

    //Change the n_prev
    *n_prev = ((struct commit*)commit)->n_prev;

    if (*n_prev == 0) {
        return NULL;
    }

    //Assertion: n_prev must be 1 or 2
    char **commit_id = calloc(*n_prev, sizeof(*commit_id)); //array of pointers to return later
    struct commit **prev = ((struct commit*)commit)->prev; //previous commits

    if (*n_prev == 1) {
        commit_id[0] = strdup(prev[0]->commit_id);
    } else {
        commit_id[0] = strdup(prev[0]->commit_id);
        commit_id[1] = strdup(prev[1]->commit_id);
    }
    
    return commit_id;
}

void print_commit(void *helper, char *commit_id) {
    // TODO: Implement
}

//DONE
int svc_branch(void *helper, char *branch_name) {

    //If the branch name is NULL: return -1
    if (branch_name == NULL) {
        return -1;
    }

    //If branch name already exists: return -2
    for (size_t i = 0; i < ((struct svc*)helper)->size; i++) {
        char *name = ((struct svc*)helper)->branch[i].name;
        if (strcmp(branch_name, name) == 0) {
            return -2;
        }
    }

    //File name is invalid: return -1
    for (size_t i = 0; i < strlen(branch_name); i++) {
        int c = branch_name[i]; //Get the ASCII representation of character
        /* 
            65-90 <-> 'A'-'Z'
            97-122 <-> 'a'-'z'
            48-57 <-> '0'-'9'
            95 <-> '_'
            47 <-> '/'
            45 <-> '-'
        */
        if ( !( (c >= 97 && c <= 122) || 
                (c >= 65 && c <= 90) || 
                (c >= 48 && c <= 57) ||
              (c == 95) || (c == 47) || (c == 45) ) ) {
            return -1;
        }

    }

    //If there are uncommitted changes: return -3
    if (((struct svc*)helper)->stage->is_commited == 0) {
        return -3;
    }

    //If successful: return 0

    //Get the SVC system 
    svc_t *svc = (struct svc*)helper;
    //Allocate space for newly created branch
    svc->branch = realloc(svc->branch, (svc->size+1)*sizeof(branch_t));

    branch_t *current_branch = svc->head; //Get the current branch
    branch_t *new_branch = &svc->branch[svc->size]; //Get the newly created branch
    svc->size++; //svc number of branches increments

    //Set field for the new_branch
    //Copy everything from the current_branch except that we change the name field
    memcpy(new_branch, current_branch, sizeof(*current_branch));
    new_branch->name = strdup(branch_name); // Set the name field

    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    // TODO: Implement
    return 0;
}

char **list_branches(void *helper, int *n_branches) {
    // TODO: Implement
    return NULL;
}

//Partially DONE: create branch!!!
int svc_add(void *helper, char *file_name) {

    svc_t *svc = ((struct svc*)helper);

    //If it is the first time we call svc_add, create branch
    if (svc->size == 0) {
        svc->branch = calloc(1, sizeof(branch_t));
        svc->head = &svc->branch[0];
        svc->size++;
        svc->stage = calloc(1, sizeof(stage_t));

        svc->head->name = strdup("master"); //We only set the name of the branch here
    }

    //If the file name is NULL: return -1
    if (file_name == NULL) {
        return -1;
    }
    
    stage_t *stage = ((struct svc*)helper)->stage;

    //If the file name is already under version control: return -2
    for (size_t i = 0; i < stage->size; i++) {
        file_t *file = &stage->staged_file[i];
        if (strcmp(file->file_path, file_name) == 0 && file->state != REMOVED) {
            return -2;
        }
    }

    //If the file does not exists: return -3
    FILE* fp;
    if ((fp=fopen(file_name, "r")) == NULL) {
        return -3;
    }

    //Else, add it to the SVC system
    file_t *new_file = calloc(1, sizeof(file_t));

    //Get the file content
    fseek(fp, 0, SEEK_END);
    long file_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char file_contents[file_length+1];
    file_contents[file_length] = '\0';
    fread(file_contents, sizeof(char), file_length, fp);
    fclose(fp);

    new_file->file_content = malloc(sizeof(char)*(file_length+1)); //Realloc file_content field
    memcpy(new_file->file_content, file_contents, file_length+1); //Set file_content field
    new_file->file_path = strdup(file_name); //Set file_path field
    new_file->hash = hash_file(helper, file_name); //Set hash field
    new_file->state = ADDED; //Set state field

    //Store new_file in the svc system stage field

    stage->staged_file = realloc(stage->staged_file, sizeof(file_t)*(stage->size+1));//Realloc staged_file field
    memcpy(&stage->staged_file[stage->size++], new_file, sizeof(file_t)); //Set the size and staged_file field
    stage->is_commited = 0; //Set the is_commited field to False


    free(new_file);
    return hash_file(helper, file_name); //return hash_value
}

//DONE
int svc_rm(void *helper, char *file_name) {
    //If the file name is NULL: return -1
    if (file_name == NULL) {
        return -1;
    }

    stage_t *stage = ((struct svc*)helper)->stage;

    //If the file with the given name is not being tracked
    if (stage == NULL) {
        return -2;
    }

    file_t *file_to_be_deleted = NULL;

    for (size_t i = 0; i < stage->size; i++) {
        file_t *file = &stage->staged_file[i];
        if (strcmp(file->file_path, file_name) == 0 && file->state == ADDED) {
            file->state = REMOVED;
            file_to_be_deleted = file;
            break;
        }
    }

    if (file_to_be_deleted == NULL) {
        return -2;
    }

    //If the file does not exists: return -3
    FILE* fp;
    if ((fp=fopen(file_name, "r")) == NULL) {
        return -3;
    }

    return file_to_be_deleted->hash;
}

int svc_reset(void *helper, char *commit_id) {
    // TODO: Implement
    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
    // TODO: Implement
    return NULL;
}


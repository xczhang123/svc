#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svc.h"

//DONE
void *svc_init(void) {
    svc_t *svc = calloc(1, sizeof(svc_t));
    svc->branch = calloc(1, sizeof(branch_t));
    svc->head = &svc->branch[0];
    svc->size++;
    svc->stage = calloc(1, sizeof(stage_t));

    stage_t *stage = svc->stage;
    stage->tracked_file = file_t_dyn_array_init();
    stage->is_commited = 1; //No change has been made

    branch_t *branch = svc->head;
    branch->commit = commit_t_dyn_array_init();
    branch->name = strdup("master");

    return svc;
}

//PARTIALLY DONE: free commit fields
void cleanup(void *helper) {
    //Get the SVC system 
    svc_t *svc = (struct svc*)helper;

    for (int i = 0; i < svc->size; i++) {
        branch_t *branch = &svc->branch[i];
        free(branch->name);
        branch->name = NULL;

        commit_t_dyn_array_free(branch->commit);
        branch->commit = NULL;
    }
    free(svc->branch);
    svc->branch = NULL;

    file_t_dyn_array_free(svc->stage->tracked_file);
    svc->stage->tracked_file = NULL;
    
    free(svc->stage);
    svc->stage = NULL;
    
    free(svc);
}

//DONE
int hash_file(void *helper, char *file_path) {

    (void)helper;

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
    fread(file_contents, sizeof(char), file_length, fp);
    file_contents[file_length] = '\0';
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


void set_commit_id(commit_t *commit) {
    int id = 0;

    for (size_t i = 0; i < strlen(commit->message); i++) {
        id = (id + (unsigned char)commit->message[i]) % 1000;
    }
    
    //Sort the committed files in alphabetical order
    for (int i = 0; i < commit->commited_file->size; i++) {
        for (int j = 0; j < commit->commited_file->size-i-1; j++) {
            char *name1 = file_t_dyn_array_get(commit->commited_file, j)->file_path;
            char *name2 = file_t_dyn_array_get(commit->commited_file, j+1)->file_path;

            if (strcmp(name1, name2) > 0) {
                char* e = strdup(name1);
                free(name1);
                name1 = strdup(name2);
                free(name2);
                name2 = strdup(e);
                free(e);
            }
        }
    }

    //for change in commit.changes in increasing alphabetical order of file_name:
    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(commit->commited_file, i);

        if (file->state == ADDED) {
            id += 376591;
        } else if(file->state == REMOVED) {
            id += 85973;
        } else if (file->state == CHANGED) {
            id += 9573681;
        }
    }

    //For unsigned byte in change.file_name
    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(commit->commited_file, i);

        for (size_t j = 0; j < strlen(file->file_path); j++) {
            id = (id * ((unsigned char)(file->file_path[j]) % 37) % 15485863 + 1);
        }
    }

    sprintf(commit->commit_id, "%06x", id);
}

char *svc_commit(void *helper, char *message) {
    
    stage_t *stage = (((struct svc*)helper)->stage);
    branch_t *branch = ((struct svc*)helper)->head;

    //First we automatically update the state of the tracked files
    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        FILE *fp;
        if ((fp=fopen(file->file_path, "r")) == NULL) {
            //User has manually deleted the file from the file system
            file_t_dyn_array_delete_index(stage->tracked_file, i);
            i--;
        } else {//We recalculate the hash value for each file
            file->previous_hash = file->hash;
            file->hash = hash_file(helper, file->file_path);
            
            fseek(fp, 0, SEEK_END);
            long file_length = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char file_contents[file_length+1];
            file_contents[file_length] = '\0';
            fread(file_contents, sizeof(char), file_length, fp);
            fclose(fp);

            file->file_content = realloc(file->file_content,sizeof(char)*(file_length+1)); //Realloc file_content field
            memcpy(file->file_content, file_contents, file_length+1);
            
            file->state = DEFAULT;
        }
    }

    //We are guaranteed we have updated all files

    //Special case: when it is the first commit
    if (branch->commit->size == 0) {
        //Mark all files as ADDED
        for (int i = 0; i < stage->tracked_file->size; i++) {
            file_t_dyn_array_get(stage->tracked_file, i)->state = ADDED;
        }
        commit_t *prev[2] = {NULL, NULL};
        commit_t_dyn_array_add(branch->commit, stage, message, 0, prev);

        commit_t *commit = commit_t_dyn_array_get(branch->commit, branch->commit->last_commit_index); //Get the last commit
        
        set_commit_id(commit);

        return commit->commit_id;
        
    }

    //     commit_t_dyn_array_add(branch->commit, stage, message, 0);

        //Set hash and value

        
        // commit_t_dyn_array_add(branch->commit, message); //Set message field and initialize committed_file field

        // commit_t *commit = commit_t_dyn_array_get(branch->commit, branch->commit->last_commit_index); //Get the last commit

        // //Copy all the files over to commit from stage
        // for (int i = 0; i < stage->tracked_file->size; i++) {
        //      file_t_dyn_array_add(commit->commited_file, 
        //                             file_t_dyn_array_get(stage->tracked_file, i));
        // }
        // commit->n_prev = 0; //Set n_prev field
        

        // commit_t_dyn_array_add(branch->commit, commit_t_dyn_array_get(stage->tracked_file, i));
        // return;
    // }








    // commit_t *commit = commit_t_dyn_array_get(branch->commit, branch->commit->last_commit_index);
    // //If there are no changes since the last commit or message is NULL: return NULL
    // int changed = 0;
    // if (commit->commited_file->size == stage->tracked_file->size) {
    //     int hash_is_the_same;
    //     for (int i = 0; i < commit->commited_file->size; i++) {
    //         hash_is_the_same = 0;
    //         for (int j = 0; j < stage->tracked_file->size; j++) {

    //             file_t *commit_file = file_t_dyn_array_get(commit->commited_file, i);
    //             file_t *stage_file = file_t_dyn_array_get(stage->tracked_file, j);

    //             //If the file path and content is the same
    //             if (strcmp(commit_file->file_path, stage_file->file_path) == 0
    //                         && commit_file->hash == stage_file->hash) {
    //                 hash_is_the_same = 1;
    //             }
    //         }
    //         if (!hash_is_the_same) {
    //             changed = 1;
    //             break;
    //         }
    //     }
    // }
    // if (!changed || message == NULL) {
    //     return NULL;
    // }


    // for (int i = 0; i < commit->commited_file->size; i++) {
       
    //     for (int j = 0; j < stage->tracked_file->size; j++) {

    //         file_t *commit_file = file_t_dyn_array_get(commit->commited_file, i);
    //         file_t *stage_file = file_t_dyn_array_get(stage->tracked_file, j);

    //         //If the file is modified
    //         if (strcmp(commit_file->file_path, stage_file->file_path) == 0
    //                     && commit_file->hash == stage_file->hash) {
                
    //         }
    //     }
    //     if (!hash_is_the_same) {
    //         changed = 1;
    //         break;
    //     }
    // }





    // stage->is_commited = 1; //There are no uncommitted changes
    // return ;
    return 0;
}

//DONE
void *get_commit(void *helper, char *commit_id) {

    if (commit_id == NULL) {
        return NULL;
    }
    
    //Get the SVC system 
    svc_t *svc = (struct svc*)helper;

    //Loop through all the branches
    for (int i = 0; i < svc->size; i++) {
        branch_t *branch = &svc->branch[i];
        //Loop through all the commits in one branch
        for (int j = 0; j < branch->commit->size; j++) {
            commit_t *commit = commit_t_dyn_array_get(branch->commit, j);
            if (strcmp(commit->commit_id, commit_id) == 0) {
                return commit;
            }
        }
    }

    return NULL;
}

//DONE
char **get_prev_commits(void *helper, void *commit, int *n_prev) {

    (void)helper;

    if (n_prev == NULL) {
        return NULL;
    }

    //If commit is NULL, or it is the very first commit, 
    //this function should set the contents of n_prev to 0 and return NULL.
    if (commit == NULL || ((struct commit*)commit)->n_prev == 0) {
        *n_prev = 0;
        return NULL;
    }

    //Store the value in the memory pointer to by n_prev
    *n_prev = ((struct commit*)commit)->n_prev;

    //Assertion: n_prev must be 1 or 2
    char **commit_id = calloc(*n_prev, sizeof(*commit_id)); //array of pointers to return later
    struct commit **prev = ((struct commit*)commit)->prev; //previous commits

    if (*n_prev == 1) {
        commit_id[0] = strdup(prev[0]->commit_id);
    } else { //*n_prev == 2
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

    //If branch name already exists: return -2
    for (int i = 0; i < ((struct svc*)helper)->size; i++) {
        // printf("%p\n",((struct svc*)helper)->branch[i]);
        char *name = ((struct svc*)helper)->branch[i].name;
        if (strcmp(branch_name, name) == 0) {
            return -2;
        }

        // puts(name);
    }

    // printf("After this %s\n", branch_name);

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
    branch_t new_branch = {0};
    new_branch.commit = commit_t_dyn_array_init();
    
    //Copy everything from the current_branch except the name field
    for (int i = 0; i < current_branch->commit->size; i++) {
        commit_t_dyn_array_add_commit(new_branch.commit, 
                commit_t_dyn_array_get(current_branch->commit, i));
    }
    new_branch.name = strdup(branch_name); // Set the name field

    svc->size++; //svc number of branches increments
    svc->branch[svc->size-1] = new_branch;

    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    // TODO: Implement
    return 0;
}

//DONE
char **list_branches(void *helper, int *n_branches) {
    
    if (n_branches == NULL) {
        return NULL;
    }

    svc_t *svc = (struct svc*)helper;
    int n = svc->size;

    *n_branches = n;

    char **branch_names = malloc(sizeof(char*)*n);
    
    for (int i = 0; i < n; i++) {
        branch_t *branch = &svc->branch[i];
        printf("%s\n", branch->name);
        branch_names[i] = branch->name;
    }
    return branch_names;
}

//DONE
int svc_add(void *helper, char *file_name) {

    svc_t *svc = ((struct svc*)helper);

    //If the file name is NULL: return -1
    if (file_name == NULL) {
        return -1;
    }
    
    stage_t *stage = svc->stage;

    //If the file name is already under version control: return -2
    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        if (strcmp(file->file_path, file_name) == 0) {
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
    new_file->previous_hash = new_file->hash; //Set previous hash
    new_file->state = DEFAULT; //Set state field

    //Store new_file in the svc system stage field
    file_t_dyn_array_add(stage->tracked_file, new_file);

    free(new_file->file_path);
    free(new_file->file_content);
    free(new_file);
    stage->is_commited = 0; //There are some uncommitted changes
    return hash_file(helper, file_name); //return hash_value
}

//DONE
int svc_rm(void *helper, char *file_name) {
    //If the file name is NULL: return -1
    if (file_name == NULL) {
        return -1;
    }

    stage_t *stage = ((struct svc*)helper)->stage;

    int file_to_be_deleted_hash = -1;

    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        if (strcmp(file->file_path, file_name) == 0) {
            file_to_be_deleted_hash = file->hash;
            file_t_dyn_array_delete_file(stage->tracked_file, file);
            break;
        }
    }

    //If the file with the given name is not being tracked
    if (file_to_be_deleted_hash == -1) {
        return -2;
    }

    //If the file does not exists: return -3
    FILE* fp;
    if ((fp=fopen(file_name, "r")) == NULL) {
        return -3;
    }

    //After we have successfully deleted the file

    stage->is_commited = 0; //There are some uncommitted changes
    return file_to_be_deleted_hash;
}

int svc_reset(void *helper, char *commit_id) {
    // TODO: Implement
    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
    // TODO: Implement
    return NULL;
}


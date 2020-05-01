#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "svc.h"

//DONE
void *svc_init(void) {
    svc_t *svc = calloc(1, sizeof(svc_t));
    svc->branch = calloc(1, sizeof(branch_t*));
    svc->branch[0] = calloc(1, sizeof(branch_t));
    svc->head = svc->branch[0];
    svc->size++;
    svc->stage = calloc(1, sizeof(stage_t));

    stage_t *stage = svc->stage;
    stage->tracked_file = file_t_dyn_array_init();
    stage->not_changed = 1;

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
        branch_t *branch = svc->branch[i];

        free(branch->name);
        branch->name = NULL;

        commit_t_dyn_array_free(branch->commit);
        branch->commit = NULL;

        free(branch);
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
  
int compare(const void* a, const void* b) {
    // puts("dadsa\n");
    file_t **file1 = ((file_t**)a);
    file_t **file2 = ((file_t**)b);   

    char *aa = (*file1)->file_path;
    char *bb = (*file2)->file_path;

    int n = strlen(aa) <= strlen(bb) ? strlen(aa) : strlen(bb);
    for (int i = 0; i < n; i++) {
        if (aa[i] != bb[i]) {
            if (tolower(aa[i]) == tolower(bb[i])) { //Compare by ASCII values
                if ((aa[i] > bb[i])) {
                    return 1;
                } else {
                    return -1;
                }
            } else {
                return strcasecmp(aa, bb);
            }
        }
    }

    //All characters are same so far
    if (strlen(aa) > strlen(bb)) {
        return 1;
    } else { //strlen(aa) < strlen(bb)
        return -1;
    }

}


void set_commit_id(commit_t *commit) {
    int id = 0;

    for (size_t i = 0; i < strlen(commit->message); i++) {
        id = (id + (unsigned char)commit->message[i]) % 1000;
    }

    //Sort file name in alphabetic order
    qsort(commit->commited_file->file, commit->commited_file->size, sizeof(file_t*), &compare); 

    //for change in commit.changes in increasing alphabetical order of file_name:
    //For unsigned byte in change.file_name
    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(commit->commited_file, i);
        printf("file is: %s, file state is %d\n",file->file_path, file->state);
        if (file->state == ADDED) {
            id += 376591;
        } else if (file->state == REMOVED) {
            id += 85973;
        } else if (file->state == CHANGED) {
            id += 9573681;
        } //else if it is default, we do nothing
        // printf("State Middle id: %d\n", id);
        if (file->state != DEFAULT) {
            for (size_t j = 0; j < strlen(file->file_path); j++) {
                id = (id * ((unsigned char)file->file_path[j] % 37)) % 15485863 + 1;
            }
        }
    }

    // printf("id is: %d\n", id);
    printf("Calculated commit id: %06x\n", id);
    snprintf(commit->commit_id, 7 , "%06x", id);
    
}

char *svc_commit(void *helper, char *message) {

    printf("This commited message is %s\n", message);
    
    stage_t *stage = (((struct svc*)helper)->stage);
    branch_t *branch = ((struct svc*)helper)->head;

    //First we automatically update the state of the tracked files
    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        FILE *fp;
        if ((fp=fopen(file->file_path, "r")) == NULL) {
            printf("Manual removal detected! file name is %s\n", file->file_path);
            //User has manually deleted the file from the file system
            file_t_dyn_array_delete_file(stage->tracked_file, file);
            stage->not_changed = 0; //there must be some changes
            i--;
        } else {//We recalculate the hash value for each file
            file->previous_hash = file->hash;
            file->hash = hash_file(helper, file->file_path);

            if (file->previous_hash != file->hash) {

                fseek(fp, 0, SEEK_END);
                long file_length = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                char file_contents[file_length+1];
                file_contents[file_length] = '\0';
                fread(file_contents, sizeof(char), file_length, fp);
                fclose(fp);

                file->file_content = realloc(file->file_content,sizeof(char)*(file_length+1)); //Realloc file_content field
                memcpy(file->file_content, file_contents, file_length+1);


                stage->not_changed = 0; //As long as we found one change, it's atomic
            }
        }
    }

    //If there is no commit and the tracked file is empty or there are no changes since the last commit
    if ((branch->commit->last_commit_index == -1 && stage->tracked_file->size == 0) || stage->not_changed == 1 || message == NULL) {
        // printf("message %s\n", message);
        return NULL;
    }

    //We are guaranteed we have updated all maually changed files

    //Special case: when it does not have previous commit
    if (branch->commit->last_commit_index == -1) {
        //If if is the first commit in the branch
        for (int i = 0; i < stage->tracked_file->size; i++) {
            file_t_dyn_array_get(stage->tracked_file, i)->state = ADDED;
        }
        commit_t *prev[2] = {NULL, NULL};
        commit_t_dyn_array_add(branch->commit, stage, message, 0, prev);

        //Undo the mark on tracked files
        for (int i = 0; i < stage->tracked_file->size; i++) {
            file_t_dyn_array_get(stage->tracked_file, i)->state = DEFAULT;
        }

        commit_t *commit = commit_t_dyn_array_get(branch->commit, branch->commit->last_commit_index); //Get the last commit
        set_commit_id(commit);

        stage->not_changed = 1;

        printf("The commit id is %s\n", commit->commit_id);

        return commit->commit_id;
    }

    //When it does have previous commit
    commit_t *last_commit = commit_t_dyn_array_get(branch->commit, branch->commit->last_commit_index);

    commit_t *prev[2] = {last_commit, NULL};

    stage_t previous = {0}; //temporary stage object to store files in previous commit
    previous.tracked_file = last_commit->commited_file;

    commit_t_dyn_array_add(branch->commit, &previous, message, 1, prev); //Add one new commit (add all files in previous commit)

    commit_t *commit = commit_t_dyn_array_get(branch->commit, branch->commit->last_commit_index); //get current commit

    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *new_file = file_t_dyn_array_get(commit->commited_file, i);
        new_file->state = DEFAULT; //reset all the new file state to not changed (since it is the same as last commit)
    }

    //Now we handle REMOVED or CHANGED files
    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *new_file = file_t_dyn_array_get(commit->commited_file, i);

        // printf("new_file %s\n", new_file->file_path);

        int found = 0;

        for (int j = 0; j < stage->tracked_file->size; j++) {
            file_t *tracked_file = file_t_dyn_array_get(stage->tracked_file, j);

            // printf("tracked_file %s\n", tracked_file->file_path);
            // printf("path is %d\n", strcmp(new_file->file_path, tracked_file->file_path));
            // printf("new_file state %d\n", new_file->state);
            // printf("%d\n", new_file->hash == tracked_file->hash );

            if (new_file->file_path != NULL && tracked_file->file_path != NULL && strcmp(new_file->file_path, tracked_file->file_path) == 0) {
                if (new_file->hash != tracked_file->hash) {
                    free(new_file->file_content);
                    new_file->file_content = strdup(tracked_file->file_content);
                    new_file->state = CHANGED;
                    new_file->previous_hash = new_file->hash;
                    new_file->hash = tracked_file->hash;
                }
                found = 1;
            }
        }
        if (!found) {
            // free(new_file->file_content);
            // new_file->file_content = NULL;
            new_file->state = REMOVED;
        }
    }

    //Now we handle ADDED (brand-new)
    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *tracked_file = file_t_dyn_array_get(stage->tracked_file, i);

        int found = 0;

        for (int j = 0; j < commit->commited_file->size; j++) {
            file_t *new_file = file_t_dyn_array_get(commit->commited_file, j);

            if (new_file->file_path != NULL && tracked_file->file_path != NULL && strcmp(new_file->file_path, tracked_file->file_path) == 0) {
                found = 1;
            }
        }
        if (!found) {
            //Add the file to the new commit
            tracked_file->state = ADDED;
            file_t_dyn_array_add(commit->commited_file, tracked_file); //Duplicate files and store in the new commit
            tracked_file->state = DEFAULT;
        }
    }

    set_commit_id(commit);
    stage->not_changed = 1;

    printf("The commit id after set is %s\n", commit->commit_id);

    return commit->commit_id;
}

//DONE
void *get_commit(void *helper, char *commit_id) {

    printf("We try to get the commit id %s\n", commit_id);

    if (commit_id == NULL) {
        return NULL;
    }
    
    //Get the SVC system 
    svc_t *svc = (struct svc*)helper;

    //Loop through all the branches
    for (int i = 0; i < svc->size; i++) {
        branch_t *branch = svc->branch[i];
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

    printf("We try to get previous commit of %s\n", ((struct commit*)commit)->commit_id);

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
        commit_id[0] = prev[0]->commit_id;
    } else { //*n_prev == 2
        commit_id[0] = prev[0]->commit_id;
        commit_id[1] = prev[1]->commit_id;
    }
    
    return commit_id;
}

void print_commit(void *helper, char *commit_id) {

    printf("We try to print the commit\n");

    if (commit_id == NULL) {
        puts("Invalid commit id");
        return;
    }

    svc_t *svc = ((struct svc*)helper);
    branch_t *branch = svc->head;

    int found = 0;
    int index = -1;

    //Find in current branch
    for (int i = 0; i < branch->commit->size; i++) {
        commit_t *commit = commit_t_dyn_array_get(branch->commit, i);
        if (strcmp(commit->commit_id, commit_id) == 0) {
            found = 1;
            index = i;
            break;
        }
    }
    if (!found) {
        //Find in all branches
        for (int i = 0; i < svc->size; i++) {
            branch = svc->branch[i];
            for (int j = 0; j < branch->commit->size; j++) {
                commit_t *commit = commit_t_dyn_array_get(branch->commit, j);
                if (strcmp(commit->commit_id, commit_id) == 0) {
                    found = 1;
                    index = i;
                    break;
                }
            }
        }
    }

    if (!found) {
        puts("Invalid commit id");
        return;
    }

    commit_t *commit = commit_t_dyn_array_get(branch->commit, index);
    printf("%s [%s]: %s\n", commit->commit_id, branch->name, commit->message);

    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(commit->commited_file, i);
        if (file->state == ADDED) {
            printf("    + %s\n", file->file_path);
        } else if (file->state == REMOVED) {
            printf("    - %s\n", file->file_path);
        } else if (file->state == CHANGED) {
            printf("    / %s [%d --> %d]\n", file->file_path, file->previous_hash, file->hash);
        }
    }
    puts("");

    int tracked_file_size = 0;
    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(commit->commited_file, i);
        if (file->state != REMOVED) {
            tracked_file_size++;
        }
    }
    printf("    Tracked files (%d):\n", tracked_file_size);

    for (int i = 0; i < commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(commit->commited_file, i);
        if (file->state != REMOVED) {
            printf("    [%10d] %s\n", file->hash, file->file_path);
        }
    }
}

//DONE
int svc_branch(void *helper, char *branch_name) {

    printf("We have created a new branch!\n");

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
        char *name = ((struct svc*)helper)->branch[i]->name;
        if (strcmp(branch_name, name) == 0) {
            return -2;
        }

    }

    //If there are uncommitted changes: return -3
    if (((struct svc*)helper)->stage->not_changed == 0) {
        return -3;
    }

    //If successful: return 0

    //Get the SVC system 
    svc_t *svc = (struct svc*)helper;
    //Allocate space for newly created branch

    svc->branch = realloc(svc->branch, (svc->size+1)*sizeof(branch_t));

    branch_t *current_branch = svc->head; //Get the current branch
    branch_t *new_branch = malloc(sizeof(branch_t));
    new_branch->commit = commit_t_dyn_array_init();
    
    //Copy everything from the current_branch except the name field
    for (int i = 0; i < current_branch->commit->size; i++) {
        commit_t_dyn_array_add_commit(new_branch->commit, 
                commit_t_dyn_array_get(current_branch->commit, i));
    }
    new_branch->name = strdup(branch_name); // Set the name field

    svc->size++; //svc number of branches increments
    svc->branch[svc->size-1] = new_branch;

    return 0;
}

int svc_checkout(void *helper, char *branch_name) {

    printf("We checked out a branch %s\n", branch_name);

    if (branch_name == NULL) {
        return -1;
    }

    svc_t *svc = ((struct svc*)helper);

    int found = 0;
    int index = -1;
    for (int i = 0; i < svc->size; i++) {
        if (strcmp(svc->branch[i]->name, branch_name) == 0) {
            found = 1;
            index = i;
            break;
        }
    }
    
    if (!found) {
        return -1;
    }

    //If there are uncommitted changes
    if (svc->stage->not_changed == 0) {
        return -2;
    }

    svc->head = svc->branch[index];
    return 0;
}

//DONE
char **list_branches(void *helper, int *n_branches) {

    printf("We listed the branches\n");
    
    if (n_branches == NULL) {
        return NULL;
    }

    svc_t *svc = (struct svc*)helper;
    int n = svc->size;

    *n_branches = n;

    char **branch_names = malloc(sizeof(char*)*n);
    
    for (int i = 0; i < n; i++) {
        branch_t *branch = svc->branch[i];
        printf("%s\n", branch->name);
        branch_names[i] = branch->name;
    }
    return branch_names;
}

//DONE
int svc_add(void *helper, char *file_name) {

    printf("This added file name is %s\n", file_name);

    svc_t *svc = ((struct svc*)helper);
    branch_t *branch = svc->head;

    //If the file name is NULL: return -1
    if (file_name == NULL) {
        return -1;
    }
    
    stage_t *stage = svc->stage;

    //If the file name is already under version control: return -2
    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        if (file->file_path != NULL && strcmp(file->file_path, file_name) == 0) {
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

    //Determine whether there is a change that needs to be committed
    //If it is the very first commit to the SVC system
    if (branch->commit->last_commit_index == -1) {
        if (stage->tracked_file->size != 0) {
            stage->not_changed = 0; //changed
        } else {
            stage->not_changed = 1;//not changed
        }
    } 
    else { //Normal case
        commit_t *commit =  commit_t_dyn_array_get(svc->head->commit, svc->head->commit->last_commit_index);
        commit_t *last_commit =  commit_t_dyn_array_get(svc->head->commit, svc->head->commit->last_commit_index-1);
        if (last_commit == NULL) {//It is the first commit
            if (stage->tracked_file->size != commit->commited_file->size) {
                stage->not_changed = 0;
            } else {
                stage->not_changed = 1; //not changed
            }
        } else {
            if (stage->tracked_file->size != last_commit->commited_file->size) {
                stage->not_changed = 0; 
            } else {
                stage->not_changed = 1;//changed
            }
        }
    }

    //Free the constructed file_t object
    free(new_file->file_path);
    free(new_file->file_content);
    free(new_file);
    return hash_file(helper, file_name); //return hash_value
}

//DONE
int svc_rm(void *helper, char *file_name) {

    printf("This removed file name is %s\n", file_name);

    svc_t *svc = ((struct svc*)helper);
    branch_t *branch = svc->head;

    //If the file name is NULL: return -1
    if (file_name == NULL) {
        return -1;
    }

    stage_t *stage = ((struct svc*)helper)->stage;

    int file_to_be_deleted_hash = -1;

    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        if (file->file_path != NULL && strcmp(file->file_path, file_name) == 0) {
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

    //There are some uncommitted changes (addition of one)
    //If it is the first commit
    if (branch->commit->last_commit_index == -1) {
        if (stage->tracked_file->size != 0) {
            stage->not_changed = 0;
        } else {
            stage->not_changed = 1;//Set it back to normal
        }
    } 
    else { //Normal condition
        commit_t *commit =  commit_t_dyn_array_get(svc->head->commit, svc->head->commit->last_commit_index);
        commit_t *last_commit =  commit_t_dyn_array_get(svc->head->commit, svc->head->commit->last_commit_index-1);
        if (last_commit == NULL) {
            if (stage->tracked_file->size != commit->commited_file->size) {
                stage->not_changed = 0;
            } else {
                stage->not_changed = 1; // Set it back to normal
            }
        } else {
            if (stage->tracked_file->size != last_commit->commited_file->size) {
                stage->not_changed = 0; 
            } else {
                stage->not_changed = 1;//Set it back to normal
            }
        }
    }
    return file_to_be_deleted_hash;
}

int svc_reset(void *helper, char *commit_id) {

    printf("We reset to the commit %s\n", commit_id);

    if (commit_id == NULL) {
        return -1;
    }

    svc_t *svc = ((struct svc*)helper);
    branch_t *branch = svc->head;
    commit_t *current_commit = commit_t_dyn_array_get(branch->commit, branch->commit->last_commit_index);
    stage_t *stage = svc->stage;

    int found = 0;
    int index = -1;
    for (int i = 0; i < branch->commit->size; i++) {
        commit_t *pre_commit = commit_t_dyn_array_get(branch->commit, i);

        //We can not reset forward or not a simple path
        if (pre_commit == current_commit || pre_commit->n_prev == 2) {
            break;
        }

        if (strcmp(pre_commit->commit_id, commit_id) == 0) {
            found = 1;
            index = i;
            break;
        }
    }

    if (!found) {
        return -2;
    }

    commit_t *new_commit = commit_t_dyn_array_get(branch->commit, index);

    for (int i = 0; i < new_commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(new_commit->commited_file, i);
        if (file->state == CHANGED || file->state == ADDED) {
            FILE *fp = fopen(file->file_path, "w");
            fputs(file->file_content, fp); //Restore all changes
            fclose(fp);
        }
    }

    //Restore stage to the same as new_commmit
    file_t_dyn_array_free(stage->tracked_file);

    stage->not_changed = 1;
    stage->tracked_file = file_t_dyn_array_init();

    //Track all files in previous commit
    for (int i = 0; i < new_commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(new_commit->commited_file, i);
        if (file->state != REMOVED) {
            file_t_dyn_array_add(stage->tracked_file, file);
        } 
    }

    //Set tracked files state to DEFAULT
    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        file->state = DEFAULT;
    }

    branch->commit->last_commit_index = index;
    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {

    printf("We are merging now!\n");

    if (branch_name == NULL) {
        printf("Invalid branch name\n");
        return NULL;
    }

    svc_t *svc = ((struct svc*)helper);
    branch_t *current_branch = svc->head;
    commit_t *current_commit = commit_t_dyn_array_get(current_branch->commit, current_branch->commit->last_commit_index);
    stage_t *stage = svc->stage;

    int found = 0;
    int index = -1;
    for (int i = 0; i < svc->size; i++) {
        branch_t *branch = svc->branch[i];

        if (strcmp(branch->name, branch_name) == 0) {
            index = i;
            found = 1;
        }
    }

    if (!found) {
        printf("Branch not found\n");
        return NULL;
    }

    //If the given name is the currently checked out branch
    if (strcmp(branch_name, current_branch->name) == 0) {
        printf("Cannot merge a branch with itself\n");
        return NULL;
    }

    //If there are uncommitted changes
    if (stage->not_changed == 0) {
        printf("Changes must be committed\n");
        return NULL;
    }

    branch_t *merged_branch = svc->branch[index];
    commit_t *merged_branch_commit = commit_t_dyn_array_get(merged_branch->commit, merged_branch->commit->last_commit_index);

    //We use stage as temporary storage for the changes
    file_t_dyn_array_free(stage->tracked_file);
    stage->tracked_file = file_t_dyn_array_init();
    stage->not_changed = 1;

    //Add previous commit to the stage
    for (int i = 0; i < current_commit->commited_file->size; i++) {
        file_t *file = file_t_dyn_array_get(current_commit->commited_file, i);
        int state = file->state;
        file->state = DEFAULT; //Not changed since it is comparing the previous commit
        file_t_dyn_array_add(stage->tracked_file, file);
        file->state = state;
    }

    //If there are new files from the merged commit
    for (int i = 0; i < merged_branch_commit->commited_file->size; i++) {
        file_t *file1 = file_t_dyn_array_get(merged_branch_commit->commited_file, i);
        int found = 0;
        for (int j = 0; j < current_commit->commited_file->size; j++) {
            file_t *file2 = file_t_dyn_array_get(current_commit->commited_file, j);
            if (file1->file_path != NULL && file2->file_path != NULL && strcmp(file1->file_path, file2->file_path) == 0) {
                found = 1;
            }
        }
        if (!found) {
            int state = file1->state;
            file1->state = ADDED;
            file_t_dyn_array_add(stage->tracked_file, file1);
            file1->state = state;
            stage->not_changed = 0;
        }
    }

    //Now stage has all the files required that is either ADDED or DEFAULT

    //Replace with those in the resolutions
    for (int i = 0; i < n_resolutions; i++) {

        if (resolutions[i].resolved_file == NULL) {
            for (int j = 0; j < stage->tracked_file->size; j++) {
                file_t *file = file_t_dyn_array_get(stage->tracked_file, j);
                if (file->file_path != NULL && strcmp(file->file_path, resolutions[i].file_name) == 0) {
                    file->state = REMOVED;
                    stage->not_changed = 0;
                }
            }
        } else { //if there are resolutions available
                //files are taken from the merged branch
            for (int j = 0; j < stage->tracked_file->size; j++) {
                file_t *file = file_t_dyn_array_get(stage->tracked_file, j);
                if (file->file_path != NULL && strcmp(file->file_path, resolutions[i].file_name) == 0) {

                    file_t *file_in_stage = file_t_dyn_array_get(stage->tracked_file, j);

                    
                    FILE *fp = fopen(resolutions[i].file_name, "r");

                    fseek(fp, 0, SEEK_END);
                    long file_length = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    char file_contents[file_length+1];
                    fread(file_contents, sizeof(char), file_length, fp);
                    file_contents[file_length] = '\0';
                    fclose(fp);

                    printf("file content is %s\n", file_contents);

                    free(file_in_stage->file_content);
                    file_in_stage->file_content = strdup(file_contents);
                    fp = fopen(file_in_stage->file_path, "w");//Write the content into previous files
                    fputs(file_contents, fp);
                    fclose(fp);
                    
                    file_in_stage->previous_hash = file_in_stage->hash;
                    file_in_stage->hash = hash_file(helper, file_in_stage->file_path);

                    printf("new file content is %s\n", file_in_stage->file_content);

                    printf("previous file hash is %d\n", file_in_stage->previous_hash);
                    printf("new hash is %d\n", file_in_stage->hash);

                    if (file_in_stage->previous_hash != file_in_stage->hash) {
                        file_in_stage->state = CHANGED;
                        stage->not_changed = 0;
                    } 
                    else {
                        file_in_stage->state = DEFAULT;// no change
                        stage->not_changed = 1;
                    }

                }
            }
        }
    }
    //Now we have incorporated all state changes of the files in stage

    //consturct the message
    // char *message = strdup("Merged branch random branch");
    // strcat(message, branch_name);
    char message[14+50+1] = {0};
    char prefix[15] = "Merged branch ";
    sprintf(message, "%s", prefix);
    sprintf(message+14, "%s", branch_name);

    char *message2 = strdup(message);
    // printf("length of the message is %ld\n", 14+strlen(branch_name));
    // message[strlen(message)] = '\0';
    //char message[5] = "test";

    //Make the new commit from the stage
    commit_t *prev[2] = {current_commit, merged_branch_commit};
    commit_t_dyn_array_add(current_branch->commit, stage, message, 2, prev); 

    commit_t *new_commit = commit_t_dyn_array_get(current_branch->commit, current_branch->commit->last_commit_index);
    set_commit_id(new_commit);

    //Reset the file state in the stage to DEFAULT
    for (int i = 0; i < stage->tracked_file->size; i++) {
        file_t *file = file_t_dyn_array_get(stage->tracked_file, i);
        file->state = DEFAULT;
    }
    stage->not_changed = 1;

    printf("Merge successful\n");

    printf("After calling merge, we have the new commit id %s\n", new_commit->commit_id);
    printf("The message is %s\n", message);

    free(message2);

    return new_commit->commit_id;
}

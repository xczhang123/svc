#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commit_t_dyn_array.h"

#define COMMIT_T_DYN_ARRAY_DEF_CAPACITY (2)

void commit_t_dyn_array_resize(struct commit_t_dyn_array *d) {
    d->capacity *= 2;
    d->commit = realloc(d->commit, sizeof(*(d->commit)) * d->capacity);
}

struct commit_t_dyn_array* commit_t_dyn_array_init() {
    struct commit_t_dyn_array *d = malloc(sizeof(struct commit_t_dyn_array));
    d->capacity = COMMIT_T_DYN_ARRAY_DEF_CAPACITY;
    d->size = 0;
    d->last_commit_index = d->size;
    d->commit = calloc(d->capacity, sizeof(*(d->commit)));

    return d;
}

void commit_t_dyn_array_add(struct commit_t_dyn_array *dyn, stage_t *stage, char *message, int n_prev, commit_t *prev[2]) {

    if (dyn->capacity == dyn->size) {
        commit_t_dyn_array_resize(dyn);
    }

    dyn->commit[dyn->size] = malloc(sizeof(commit_t));
    dyn->commit[dyn->size]->message = strdup(message);
    file_t_dyn_array_init(dyn->commit[dyn->size]->commited_file);
    for (int i = 0; i < stage->tracked_file->size; i++) {
            file_t_dyn_array_add(dyn->commit[dyn->size]->commited_file, 
                                    file_t_dyn_array_get(stage->tracked_file, i));
    }
    dyn->commit[dyn->size]->n_prev = n_prev;
    if (prev[0] != NULL) {
        dyn->commit[dyn->size]->prev[0] = malloc(sizeof(commit_t));
        memcpy(dyn->commit[dyn->size]->prev[0], prev[0], sizeof(commit_t));
    } else if (prev[0] != NULL && prev[1] != NULL) {
        dyn->commit[dyn->size]->prev[0] = malloc(sizeof(commit_t));
        memcpy(dyn->commit[dyn->size]->prev[0], prev[0], sizeof(commit_t));
        dyn->commit[dyn->size]->prev[1] = malloc(sizeof(commit_t));
        memcpy(dyn->commit[dyn->size]->prev[1], prev[1], sizeof(commit_t));
    } else { //Both are NULL;
        dyn->commit[dyn->size]->prev[0] = NULL;
        dyn->commit[dyn->size]->prev[1] = NULL;
    }
    

    dyn->size++;
    dyn->last_commit_index = dyn->size-1;

}

commit_t* commit_t_dyn_array_get(struct commit_t_dyn_array *dyn, int index) {
    if (index < 0 || index >= dyn->size) {
        return NULL;
    }

    return dyn->commit[index];
}

void commit_t_dyn_array_free(struct commit_t_dyn_array * dyn) {
    for (int i = 0; i < dyn->size; i++) {
        free(dyn->commit[i]->message);
        file_t_dyn_array_free(dyn->commit[i]->commited_file);
        if (dyn->commit[i]->prev[0] != NULL)
            free(dyn->commit[i]->prev[0]);
        if (dyn->commit[i]->prev[1] != NULL)
            free(dyn->commit[i]->prev[1]);
        free(dyn->commit[i]);
    }
    free(dyn->commit);
    free(dyn);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_t_dyn_array.h"

#define file_t_DYN_ARRAY_DEF_CAPACITY (2)

void file_t_dyn_array_resize(struct file_t_dyn_array *d) {
    d->capacity *= 2;
    d->file = realloc(d->file, sizeof(*(d->file)) * d->capacity);
}

struct file_t_dyn_array* file_t_dyn_array_init() {
    struct file_t_dyn_array *d = malloc(sizeof(struct file_t_dyn_array));
    d->capacity = file_t_DYN_ARRAY_DEF_CAPACITY;
    d->size = 0;
    d->file = malloc(sizeof(*(d->file)) * d->capacity);

    return d;
}

void file_t_dyn_array_add(struct file_t_dyn_array *dyn, file_t *file) {

    if (dyn->capacity == dyn->size) {
        file_t_dyn_array_resize(dyn);
    }
    dyn->file[dyn->size] = malloc(sizeof(*file));
    dyn->file[dyn->size]->file_content = malloc(strlen(file->file_content)+1);
    dyn->file[dyn->size]->file_path = malloc(strlen(file->file_path)+1);
    
    memcpy(dyn->file[dyn->size]->file_content, file->file_content, strlen(file->file_content)+1);
    memcpy(dyn->file[dyn->size]->file_path, file->file_path, strlen(file->file_path)+1);

    dyn->file[dyn->size]->hash = file->hash;
    dyn->file[dyn->size]->previous_hash = file->previous_hash;
    dyn->file[dyn->size]->state = file->state;

    dyn->size++;
}

void file_t_dyn_array_delete_index(struct file_t_dyn_array *dyn, int index) {

    if (index < 0 || index >= dyn->size) {
        return;
    }

    free(dyn->file[index]->file_content);
    free(dyn->file[index]->file_path);
    free(dyn->file[index]);

    for (int i = index; i < dyn->size-1; i++) {
        dyn->file[i] = dyn->file[i+1];
    }

    dyn->size--;
}

void file_t_dyn_array_delete_file(struct file_t_dyn_array *dyn, file_t *file) {

    int found = 0;
    int index = -1;
    for (int i = 0; i < dyn->size; i++) {
        if (strcmp(dyn->file[i]->file_path, file->file_path) == 0) {
            found = 1;
            index = i;
            break;
        }
    }

    if (found) {

        free(dyn->file[index]->file_content);
        free(dyn->file[index]->file_path);
        free(dyn->file[index]);

        for (int i = index; i < dyn->size-1; i++) {
            dyn->file[i] = dyn->file[i+1];
        }
        
        dyn->size--;
    }

}

file_t* file_t_dyn_array_get(struct file_t_dyn_array *dyn, int index) {
    if (index < 0 || index >= dyn->size) {
        return NULL;
    }

    return dyn->file[index];
}

void file_t_dyn_array_free(struct file_t_dyn_array * dyn) {
    for (int i = 0; i < dyn->size; i++) {
        free(dyn->file[i]->file_content);
        free(dyn->file[i]->file_path);
        free(dyn->file[i]);
    }
    free(dyn->file);
    free(dyn);
}
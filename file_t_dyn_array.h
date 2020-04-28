#ifndef file_t_dyn_array_h
#define file_t_dyn_array_h

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
    int previous_hash;
} file_t;

struct file_t_dyn_array {
    int capacity;
    int size;
    file_t **file;
};

void file_t_dyn_array_resize(struct file_t_dyn_array *d);

struct file_t_dyn_array* file_t_dyn_array_init();

void file_t_dyn_array_add(struct file_t_dyn_array *dyn, file_t *file);

void file_t_dyn_array_delete_index(struct file_t_dyn_array *dyn, int index);

void file_t_dyn_array_delete_file(struct file_t_dyn_array *dyn, file_t *file);

file_t* file_t_dyn_array_get(struct file_t_dyn_array *dyn, int index);

void file_t_dyn_array_free(struct file_t_dyn_array *dyn);

#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "svc.h"

typedef struct command command_t;

struct command {
	char* str;
	int (*exe)();
};

int test_first_commit() {
    void *helper = svc_init();

    assert(svc_add(helper, "test1.txt") > 0);
    assert(svc_add(helper, "test2.txt") > 0);

    struct file_t_dyn_array *tracked_files = ((struct svc*)helper)->stage->tracked_file;

    assert(tracked_files->size == 2);
    assert(file_t_dyn_array_get(tracked_files,0)->file_path != NULL);
    assert(file_t_dyn_array_get(tracked_files,0)->file_content != NULL);

    char *commit_id = svc_commit(helper, "first commit");

    // printf("%s\n", commit_id);

    cleanup(helper);
    
    
    return 1;
}

int test_add_branch() {
    void *helper = svc_init();

    // svc_t *svc = ((struct svc*)helper);

    svc_add(helper, "empty.txt");

    svc_commit(helper, "first commit");

    cleanup(helper);

    return 1;
}

int example1() {
    void *helper = svc_init();

    assert(hash_file(helper, "hello.py") == 2027);
    assert(hash_file(helper, "fake.c") == -2);
    assert(svc_commit(helper, "No changes") == NULL);

    assert(svc_add(helper, "hello.py") == 2027);

    // printf("%d\n", ((struct svc*)helper)->stage->not_changed);

    assert(svc_add(helper, "Tests/test1.in") == 564);

    // printf("%d\n", ((struct svc*)helper)->stage->not_changed);

    assert(svc_add(helper, "Tests/test1.in") == -2);

    // printf("%d\n", ((struct svc*)helper)->stage->not_changed);

    // printf("%s\n", svc_commit(helper, "Initial commit"));
    assert(strcmp(svc_commit(helper, "Initial commit"), "74cde7") == 0);
    // Return value: "74cde7"

    cleanup(helper);

    return 1;
}

int example2() {

    void *helper = svc_init();
    svc_t *svc = ((struct svc*)helper);

    FILE* fp0 = fopen("COMP2017/svc.h", "w");
    char buf0[] = "#ifndef svc_h\n#define svc_h\nvoid *svc_init(void);\n#endif\n";
    fputs(buf0, fp0);
    fclose(fp0);

    assert(svc_add(helper, "COMP2017/svc.h") == 5007);

    FILE *fp = fopen("COMP2017/svc.c", "w");
    char buf1[] = "# include \"svc.h\"\nvoid *svc_init(void) {\n    //TODO: implement\n}\n";
    fputs(buf1,fp);
    fclose(fp);

    assert(svc_add(helper, "COMP2017/svc.c") == 5217);

    // printf("%s\n", svc_commit(helper, "Initial commit"));
    assert(strcmp(svc_commit(helper, "Initail commit"), "7b3e30") == 0);

    branch_t *branch = svc->head;
    commit_t *commit = commit_t_dyn_array_get(branch->commit, 0);
    printf("%s\n", branch->name);
    printf("%s\n", commit->commit_id);
    printf("%ld\n", commit->n_prev);
    printf("%s %d\n", file_t_dyn_array_get(commit->commited_file, 0)->file_path, file_t_dyn_array_get(commit->commited_file, 0)->state);
    printf("%s %d\n",file_t_dyn_array_get(commit->commited_file, 1)->file_path, file_t_dyn_array_get(commit->commited_file, 0)->state);
    printf("prev1: %s\n", commit->prev[0]->commit_id);
    printf("prev2: %p\n", commit->prev[1]);
    printf("-------\n");

    assert(svc_branch(helper, "random_branch") == 0);

    assert(svc_checkout(helper, "random_branch") == 0);
    
    char buf2[] = "#include \"svc.h\"\nvoid *svc_init(void) {\n    return NULL;\n}\n";
    fp = fopen("COMP2017/svc.c", "w");
    fputs(buf2,fp);
    fclose(fp);

    branch = svc->head;
    printf("%s\n", branch->name);
    stage_t *stage = svc->stage;

    // for (int i = 0; i < stage->tracked_file->size; i++) {
    //     printf("Before removal: the stage files are: %s state is: %d\n", file_t_dyn_array_get(stage->tracked_file, i)->file_path,
    //                     file_t_dyn_array_get(stage->tracked_file,i)->state);
    // }
    // printf("--------\n");

    assert(svc_rm(helper, "COMP2017/svc.h") == 5007);

    // stage = svc->stage;
    // for (int i = 0; i < stage->tracked_file->size; i++) {
    //     printf("After removal: the stage files are: %s state is: %d\n", file_t_dyn_array_get(stage->tracked_file, i)->file_path,
    //                     file_t_dyn_array_get(stage->tracked_file,i)->state);
    // }
    // printf("--------\n");

    assert(strcmp(svc_commit(helper, "Implemented svc_init"), "73eacd") == 0);

    branch = svc->head;
    printf("%s\n", branch->name);
    commit = commit_t_dyn_array_get(branch->commit, 0);

    assert(svc_reset(helper, "7b3e30") == 0);

    // stage = svc->stage;
    // for (int i = 0; i < stage->tracked_file->size; i++) {
    //     printf("After reset: the stage files are: %s state is: %d\n", file_t_dyn_array_get(stage->tracked_file, i)->file_path,
    //                     file_t_dyn_array_get(stage->tracked_file,i)->state);    }
    // printf("------\n");
    
    // printf("first commit\n");
    // printf("%s\n", commit->commit_id);
    // printf("%ld\n", commit->n_prev);
    // printf("%s %d\n", file_t_dyn_array_get(commit->commited_file, 0)->file_path, file_t_dyn_array_get(commit->commited_file, 0)->state);
    // printf("%s %d\n",file_t_dyn_array_get(commit->commited_file, 1)->file_path, file_t_dyn_array_get(commit->commited_file, 1)->state);
    // printf("prev1: %s\n", commit->prev[0]->commit_id);
    // printf("prev2: %p\n", commit->prev[1]);
    // printf("-------\n");

    branch = svc->head;
    printf("%s\n", branch->name);
    commit = commit_t_dyn_array_get(branch->commit, 1);
    stage = svc->stage;

    // printf("second commit\n");
    // printf("%s\n", commit->commit_id);
    // printf("%ld\n", commit->n_prev);
    // printf("%s %d\n", file_t_dyn_array_get(commit->commited_file, 0)->file_path, file_t_dyn_array_get(commit->commited_file, 0)->state);
    // printf("%s %d\n",file_t_dyn_array_get(commit->commited_file, 1)->file_path, file_t_dyn_array_get(commit->commited_file, 1)->state);
    // printf("prev1: %s\n", commit->prev[0]->commit_id);
    // printf("prev2: %p\n", commit->prev[1]);
    // printf("--------\n");


    // stage = svc->stage;
    // for (int i = 0; i < stage->tracked_file->size; i++) {
    //     printf("Before rewrite : the stage files are: %s state is: %d\n", file_t_dyn_array_get(stage->tracked_file, i)->file_path,
    //                     file_t_dyn_array_get(stage->tracked_file,i)->state);    }
    // printf("------\n");

    //Then we rewrite the svc.c
    //both changed
    fp = fopen("COMP2017/svc.c", "w");
    fputs(buf2,fp);
    fclose(fp);


    // remove("COMP2017/svc.h");
    // fp = fopen("COMP2017/svc.h", "w");
    // fputs(buf2,fp);
    // fclose(fp);

    // remove("COMP2017/svc.c");
    // remove("COMP2017/svc.h");

    assert(strcmp(svc_commit(helper, "Implemented svc_init"), "24829b") == 0);

    // stage = svc->stage;
    // for (int i = 0; i < stage->tracked_file->size; i++) {
    //     printf("After rewrite : the stage files are: %s state is: %d\n", file_t_dyn_array_get(stage->tracked_file, i)->file_path,
    //                     file_t_dyn_array_get(stage->tracked_file,i)->state);    }
    // printf("------\n");


    branch = svc->head;
    printf("%s\n", branch->name);

    commit = commit_t_dyn_array_get(branch->commit, 2);
    stage = svc->stage;
    
    printf("third commit\n");
    printf("%s\n", commit->commit_id);
    printf("%ld\n", commit->n_prev);
    printf("%s %d\n", file_t_dyn_array_get(commit->commited_file, 0)->file_path, file_t_dyn_array_get(commit->commited_file, 0)->state);
    printf("%s %d\n", file_t_dyn_array_get(commit->commited_file, 1)->file_path, file_t_dyn_array_get(commit->commited_file, 1)->state);
    printf("prev1: %s\n", commit->prev[0]->commit_id);
    printf("prev2: %p\n", commit->prev[1]);
    printf("-------\n");



    // printf("%d\n",hash_file(helper, "COMP2017/svc.c"));


    // assert(strcmp(svc_commit(helper, "Implemented svc_init"), "24829b") == 0);


    //assert(svc_reset(helper, "7b3e30") == 0);

    remove("COMP2017/svc.c");
    remove("COMP2017/svc.h");

    fp = fopen("c.a", "w");
    char buf[] = "!<arch>\n/               1554401209  0     0     0       1122      `\n";
    fwrite(buf, 1, strlen(buf),fp);
    fclose(fp);
    printf("%d\n",hash_file(helper, "c.a"));

    remove("c.a");

    cleanup(helper);
    return 1;
}



command_t tests[] = {
   {"test_first_commit", &test_first_commit},
   {"test_add_branch", &test_add_branch},
   {"example1", &example1},
   {"example2", &example2}
};


int main(int argc, char** argv) {
  int test_n = sizeof(tests) / sizeof(command_t);
  if(argc >= 2) {
		for(int i = 0; i < test_n; i++) {
			if(strcmp(argv[1], tests[i].str) == 0) {
				if(tests[i].exe()) {
				  fprintf(stdout, "%s Passed\n", tests[i].str);
				} else {
				  fprintf(stdout, "%s Failed\n", tests[i].str);
				}
			}
		}
        if (strcmp(argv[1], "all") == 0) {
            for(int i = 0; i < test_n; i++) {
				if(tests[i].exe()) {
				  fprintf(stdout, "%s Passed\n", tests[i].str);
				} else {
				  fprintf(stdout, "%s Failed\n", tests[i].str);
				}
            }
        }
	}
}

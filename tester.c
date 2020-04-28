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

    svc_t *svc = ((struct svc*)helper);

    assert(svc->size == 1);

    // svc_add(helper, "test1.txt");

    // //printf("%s\n",svc_commit(helper, "test"));

    // int ret1 = svc_branch(helper, "test1");

    // assert(ret1 == 0);
    // assert(svc->size == 2);

    // int ret2 = svc_branch(helper, "test2");

    // assert(ret2 == 0);
    // assert(svc->size == 3);

    // int n_branch;
    // char **branch_names = list_branches(helper, &n_branch);
    // assert (n_branch == 3);

    // free(branch_names);
    cleanup(helper);

    return 1;
}

int example1() {
    // void *helper = svc_init();
    // assert(hash_file(helper, "hello.py") == 2027);
    // assert(hash_file(helper, "fake.c") == -2);
    // assert(svc_commit(helper, "No changes") == NULL);
    //Return value: NULL
    // svc_add(helper, "hello.py");
    // //Return value: 2027
    // //svc_add(helper, "Tests/test1.in");
    // //Return value: 564 (from example above)
    // svc_add(helper, "Tests/test1.in");
    // // Return value: -2
    // svc_commit(helper, "Initial commit");
    // Return value: "74cde7"
}



command_t tests[] = {
   {"test_first_commit", &test_first_commit},
   {"test_add_branch", &test_add_branch},
   {"example1", &example1}
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

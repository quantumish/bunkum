#define _GNU_SOURCE

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/fcntl.h>

#include <pthread.h>
#include <libunwind.h>
#include <libunwind-ptrace.h>

#include "log.h"
#include "profile.h"

shitvec_t profile(pid_t tid) {
    shitvec_t stack = shitvec_new(MAX_SYMLEN);    
    errno = 0;
    ptrace(PTRACE_ATTACH, tid);
    kill(tid, SIGSTOP);
    waitpid(tid, NULL, 0);
    void* ui = _UPT_create(tid);
    unw_cursor_t c;
    unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);
    unw_init_remote(&c, as, ui);
    do {
        unw_word_t offset;
        char fname[MAX_SYMLEN] = {0};
        int resp = unw_get_proc_name(&c, fname, sizeof(fname), &offset);
        shitvec_push(&stack, fname);
    } while(unw_step(&c) > 0);
    _UPT_resume(as, &c, ui);
    _UPT_destroy(ui);
    kill(tid, SIGSTOP);
    waitpid(tid, NULL, 0);
    ptrace(PTRACE_DETACH, tid, NULL, NULL);
    return stack;
}

struct profile_node profile_node_new(char* name) {
	struct profile_node out = {
		.children = shitvec_new(sizeof(struct profile_node)),
		.samples = 0,
		.symbol = {0}
	};
	strcpy(out.symbol, name);
	return out;
}

struct profile_node profile_res_new() {
	return profile_node_new("");
}

int cmp_prof_node(void* _a, void* _b) {
	struct profile_node* a = _a;
	struct profile_node* b = _b;
	return strcmp(a->symbol, b->symbol);	
}

void profile_proc_stack(struct profile_node* prof_res, shitvec_t* stack) {
	if (stack->vec_sz == 0) return;	
	struct profile_node* node = prof_res;
	for (int i = stack->vec_sz-1; i > 0; i--) {
		char* sym = shitvec_get(stack, i);
		int index = shitvec_check(&node->children, sym, cmp_prof_node);
		if (index == -1) {
			struct profile_node new = profile_node_new(sym);
			shitvec_push(&node->children, &new);
			index = node->children.vec_sz - 1;
		}
	    node = shitvec_get(&node->children, index);
		node->samples += 1;
	}
}

void profile_dump(struct profile_node* prof_res, int indent) {
	struct profile_node* node = prof_res;
	if (indent >= 0) {
		for (int i = 0; i < indent; i++) printf("  ");
		printf("%s (%d) \n", node->symbol, node->samples);
	}
	for (int i = 0; i < node->children.vec_sz; i++) {
		profile_dump(shitvec_get(&node->children, i), indent+1);
	}
}

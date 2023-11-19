#ifndef PROFILE_H
#define PROFILE_H

#include <sys/types.h>

#include "vec.h"

#define MAX_SYMLEN 32
vec_t profile(pid_t pid);

struct profile_node {
	char symbol[MAX_SYMLEN];
	unsigned int samples;
	vec_t children;	
};

struct profile_node profile_res_new();
void profile_proc_stack(struct profile_node* prof_res, vec_t* stack);
void profile_dump(struct profile_node* prof_res, int indent);

#endif

#ifndef SHITVEC_H
#define SHITVEC_H

#include <stddef.h>
#include <stdbool.h>

#define SHITVEC_INIT_SZ 1024

typedef struct vec {
    void* arr;
    size_t e_sz;
    size_t alloc_sz;
    size_t vec_sz;
} vec_t;

typedef int(*sv_cmp_t)(void*, void*);

vec_t vec_new(size_t e_sz);
void* vec_get(vec_t* sv, size_t index);
void* vec_last(vec_t* sv);
void vec_push(vec_t* sv, void* item);
void vec_subpush(vec_t* sv, void* item, size_t sz);
int vec_check(vec_t* sv, void* item, sv_cmp_t cmp);
void vec_sort(vec_t* sv, int(*cmp)(const void*, const void*));
void vec_free(vec_t* sv);

#endif



#ifndef SHITVEC_H
#define SHITVEC_H

#include <stddef.h>
#include <stdbool.h>

#define SHITVEC_INIT_SZ 1024

typedef struct shitvec {
    void* arr;
    size_t e_sz;
    size_t alloc_sz;
    size_t vec_sz;
} shitvec_t;

typedef int(*sv_cmp_t)(void*, void*);

shitvec_t shitvec_new(size_t e_sz);
void* shitvec_get(shitvec_t* sv, size_t index);
void* shitvec_last(shitvec_t* sv);
void shitvec_push(shitvec_t* sv, void* item);
void shitvec_subpush(shitvec_t* sv, void* item, size_t sz);
bool shitvec_check(shitvec_t* sv, void* item, sv_cmp_t cmp);
void shitvec_sort(shitvec_t* sv, int(*cmp)(const void*, const void*));
void shitvec_free(shitvec_t* sv);

#endif



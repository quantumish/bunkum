#include <stdlib.h>
#include <string.h>

#include "vec.h"

vec_t vec_new(size_t e_sz) {
    vec_t sv;
    sv.alloc_sz = SHITVEC_INIT_SZ;
    sv.vec_sz = 0;
    sv.e_sz = e_sz;
    sv.arr = malloc(sv.alloc_sz);
    memset(sv.arr, 0, sv.alloc_sz);
    return sv;
}

void* vec_get(vec_t* sv, size_t index) {
    if (index > sv->vec_sz) return 0x0;
    return sv->arr+(sv->e_sz * index);
}

void* vec_last(vec_t* sv) {
	return vec_get(sv, sv->vec_sz-1);
}

void vec_push(vec_t* sv, void* item) {
    // FIXME sketchy af
    if ((sv->arr+(2 * sv->e_sz * sv->vec_sz)) > sv->arr+sv->alloc_sz) {
        sv->arr = realloc(sv->arr, sv->alloc_sz * 2);
        sv->alloc_sz *= 2;
    }
    memcpy(sv->arr+(sv->vec_sz * sv->e_sz), item, sv->e_sz);
    sv->vec_sz += 1;
}

void vec_subpush(vec_t* sv, void* item, size_t sz) {
    sv->vec_sz += 1;
    if ((sv->arr+(sv->e_sz * sv->vec_sz)) > sv->arr+sv->alloc_sz) {
        sv->arr = realloc(sv->arr, sv->alloc_sz * 2);
    }
    memcpy(sv->arr+(sv->vec_sz * sv->e_sz), item, sz);
}

int vec_check(vec_t* sv, void* item, sv_cmp_t cmp) {
    for (size_t i = 0; i < sv->vec_sz; i++) {
        if (cmp(sv->arr+(i*sv->e_sz), item) == 0) {
            return i;
        } 
    }
    return -1;
}

void vec_sort(vec_t* sv, int(*cmp)(const void*, const void*)) {
    qsort(sv->arr, sv->vec_sz, sv->e_sz, cmp);
}

void vec_free(vec_t* sv) {
    free(sv->arr);
}

#ifdef TEST
#include "../test.h"

void test_vec_sanity() {
    vec_t sv = vec_new(8);
    vec_push(&sv, "whee");
    vec_push(&sv, "whoo");
    assert_str_eq("whee", vec_get(&sv, 0));
    assert_str_eq("whoo", vec_get(&sv, 1));
    assert_size_eq(2, sv.vec_sz);
    assert_size_eq(8, sv.e_sz);
    assert_bool(vec_check(&sv, "whoo", (sv_cmp_t)strcmp));
};
#endif

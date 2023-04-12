
#include <stdlib.h>
#include <string.h>

#include "shitvec.h"

shitvec_t shitvec_new(size_t e_sz) {
    shitvec_t sv;
    sv.alloc_sz = SHITVEC_INIT_SZ;
    sv.vec_sz = 0;
    sv.e_sz = e_sz;
    sv.arr = malloc(sv.alloc_sz);
    memset(sv.arr, 0, sv.alloc_sz);
    return sv;
}

void* shitvec_get(shitvec_t* sv, size_t index) {
    if (index > sv->vec_sz) return 0x0;
    return sv->arr+(sv->e_sz * index);
}

void shitvec_push(shitvec_t* sv, void* item) {
    // FIXME sketchy af
    if ((sv->arr+(2 * sv->e_sz * sv->vec_sz)) > sv->arr+sv->alloc_sz) {
        sv->arr = realloc(sv->arr, sv->alloc_sz * 2);
        sv->alloc_sz *= 2;
    }
    memcpy(sv->arr+(sv->vec_sz * sv->e_sz), item, sv->e_sz);
    sv->vec_sz += 1;
}

void shitvec_subpush(shitvec_t* sv, void* item, size_t sz) {
    sv->vec_sz += 1;
    if ((sv->arr+(sv->e_sz * sv->vec_sz)) > sv->arr+sv->alloc_sz) {
        sv->arr = realloc(sv->arr, sv->alloc_sz * 2);
    }
    memcpy(sv->arr+(sv->vec_sz * sv->e_sz), item, sz);
}

bool shitvec_check(shitvec_t* sv, void* item, sv_cmp_t cmp) {
    for (size_t i = 0; i < sv->vec_sz; i++) {
        if (cmp(sv->arr+(i*sv->e_sz), item) == 0) {
            return true;
        } 
    }
    return false;    
}

void shitvec_sort(shitvec_t* sv, int(*cmp)(const void*, const void*)) {
    qsort(sv->arr, sv->vec_sz, sv->e_sz, cmp);
}

void shitvec_free(shitvec_t* sv) {
    free(sv->arr);
}

#ifdef TEST
#include "../test.h"

void test_shitvec_sanity() {
    shitvec_t sv = shitvec_new(8);
    shitvec_push(&sv, "whee");
    shitvec_push(&sv, "whoo");
    assert_str_eq("whee", shitvec_get(&sv, 0));
    assert_str_eq("whoo", shitvec_get(&sv, 1));
    assert_size_eq(2, sv.vec_sz);
    assert_size_eq(8, sv.e_sz);
    assert_bool(shitvec_check(&sv, "whoo", (sv_cmp_t)strcmp));
};
#endif

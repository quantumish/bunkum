#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "test.h"

void assert_str_eq(char* a, char* b) {
    if (strcmp(a, b) != 0) {
        printf("(%s != %s)", a, b);
        exit(1);
    }
} 

void assert_size_eq(size_t a, size_t b) {
    if (a != b) {
        printf("(%zu != %zu)", a, b);
        exit(1);
    }
}

void assert_bool(bool expr) {
    if (!expr) {
        exit(1);
    }
}

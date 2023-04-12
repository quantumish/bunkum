#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stddef.h>

void assert_str_eq(char* a, char* b);
void assert_float_eq(float a, float b);
void assert_size_eq(size_t a, size_t b);
void assert_bool(bool expr);

#endif

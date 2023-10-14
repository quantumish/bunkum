#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

typedef struct hashmap_t {
    void* keys;
    void* vals;
    size_t k_sz;
    size_t v_sz;
    size_t len;
    size_t filled;
	bool vark;
} hashmap_t;

// Generates a new hashmap_t.
hashmap_t hashmap_new(size_t ksize, size_t vsize);

#define hashmap_init(t1, t2) hashmap_new(sizeof(t1), sizeof(t2))

// Sets a key-value pair in the hashmap_t passed to it. Returns 0x0 for success, 0x1 for failure.
int hashmap_set(hashmap_t* h, void* k, void* v);

// Returns address of value associated with key in hashmap if it exists, otherwise returns 0x0 for no value or 0x1 for full table.
void* hashmap_get(hashmap_t* h, void* k);

// Deletes key in hashmap (but not value).
int hashmap_del(hashmap_t* h, void* k);

// Frees a hashmap.
void hashmap_free(hashmap_t* h);

void hashmap_dump(hashmap_t* h);

#endif

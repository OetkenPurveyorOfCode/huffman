#pragma once
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "../util/array.h"
#define ARENA_ASAN_SEPARATION 8
#define ARENA_BACKEND_MALLOC
#define ARENA_IMPLEMENTATION
#include "../util/arena.h"


#define WRITE_CALLBACK_ADVANCE_WRITE_COUNTER_BY_MODULUS_AND_WRITE_FULL_FRAME (SIZE_MAX-1)

#ifndef ORDERING_DEFINED
#define ORDERING_DEFINED
typedef enum {
    Ordering_LessThan = -1,
    Ordering_Equal = 0,
    Ordering_GreaterThan = 1,
} Ordering;
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

typedef ptrdiff_t ssize_t;

static inline void mem_swap(char* tmp, char* elemsc, size_t elem_size, size_t i, size_t j) {
    memcpy(tmp, &elemsc[i*elem_size], elem_size);
    memcpy(&elemsc[i*elem_size], &elemsc[j*elem_size], elem_size);
    memcpy(&elemsc[j*elem_size], tmp, elem_size);
    
}

void dummy_write_cb(void* userdata_void, size_t index) {
    (void)userdata_void;
    (void)index;
}

void heapify_min(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len, 
    size_t i,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    char* tmp
) {
    char* elemsc = (char*)elems;
	size_t minimum = i; 
	size_t right_child_index = 2 * i + 2; 
	size_t left_child_index = 2 * i + 1; 
    
	if (left_child_index < elem_len && cmp_cb(userdata, &elemsc[left_child_index*elem_size], &elemsc[minimum*elem_size]) == Ordering_LessThan) {
		minimum = left_child_index; 
    }
	if (right_child_index < elem_len && cmp_cb(userdata, &elemsc[right_child_index*elem_size], &elemsc[minimum*elem_size])== Ordering_LessThan) {
		minimum = right_child_index; 
    }
	if (minimum != i) { 
        mem_swap(tmp, elemsc, elem_size, i, minimum);
        write_cb(userdata, i);
        write_cb(userdata, minimum);
		heapify_min(userdata, elems, elem_size, elem_len, minimum, write_cb, cmp_cb, tmp); 
	} 
}

static void __attribute__((no_sanitize("unsigned-integer-overflow"))) heapify_min_all(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* key = malloc(elem_size);
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        heapify_min(userdata, elems, elem_size, elem_len, i, write_cb, cmp_cb, key);
    }
    free(key);
}

void heapify_min2(
    void* userdata,
    void* base,
    void* elems,
    size_t elem_size,
    size_t elem_len, 
    size_t i,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    char* tmp
) {
    char* elemsc = (char*)elems;
	size_t minimum = i; 
	size_t right_child_index = 2 * i + 2; 
	size_t left_child_index = 2 * i + 1; 
    
	if (left_child_index < elem_len && cmp_cb(userdata, &elemsc[left_child_index*elem_size], &elemsc[minimum*elem_size]) == Ordering_LessThan) {
		minimum = left_child_index; 
    }
	if (right_child_index < elem_len && cmp_cb(userdata, &elemsc[right_child_index*elem_size], &elemsc[minimum*elem_size])== Ordering_LessThan) {
		minimum = right_child_index; 
    }
	if (minimum != i) { 
        mem_swap(tmp, elemsc, elem_size, i, minimum);
        write_cb(userdata, (elems-base)/elem_size+i);
        write_cb(userdata, (elems-base)/elem_size+minimum);
		heapify_min2(userdata, base, elems, elem_size, elem_len, minimum, write_cb, cmp_cb, tmp); 
	} 
}

static void __attribute__((no_sanitize("unsigned-integer-overflow"))) heapify_min_all2(
    void* userdata,
    void* base,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* key = malloc(elem_size);
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        heapify_min2(userdata, base, elems, elem_size, elem_len, i, write_cb, cmp_cb, key);
    }
    free(key);
}


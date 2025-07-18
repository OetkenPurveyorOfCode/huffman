#pragma once
#include "arena.h"

typedef enum {
    Ordering_LessThan = -1,
    Ordering_Equal = 0,
    Ordering_GreaterThan = 1,
} Ordering;

typedef struct {
    Arena* arena;
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2);
    void* userdata;
    char* data;
    size_t elem_size;
    size_t len;
    size_t capacity;
} HeapQueue;

HeapQueue hq_from_array(Arena* arena, void* userdata, void* elems, size_t elem_size, size_t elem_len, Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2));
void* hq_pop(HeapQueue* q);
void hq_push(HeapQueue* q, void* elem);

#ifdef HEAPQ_IMPLEMENTATION
void hq_impl_heapify_min(void* userdata,void* elems,size_t elem_size,size_t elem_len, size_t i,Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2));
void hq_impl_swap(char* lhs, char* rhs, size_t len);

HeapQueue hq_from_array(Arena* arena, void* userdata, void* elems, size_t elem_size, size_t elem_len, Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)){
    void* data = arena_alloc(arena, elem_len*elem_size);
    memcpy(data, elems, elem_len*elem_size);
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        hq_impl_heapify_min(userdata, data, elem_size, elem_len, i, cmp_cb);
    }
    return (HeapQueue){
        .arena = arena,
        .userdata = userdata,
        .cmp_cb = cmp_cb,
        .data = data,
        .elem_size = elem_size,
        .len = elem_len,
        .capacity = elem_len,
    };
}

void* hq_pop(HeapQueue* q) {
    if (q->len == 0) return 0;
    char* elemsc = q->data;
    //hq_impl_swap(&elemsc[(q->len-1)*q->elem_size], &elemsc[0], q->elem_size);
    size_t elem_size = q->elem_size;
    Arena tmp_arena = *q->arena;
    char* tmp = arena_alloc(&tmp_arena, q->elem_size);
    memcpy(tmp, &elemsc[(q->len-1)*q->elem_size], elem_size);
    memcpy(&elemsc[(q->len-1)*q->elem_size], elemsc, elem_size);
    memcpy(elemsc, tmp, elem_size);
    hq_impl_heapify_min(q->userdata, elemsc, q->elem_size, q->len-1, 0, q->cmp_cb);
    q->len -= 1;
    return &elemsc[q->len*q->elem_size];
}

void hq_impl_siftdown(
    Arena arena,
    void* userdata,
    char* elemsc,
    size_t elem_size,
    size_t elem_len,
    size_t pos,
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    size_t startpos = 0;
    char* newitem = arena_alloc(&arena, elem_size);
    char* parent = arena_alloc(&arena, elem_size);
    memcpy(newitem, &elemsc[pos*elem_size], elem_size);
    while (pos > 0) {
        size_t parentpos = (pos - 1) >> 1;
        memcpy(parent, &elemsc[parentpos*elem_size], elem_size);
        if (cmp_cb(userdata, newitem, parent) == Ordering_LessThan) {
            memcpy(&elemsc[pos*elem_size], parent, elem_size);
            pos = parentpos;
            continue;
        }
        break;
    }
    memcpy(&elemsc[pos*elem_size], newitem, elem_size);
}

void hq_push(HeapQueue* q, void* elem) {
    while (q->len + 1 >= q->capacity) {
        size_t old_capacity = q->capacity;
        if (q->capacity) q->capacity *= 2;
        else q->capacity = 1;
        q->data = arena_realloc(q->arena, q->data, old_capacity*q->elem_size, q->capacity*q->elem_size);
    }
    memcpy(&q->data[q->len*q->elem_size], elem, q->elem_size);
    q->len += 1;
    hq_impl_siftdown(*q->arena, q->userdata, q->data, q->elem_size, q->len, q->len-1, q->cmp_cb);
}


// TODO: make non-recursive
void hq_impl_heapify_min(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len, 
    size_t i,
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
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
        hq_impl_swap(&elemsc[i*elem_size], &elemsc[minimum*elem_size], elem_size);
		hq_impl_heapify_min(userdata, elems, elem_size, elem_len, minimum, cmp_cb); 
	} 
}

void hq_impl_swap(char* lhs, char* rhs, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char tmp  = lhs[i];
        lhs[i] = rhs[i];
        rhs[i] = tmp;
    }
}


#endif

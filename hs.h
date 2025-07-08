#pragma once
#include "sort_common.h"

void heapify(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len, 
    size_t i,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    char* tmp
    ) 
{
    char* elemsc = (char*)elems;
	size_t maximum = i; 
	size_t right_child_index = 2 * i + 2; 
	size_t left_child_index = 2 * i + 1; 
    
	if (left_child_index < elem_len && cmp_cb(userdata, &elemsc[left_child_index*elem_size], &elemsc[maximum*elem_size]) == Ordering_GreaterThan) {
		maximum = left_child_index; 
    }
	if (right_child_index < elem_len && cmp_cb(userdata, &elemsc[right_child_index*elem_size], &elemsc[maximum*elem_size])== Ordering_GreaterThan) {
		maximum = right_child_index; 
    }
	if (maximum != i) { 
        mem_swap(tmp, elemsc, elem_size, i, maximum);
        write_cb(userdata, i);
        write_cb(userdata, maximum);
		heapify(userdata, elems, elem_size, elem_len, maximum, write_cb, cmp_cb, tmp); 
	} 
}

static void __attribute__((no_sanitize("unsigned-integer-overflow"))) heapsort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    size_t modulus,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    (void)modulus;
    char* elemsc = (char*)elems;
    char* tmp = malloc(elem_size);
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        heapify(userdata, elems, elem_size, elem_len, i, write_cb, cmp_cb, tmp);
    }
    for (size_t i = elem_len - 1; i > 0; i--) {
        mem_swap(tmp, elemsc, elem_size, 0, i);
        write_cb(userdata, 0);
        write_cb(userdata, i);
        heapify(userdata, elems, elem_size, i, 0, write_cb, cmp_cb, tmp);
    }
    free(tmp);
}

static void heapifysort2(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    size_t modulus,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    (void)modulus;
    for (size_t i = 0; i < elem_len; i++) {
        heapify_min_all(userdata, elems+i*elem_size, elem_size, elem_len-i, write_cb, cmp_cb);
    }
}

static void heapifysort_impl(
    void* userdata,
    void* base,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    if (elem_len <= 1) return;
    heapify_min_all2(userdata, base, elems, elem_size, elem_len, write_cb, cmp_cb);
    size_t middle = elem_len/2;
    heapifysort_impl(userdata, base, elems,                  elem_size, middle,          write_cb, cmp_cb);
    heapifysort_impl(userdata, base, elems+middle*elem_size, elem_size, elem_len-middle, write_cb, cmp_cb);
    heapify_min_all2(userdata, base, elems,                  elem_size, elem_len,        write_cb, cmp_cb);
}

void maxsort(
    void* userdata,
    void* elems_in,
    size_t elem_size,
    size_t elem_len,
    size_t modulus,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
);

static void heapifysort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    size_t modulus,
    void (*write_cb)(void* userdata_void, size_t index),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    heapifysort_impl(userdata, elems, elems, elem_size, elem_len, write_cb, cmp_cb);
    maxsort(userdata, elems, elem_size, elem_len, modulus, write_cb, cmp_cb);
}




#ifdef TEST_HEAPSORT
Ordering cmp_int(void* ud, void* a, void* b) {
    int ia = *((int*)a);
    int ib = *((int*)b);
    if (ia < ib) {return Ordering_LessThan;}
    else if (ia == ib) {return Ordering_Equal;}
    else { return Ordering_GreaterThan;}
}

void write_cb(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end) {
    return;
}
int main() 
{ 
	// initializing the array 
	int arr[] = {1, 11, 20, 18, 5, 15, 3, 2 }; 

	// Displaying original array 
	printf("Original Array : "); 
	for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) { 
		printf("%d ", arr[i]); 
	} 

	printf("\n"); 
	heapsort(0, arr, sizeof(arr[0]), sizeof(arr)/sizeof(arr[0]), 1000, write_cb, cmp_int);

	// Displaying sorted array 
	printf("Array after performing heap sort: "); 
	for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) { 
		printf("%d ", arr[i]); 
	} 
	return 0; 
}
#endif


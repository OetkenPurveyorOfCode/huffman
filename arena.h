#ifndef ARENA_H
#define ARENA_H
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

    #ifdef __has_feature
        #if __has_feature(address_sanitizer) 
            #define ARENA_ASAN_ENABLED (1)
        #else
            #define ARENA_ASAN_ENABLED (0)
        #endif // __has_feature(asan)
    #else
        #ifdef __SANITIZE_ADDRESS__
            #define ARENA_ASAN_ENABLED (1)
        #else
            #define ARENA_ASAN_ENABLED (0)
        #endif // __SANITIZE_ADDRESS__
    #endif // __has_feature

    #if ARENA_ASAN_ENABLED
        void __asan_poison_memory_region(void const volatile *addr,size_t size);
        void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
        #define ARENA_ASAN_POISON(addr, size) __asan_poison_memory_region((addr), (size))
        #define ARENA_ASAN_UNPOISON(addr, size) __asan_unpoison_memory_region((addr), (size))
    #else
        #define ARENA_ASAN_POISON(addr, size) ((void)(addr), (void)(size))
        #define ARENA_ASAN_UNPOISON(addr, size) ((void)(addr), (void)(size))
    #endif // ARENA_ASAN_ENABLED

    #ifndef ARENA_OOM
    #define ARENA_OOM() assert(0 && "OOM")
    #endif

    #ifndef ARENA_ASAN_SEPARATION
        #ifdef ARENA_ASAN_ENABLED
            #define ARENA_ASAN_SEPARATION (8)
        #else 
            #define ARENA_ASAN_SEPARATION (0)
        #endif
    // Must be a power of two
    #endif

    _Static_assert((ARENA_ASAN_SEPARATION & (ARENA_ASAN_SEPARATION-1)) == 0, "Arena ASAN separation must be a power of two"); 

typedef enum {
    ARENA_DEINIT_SUCCESS,
    ARENA_DEINIT_FAILED,
    ARENA_DEINIT_INVALID,
} ArenaDeinitResult;

typedef struct {
    unsigned char* memory;
    ptrdiff_t offset;
    ptrdiff_t length;
    ptrdiff_t reserved_length; 
    ptrdiff_t page_size;
} Arena;

typedef enum {
    ARENA_FLAG_ZEROED = 1,
    ARENA_FLAG_ASAN_SEPARATION = 2,
    ARENA_FLAG_ASAN_POISON = 4,
    //ARENA_FLAG_WRITE_PROTECTED = 8,
} ArenaFlags;

#define arena_new(a, t, n) (t*)arena_alloc_ex(a, sizeof(t), ARENA_FLAG_ZEROED | ARENA_FLAG_ASAN_SEPARATION, _Alignof(t), n)
Arena arena_init(ptrdiff_t size);
Arena arena_from_alloc_memory(void* memory, size_t size); // Creates an arena from passed in memory, do not pass in a static char array (if -fstrict-aliasing)
void* arena_alloc(Arena* arena, size_t size);
void* arena_realloc(Arena* arena, void* old, size_t old_size, size_t new_size);
void* arena_dup(Arena* arena, void* data, size_t size);  
void* arena_alloc_ex(Arena* arena, ptrdiff_t size, ArenaFlags flags, ptrdiff_t align, ptrdiff_t count); // Allocates size bytes with alignment align, and optional zero-initialisation
void arena_reset(Arena* arena); // Reset arena, duh
ptrdiff_t arena_report_allocated_size(Arena arena);
void print_arena(Arena arena);

#ifdef ARENA_IMPLEMENTATION
void* arena_backend_reserve_pages(size_t size, size_t* committed);
void* arena_backend_commit_pages(void* addr, size_t size);
ArenaDeinitResult arena_backend_free_pages(void* addr, size_t size);
ptrdiff_t arena_backend_query_page_size();


Arena arena_from_alloc_memory(void* memory, size_t size) {
    ARENA_ASAN_POISON(memory, size);
    return (Arena){
        .memory = (unsigned char*)memory,
        .offset = 0,
        .length = size,
        .reserved_length = size,
        .page_size = 4096,
    };
}

bool arena_impl_is_power_of_two(size_t align) {
    return (align & (align-1)) == 0;
}

void arena_reset(Arena* arena) {
    ARENA_ASAN_POISON(arena->memory, arena->length);
    arena->offset = 0;
}

#ifdef ARENA_BACKEND_FIXED
    void* arena_backend_reserve_pages(size_t size, size_t* commited) {
        *commited = size;
        return 1; // assume already reserved
    }

    void* arena_backend_commit_pages(void* addr, size_t size) {
        return 0;
    }

    ArenaDeinitResult arena_backend_free_pages(void* addr, size_t size) {
        return 0;
    }
    
    ptrdiff_t arena_backend_query_page_size() {
        return 4096;
    }
#endif // ARENA_BACKEND_FIXED

#ifdef ARENA_BACKEND_PAGEALLOC
    #ifdef _WIN32
        #define ARENA_BACKEND_VIRTUALALLOC
    #endif
    #ifdef __linux__
        #define ARENA_BACKEND_MMAP
    #endif
#endif

#ifdef ARENA_BACKEND_VIRTUALALLOC
    #include <Windows.h>

    void* arena_backend_reserve_pages(size_t size, size_t* committed) {
        *committed = 0;
        return VirtualAlloc(0, (DWORD)size, MEM_RESERVE, PAGE_NOACCESS);
    }

    void* arena_backend_commit_pages(void* addr, size_t size) {
        return VirtualAlloc(addr, (DWORD)size, MEM_COMMIT, PAGE_READWRITE);
    }

    ArenaDeinitResult arena_backend_free_pages(void* addr, size_t size) {
        if (VirtualFree(addr, size, MEM_RELEASE) == 0) {
            return ARENA_DEINIT_FAILED;
        }
        return ARENA_DEINIT_SUCCESS;
    }

    ptrdiff_t arena_backend_query_page_size() {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return info.dwPageSize;
    }
#endif // ARENA_BACKEND_VIRTUALALLOC

#ifdef ARENA_BACKEND_MALLOC
    #include <string.h>
    #include <stdlib.h>
    void* arena_backend_reserve_pages(size_t size, size_t* committed) {
        *committed = size;
        return malloc(size);
    }

    void* arena_backend_commit_pages(void* addr, size_t size) {
        (void)addr; (void)size;
        ARENA_OOM();
        return 0;
    }

    ArenaDeinitResult arena_backend_free_pages(void* addr, size_t size) {
        (void)size;
        free(addr);
        return ARENA_DEINIT_SUCCESS;
    }

    ptrdiff_t arena_backend_query_page_size() {
        return 4096;
    }
#endif // ARENA_BACKEND_MALLOC

#ifdef ARENA_BACKEND_MMAP

#endif // ARENA_BACKEND_MMAP

Arena arena_init(ptrdiff_t size) {
    assert(size > 0);
    size_t length;
    void* memory = arena_backend_reserve_pages(size, &length);
    ptrdiff_t page_size = arena_backend_query_page_size();
    assert(arena_impl_is_power_of_two(page_size));
    if (memory) {
        ARENA_ASAN_POISON(memory, size);
    }
    return (Arena){
        .memory = (unsigned char*)memory,
        .offset = 0,
        .length = length,
        .reserved_length = size,
        .page_size = page_size,
    };
}

void* arena_alloc(Arena* arena, size_t size) {
    return arena_alloc_ex(arena, size, ARENA_FLAG_ZEROED | ARENA_FLAG_ASAN_SEPARATION, 2*sizeof(void*), 1);
}

void* arena_dup(Arena* arena, void* data, size_t size) {
    void* storage = arena_alloc(arena, size);
    memcpy(storage, data, size);
    return storage;
}

void* arena_realloc(Arena* arena, void* old, size_t old_size, size_t new_size) {
    // TODO: reuse and reisze allocation if it is the last one
    void* new_data = arena_alloc(arena, new_size);
    memcpy(new_data, old, old_size);
    return new_data;
}

void* arena_alloc_ex(Arena* arena, ptrdiff_t size, ArenaFlags flags, ptrdiff_t align, ptrdiff_t count)  {
    if (!arena->memory) {
        ARENA_OOM();
    }
    assert(align != 0);
    assert(arena_impl_is_power_of_two(align));
    assert(size != 0);
    assert(count != 0);
    if (ARENA_ASAN_ENABLED) {
        if (align < 8) align = 8;
        if (flags & ARENA_FLAG_ASAN_SEPARATION) {
            arena_alloc_ex(arena, ARENA_ASAN_SEPARATION, ARENA_FLAG_ASAN_POISON, 1, 1);
        }
    }
    ptrdiff_t padding = -(uintptr_t)(&arena->memory[arena->offset]) & (align - 1);
    if (count >= (arena->length - arena->offset - padding)/size) {
        if (count >= (arena->reserved_length - padding)/size) {
            ARENA_OOM();
        }
        arena->length += padding + count*size;
        arena->length = (arena->length+arena->page_size & ~(arena->page_size-1));
        assert(arena->length % arena->page_size == 0);
        //printf("Committed size %lld\n", arena->length);
        arena_backend_commit_pages(arena->memory, arena->length);
    }
    void* r = arena->memory + arena->offset + padding;
    arena->offset += padding + count*size;
    if (!(flags & ARENA_FLAG_ASAN_POISON)) {
        ARENA_ASAN_UNPOISON(r, count*size);
    }
    if (flags & ARENA_FLAG_ZEROED) {
        memset(r, 0, count*size);
    }
    return r;
}

ArenaDeinitResult arena_deinit(Arena* arena) {
    if (arena->memory) {
        arena_reset(arena);
        return arena_backend_free_pages(arena->memory, arena->reserved_length);
    }
    return ARENA_DEINIT_INVALID;
}

ptrdiff_t arena_report_allocated_size(Arena arena) {
    return arena.offset;
}

void print_arena(Arena arena) {
    printf(
        "Arena: memory %p, offset %lld, length %lld, reserved_length %lld\n", 
        arena.memory,
        arena.offset,
        arena.length,
        arena.reserved_length
    );
}


#endif // ARENA_IMPLEMENTATION

#endif // ARENA_H

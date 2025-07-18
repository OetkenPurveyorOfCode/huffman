#define _CRT_SECURE_NO_WARNINGS (1)
#include <stdio.h>
#define ARENA_IMPLEMENTATION
#define ARENA_BACKEND_VIRTUALALLOC
#include "arena.h"
#include <stdint.h>
#include <errno.h>
#define BITWRITER_IMPLEMENTATION
#include "bit_writer.h"
#define BITREADER_IMPLEMENTATION
#include "bit_reader.h"
#define HUFFMAN_IMPLEMENTATION
#include "huffman.h"

char* readfile(Arena* arena, char* path, size_t* len) {
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *string = arena_alloc_ex(arena, fsize+1, ARENA_FLAG_ASAN_SEPARATION, 1, 1);
    fread(string, fsize, 1, f);
    fclose(f);
    string[fsize] = 0;
    *len = fsize;
    return string;
}

int main() {
    Arena arena = arena_init(1000000);
    size_t msg_len = 0;
    char* msg = readfile(&arena, "main.c", &msg_len); 
    if (errno != 0) perror("File read failed\n");
    
    FILE* out = fopen("out2.txt", "wb");
    BitWriterUserdata usrdata = {.f = out };
    BitWriter writer = {
        .write_bit = write_bit,
        .flush = flush,
        .userdata = &usrdata,
    };
    if (!huffman_write(arena, writer, msg, msg_len)) {
        perror("Failed to encode message");
        return -1;
    }
    fclose(out);

    FILE* in = fopen("out2.txt", "rb");
    BitReaderUserdata reader_data = {
        .f = in,
    };
    BitReader reader = {
        .userdata = &reader_data,
        .read_bit = read_bit,
    };
    size_t decoded_len = 0;
    bool ok = true;
    char* decoded = huffman_read(&arena, reader, &decoded_len, &ok);
    //printf("%.*s\n", (int)msg_len, msg);
    printf("%.*s\n", (int)decoded_len, decoded);
    assert(msg_len == decoded_len);
}

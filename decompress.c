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

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: decompress <infile> <outfile>\n");
        return -1;
    }
    char* infile = argv[1];
    char* outfile = argv[2];
    Arena arena = arena_init(1000000);
    FILE* in = fopen(infile, "rb");
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
    //printf("%.*s\n", (int)decoded_len, decoded);
    FILE* f = fopen(outfile, "wb");
    fwrite(decoded, 1, decoded_len, f);
    if (ferror(f)) {
        perror("File save failed\n");
        return -1;
    }
    fclose(f);
}

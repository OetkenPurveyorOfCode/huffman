#pragma once
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    void* userdata;
    bool (*read_bit)(void* userdata, bool* bit);
} BitReader;

typedef struct {
    FILE* f;
    bool buffer[8];
    size_t cursor;
} BitReaderUserdata;

bool read_bit(void* reader_data, bool* bit) {
    BitReaderUserdata* userdata = (BitReaderUserdata*)reader_data;
    if (userdata->cursor) {
        userdata->cursor -= 1;
        *bit = userdata->buffer[userdata->cursor];
        return true;
    }
    else {
        int chr = fgetc(userdata->f);
        if (chr == EOF) return false;
        unsigned char byte = chr;
        for (size_t i = 0; i < 8; i++) {
            userdata->buffer[i] = byte & (1 << i);
        }
        *bit = userdata->buffer[7];
        userdata->cursor = 7;
    }
    return true;
}

bool read_byte(BitReader reader, unsigned char* byte) {
    *byte = 0;
    for (size_t i = 0; i < 8; i++) {
        bool bit = false;
        if (!reader.read_bit(reader.userdata, &bit)) return false;
        *byte |= bit << (7-i);
    }
    return true;
}

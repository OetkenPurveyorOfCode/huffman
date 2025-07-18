#pragma once
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    void* userdata;
    bool (*write_bit)(void*, bool);
    bool (*flush)(void*);
} BitWriter;

typedef struct {
    FILE* f;
    bool buffer[8];
    size_t cursor;
} BitWriterUserdata;

bool write_bit(void* writer_data, bool bit);
bool write_bit_dbg(void* userdata, bool bit);
bool write_byte(BitWriter writer, unsigned char byte);
bool flush(void* writer_data);

#ifdef BITWRITER_IMPLEMENTATION

bool write_bit(void* writer_data, bool bit) {
    BitWriterUserdata* userdata = (BitWriterUserdata*)writer_data;
    if (userdata->cursor < 7) {
        userdata->buffer[userdata->cursor++] = bit;
    }
    else {
        assert(userdata->cursor == 7);
        userdata->buffer[userdata->cursor++] = bit;
        unsigned char byte = 0;
        for (size_t i = 7; i < 8; i--) {
            byte |= ((unsigned char)userdata->buffer[i] << (7-i));
        }
        userdata->cursor = 0;
        return fputc(byte, userdata->f) != EOF;
    }
    return true;
}

bool flush(void* writer_data) {
    BitWriterUserdata* userdata = (BitWriterUserdata*)writer_data;
    if (userdata->cursor != 0) {
        unsigned char byte = 0;
        for (size_t i = userdata->cursor-1; i < userdata->cursor; i--) {
            byte |= ((unsigned char)userdata->buffer[i] << (7-i));
        }
        userdata->cursor = 0;
        return fputc(byte, userdata->f) != EOF;
    }
    return true;
}

bool write_bit_dbg(void* userdata, bool bit) {
    (void)userdata;
    return printf("%c", (char)bit + '0') == 1;
}

bool write_byte(BitWriter writer, unsigned char byte) {
    char result = 0;
    //printf("Writing byte\n");
    result += writer.write_bit(writer.userdata, byte & 0b10000000);
    result += writer.write_bit(writer.userdata, byte & 0b01000000);
    result += writer.write_bit(writer.userdata, byte & 0b00100000);
    result += writer.write_bit(writer.userdata, byte & 0b00010000);
    result += writer.write_bit(writer.userdata, byte & 0b00001000);
    result += writer.write_bit(writer.userdata, byte & 0b00000100);
    result += writer.write_bit(writer.userdata, byte & 0b00000010);
    result += writer.write_bit(writer.userdata, byte & 0b00000001);
    return (result == 8);
}

#endif

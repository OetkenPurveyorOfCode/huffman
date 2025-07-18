#pragma once
#include <stdint.h>
#include "arena.h"
#include "bit_writer.h"
#include "bit_reader.h"


[[nodiscard]] bool huffman_write(Arena arena, BitWriter writer, char* msg, size_t len);
char* huffman_read(Arena* arena, BitReader reader, size_t* len, bool* ok);

#ifdef HUFFMAN_IMPLEMENTATION 
    #ifdef HUFFMAN_IMPLEMENTATION 
    #define HEAPQ_IMPLEMENTATION
    #endif
    #include "heapq.h"

typedef struct Node {
    unsigned char symbol;
    struct Node* left;
    struct Node* right;
    int64_t freq;
} Node;

typedef struct {
    bool code[256];
    size_t len;
} HuffmanTableEntry;

typedef struct {
    HuffmanTableEntry entries[256];
} HuffmanTable;



Ordering cmp_nodes(void* userdata, void* av, void* bv) {
    (void)userdata;
    Node* a = (Node*)av;
    Node* b = (Node*)bv;
    if (a->freq == b->freq) return Ordering_Equal;
    else if (a->freq < b->freq) return Ordering_LessThan;
    else return Ordering_GreaterThan;
}


void print_huffman_table(HuffmanTable* huffman_table) {
    for (size_t i = 0; i < 256; i++) {
        HuffmanTableEntry* entry = &huffman_table->entries[i];
        if (entry->len) {
            printf("%c: ", (unsigned char)i);
            for (size_t i = 0; i < entry->len; i++) {
                printf("%c", entry->code[i] + '0');
            }
            printf("\n");
        }
    }
}

void write_encoded_message(HuffmanTable* huffman_table, BitWriter writer, const unsigned char* msg, size_t msg_len) {
    assert(msg_len < 0xFFFFFFFF);
    write_byte(writer, (msg_len & 0xFF000000) >> (8*3));
    write_byte(writer, (msg_len & 0x00FF0000) >> (8*2));
    write_byte(writer, (msg_len & 0x0000FF00) >> (8*1));
    write_byte(writer, (msg_len & 0x000000FF) >> (8*0));
    //printf("Writing length %zu\n", msg_len);
    for (size_t i = 0; i < msg_len; i++) {
        HuffmanTableEntry* entry = &huffman_table->entries[msg[i]];
        for (size_t i = 0; i < entry->len; i++) {
            writer.write_bit(writer.userdata, entry->code[i]);
        }
    }
}

size_t read_encoded_message_length(BitReader reader) {
    size_t length = 0;
    unsigned char byte = 0;
    read_byte(reader, &byte);
    length |= (size_t)byte << (24U);
    read_byte(reader, &byte);
    length |= (size_t)byte << (16U);
    read_byte(reader, &byte);
    length |= (size_t)byte << (8U);
    read_byte(reader, &byte);
    length |= (size_t)byte << (0U);
    return length;
}

unsigned char read_encoded_message_byte(Node* root, BitReader reader) {
    assert(root);
    if (root->symbol) {
        return root->symbol;
    }
    else {
        bool bit = false;
        read_bit(reader.userdata, &bit);
        if (bit) {
            return read_encoded_message_byte(root->left, reader);
        }
        else {
            return read_encoded_message_byte(root->right, reader);
        }
    }
}

void write_huffman_table(HuffmanTable* table, BitWriter writer) {
    size_t maximum_entry_len = 0;
    size_t entry_count = 0;
    for (size_t i = 0; i < 256; i++) {
        HuffmanTableEntry* entry = &table->entries[i];
        if (entry->len) entry_count += 1;
        if (entry->len > maximum_entry_len) {
            maximum_entry_len = entry->len;
        }
    }
    size_t nobfel /*number_of_bits_for_entry_len*/ = 0;
    while ((1<<nobfel) <= maximum_entry_len) {
        nobfel+=1;
    }
    
    //printf("nobfel %zu\nentry count: %zu\n", nobfel, entry_count);
    write_byte(writer, nobfel);
    write_byte(writer, entry_count);
    for (size_t i = 0; i < 256; i++) {
        HuffmanTableEntry* entry = &table->entries[i];
        if (entry->len) {
            write_byte(writer, i);
            //printf("Writing len %zu for %c\n", entry->len, (unsigned char)i);
            for (size_t i = nobfel-1; i < nobfel; i--) {
                //printf(".%c. ", (bool)(entry->len & (1 << i)) + '0');
                writer.write_bit(writer.userdata, entry->len & (1 << i));
            }
            //printf("\nWriting data for %c\n", (unsigned char)i);
            for (size_t i = 0; i < entry->len; i++) {
                writer.write_bit(writer.userdata, entry->code[i]);
            }
            //printf("\n");
        }
    }
}

void read_huffman_table(HuffmanTable* table, BitReader reader) {
    unsigned char nobfel = 0;
    read_byte(reader, &nobfel);
    //printf("number of bits per entry len %d\n", nobfel);
    unsigned char entry_count = 0;
    read_byte(reader, &entry_count);
    //printf("entry count %d\n", entry_count);
    for (size_t entry_it = 0; entry_it < entry_count; entry_it++) {
        unsigned char symbol = 0;
        read_byte(reader, &symbol);
        
        HuffmanTableEntry* entry = &table->entries[symbol];
        size_t length = 0;
        for (size_t biti = 0; biti < nobfel; biti++) {
            bool bit = 0;
            reader.read_bit(reader.userdata, &bit);
            length |= bit << (nobfel-1-biti);
            //printf("Reading %zu bit .%c.\n", length, bit + '0');
        }
        //printf("Reading symbol `%c` len %zu\n", symbol, length);
        HuffmanTableEntry new_entry = {
            .len = length,
        };
        for (size_t i = 0; i < length; i++) {
            bool bit = 0;
            reader.read_bit(reader.userdata, &bit);
            new_entry.code[i] = bit;
        }
        *entry = new_entry;
    }
}

void printtree(Node* root, size_t indent) {
    if (root == 0) return;
    for (size_t i = 0; i < indent; i++) putchar(' ');
    printf("%c (%lld)\n", root->symbol, root->freq);
     
    printtree(root->left, indent+1);
    printtree(root->right,indent+1);
}

Node* huffmantree_from_table(Arena* arena, HuffmanTable* table) {
    Node* root = arena_new(arena, Node, 1);
    for (size_t i = 0; i < 256; i++) {
        HuffmanTableEntry* entry = &table->entries[i];
        if (entry->len) {
            Node* node = root;
            for (size_t i = 0; i < entry->len; i++) {
                if (entry->code[i] == true) {
                    if (!node->left) node->left = arena_new(arena, Node, 1);
                    node = node->left;
                }
                else {
                    if (!node->right) node->right = arena_new(arena, Node, 1);
                    node = node->right;
                }
            }
            node->symbol = i;
        }
    }
    return root;
}

void huffman_table_from_tree(Node* root, HuffmanTable* huffman_table, unsigned char* code, size_t entry_len) {
    if (!root) return;
    if (root->left) {
        code[entry_len] = true;
        huffman_table_from_tree(root->left, huffman_table, code, entry_len+1);
    }
    if (root->right) {
        code[entry_len] = false;
        huffman_table_from_tree(root->right, huffman_table, code, entry_len+1);
    }
    if (root->symbol) {
        HuffmanTableEntry entry = {.len=entry_len};
        memcpy(entry.code, code, entry_len);
        huffman_table->entries[root->symbol] = entry;
    }
}

[[nodiscard]] bool huffman_write(Arena arena, BitWriter writer, char* msg_in, size_t msg_len) {
    unsigned char* msg = (unsigned char*)msg_in;
    int64_t frequencies[256] = {0};
    for (size_t i = 0; i < msg_len; i++) {
        frequencies[(unsigned char)msg[i]] += 1;
    }
    
    Node nodes[256] = {0};
    HeapQueue heapq = {
        .arena = &arena,
        .cmp_cb = cmp_nodes,
        .elem_size = sizeof(Node),
    };
    for (size_t i = 0; i < 256; i++) {
        nodes[i] = (Node){.freq = frequencies[i], .symbol = i};
        if (nodes[i].freq) {
            hq_push(&heapq, &nodes[i]);
        }
    }
    Node* nodes2 = (Node*)heapq.data;
    while (heapq.len > 1) {
        Node* node1 = (Node*)hq_pop(&heapq);
        Node* node2 = (Node*)hq_pop(&heapq);
        Node new_node = (Node){
            .freq = node1->freq + node2->freq,
            .symbol = '\0',
            .left = arena_dup(&arena, node1, sizeof(Node)),
            .right = arena_dup(&arena, node2, sizeof(Node)),
        };
        hq_push(&heapq, &new_node);
    }
    Node* root = (Node*)hq_pop(&heapq);
    HuffmanTable huffman_table = {0};
    unsigned char code[256];
    huffman_table_from_tree(root, &huffman_table, code, 0);
    
    write_huffman_table(&huffman_table, writer);
    write_encoded_message(&huffman_table, writer, msg, msg_len);
    write_byte(writer, 0xFF);
    writer.flush(writer.userdata);
    return true;
}

char* huffman_read(Arena* arena, BitReader reader, size_t* len, bool* ok) {
    HuffmanTable read_table = {0};
    read_huffman_table(&read_table, reader);
    //print_huffman_table(&read_table);
    size_t length = read_encoded_message_length(reader);
    char* buffer = arena_new(arena, char, length);
    {
        Arena scratch = *arena;
        Node* rroot = huffmantree_from_table(&scratch, &read_table);
        //printtree(rroot, 0);
        for (size_t i = 0; i < length; i++) {
            buffer[i] = read_encoded_message_byte(rroot, reader);
        }
        *len = length;
    }
    return buffer;
}

#endif

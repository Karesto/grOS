
#include "filesystem.h"

#include "../drivers/disk.h"
#include "../libc/mem.h"

#include <stdint.h>

static const uint16_t CHAR_PER_BLOCK = 506U; // 512 - 5 char header

uint16_t free_first;
uint16_t free_last;
uint16_t free_blocks;

void write_block0() {
    disk_buffer[0] = free_first >> 8;
    disk_buffer[1] = free_first & 0xff;
    disk_buffer[0] = free_last >> 8;
    disk_buffer[1] = free_last & 0xff;
    disk_buffer[0] = free_blocks >> 8;
    disk_buffer[1] = free_blocks & 0xff;
    d_write(0);
}

uint16_t free_block() {
    uint16_t free_b = free_first;
    --free_blocks;
    d_read(free_b);
    free_first = (((uint16_t)disk_buffer[1]) << 8) | disk_buffer[2];
    return free_b;
}

uint32_t file_new(uint16_t parent, uint64_t size, char * contents, char has_ancestor) {
    uint16_t addr = free_block();
    uint8_t m = CHAR_PER_BLOCK;
    disk_buffer[0] = 1 | ((size > CHAR_PER_BLOCK) << 1) | (has_ancestor << 2);
    disk_buffer[3] = parent >> 8;   // prev_high
    disk_buffer[4] = parent & 0xff; // prev_low
    if (size <= CHAR_PER_BLOCK) {
        disk_buffer[1] = 0;    // next_high
        disk_buffer[2] = size; // next_low
        m = size;
    }
    else {
        disk_buffer[1] = free_first >> 8;   // next_high
        disk_buffer[2] = free_first & 0xff; // next_low
    }
    for (uint8_t x = 0; x != m; ++x)
        disk_buffer[x + 5] = contents[x];
    d_write(addr);
    if (size <= CHAR_PER_BLOCK) {
        write_block0();
        return ((uint32_t)(addr) << 8) | addr;
    }
    return ((uint32_t)(addr) << 8) | (file_new(addr, size - CHAR_PER_BLOCK, &(contents[CHAR_PER_BLOCK]), 1) & 0xffff);
}

uint16_t file_write(uint16_t file, uint64_t size, char *contents) {
    d_read(file);
    uint8_t info = disk_buffer[0];
    uint16_t next = (((uint16_t)disk_buffer[1]) << 8) | disk_buffer[2];
    uint8_t m = CHAR_PER_BLOCK;
    disk_buffer[0] = 0b101 | ((size > CHAR_PER_BLOCK) << 1);
    // disk 3-4 unchanged (parent)
    if (size <= CHAR_PER_BLOCK) {
        disk_buffer[1] = 0;    // next_high
        disk_buffer[2] = size; // next_low
        m = size;
    }
    else {
        disk_buffer[1] = free_first >> 8;   // next_high
        disk_buffer[2] = free_first & 0xff; // next_low
    }
    for (uint8_t x = 0; x != m; ++x)
        disk_buffer[x + 5] = contents[x];
    d_write(file);
    if (size <= CHAR_PER_BLOCK) {
        if (info & 2) // next file exists
            file_delete(next);
        return file;
    }
    if (info & 2) // next file exists
        return file_write(next, size - CHAR_PER_BLOCK, &(contents[CHAR_PER_BLOCK]));
    return (uint16_t) (file_new(free_first, size - CHAR_PER_BLOCK, &(contents[CHAR_PER_BLOCK]), 1) & 0xffff);
}

uint16_t file_append(uint16_t file, uint64_t size, char *contents) {
    d_read(file);
    uint16_t next = (((uint16_t)disk_buffer[1]) << 8) | disk_buffer[2]; //size used
    uint8_t m = CHAR_PER_BLOCK - next;
    disk_buffer[0] = 0b101 | ((size > m) << 1);
    // disk 3-4 unchanged (parent)
    if (size <= m) {
        disk_buffer[1] = 0;    // next_high
        disk_buffer[2] = next + size; // next_low
        m = size;
    }
    else {
        disk_buffer[1] = free_first >> 8;   // next_high
        disk_buffer[2] = free_first & 0xff; // next_low
    }
    for (uint8_t x = 0; x != m; ++x)
        disk_buffer[x + 5] = contents[x + next];
    d_write(file);
    if (size <= m) {
        return file;
    }
    return (uint16_t) (file_new(free_first, size - m, &(contents[CHAR_PER_BLOCK]), 1) & 0xffff);
}

void file_delete(uint16_t file) {
    uint8_t info = 0;
    uint16_t addr = file;
    do {
        d_read(addr);
        info = disk_buffer[0];
        disk_buffer[0] = 0;
        d_write(addr);
        addr = (((uint16_t)disk_buffer[1]) << 8) | disk_buffer[2];
        ++free_blocks;
    } while (info & 2);
    d_read(free_last);
    disk_buffer[1] = file >> 8;
    disk_buffer[2] = file & 0xff;
    d_write(free_last);
    free_last = addr;
    write_block0();
}

char* file_read(uint16_t file, uint64_t size) {
    uint8_t m = CHAR_PER_BLOCK;
    uint8_t info = 0;
    char* contents = malloc(sizeof(char) * size);
    uint16_t next = file;
    uint64_t offset = 0;
    do {
        d_read(next);
        info = disk_buffer[0];
        uint16_t next = (((uint16_t)disk_buffer[1]) << 8) | disk_buffer[2];
        if (!(info & 2))
            m = next;
        if (offset + m > size)
            return 0; // error, size too small
        memcpy((char*)(disk_buffer + 5), (char*)(contents + offset), m);
        offset += CHAR_PER_BLOCK;
    } while (info & 2);
    return contents;
}



#ifndef DISK_H
#define DISK_H

#include <stdint.h>

char disk_buffer[512];

void init_disk(void);
void d_read(uint32_t addr);
void d_write(uint32_t addr);

#endif

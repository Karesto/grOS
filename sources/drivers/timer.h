#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void init_timer(uint32_t freq);
uint64_t get_time();

void wait(uint16_t delay);

#endif
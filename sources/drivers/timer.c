
#include "timer.h"

#include "../cpu/ports.h"
#include "../cpu/interrupts.h"
#include "../core/schedule.h"
#include <stdint.h>

#define ignore(x) (void)(x) //enleve un warning, pas d'autre interet...

static uint64_t tick = 0;

static void timer_callback(registers_t regs) {
    ignore(regs);
    ++tick;
    if (tick % 100== 0) {
      tick = 0;
      //// putcar('.');
      //kprintf("%d ", tick);
    }
    schedule();
}

void init_timer(uint32_t freq) {
    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)( (divisor >> 8) & 0xFF);
    /* Send the command */
    port_byte_out(0x43, 0b00110110); /* Command port */
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

uint64_t get_time() {
    return tick;
}

void wait(uint16_t delay) {
    uint64_t b = get_time() + delay;
    while (b > get_time());
}

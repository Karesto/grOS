#include "schedule.h"

void isr_clock_int(void)
{
    static int tic = 0;
    static int sec = 0;
    tic++;
    if (tic%100 == 0) {
        sec++;
        tic = 0;
    }
    schedule();
}

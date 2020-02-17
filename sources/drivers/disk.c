// gere le disque dur

#include "disk.h"

#include "../libc/iostream.h"
#include "../cpu/ports.h"
#include "../drivers/timer.h"

#include <stdint.h>

static const uint16_t disk_port   = 0x1f0;
static const uint16_t status_port = 0x1f7;

static const uint8_t disk_flush = 0xe7;
static const uint8_t disk_read  = 0x20;
static const uint8_t disk_write = 0x30;

static uint16_t info[256];
static uint32_t drive_size = 0;

void init_disk(void) {
    if (0xff == port_byte_in(status_port))
        return kprintf("\033[4f]Error:\033[_] status register is 0xff, no disk\n");
    //identify
    port_byte_out(0x1f6, 0xA0);
    io_wait();
    for (uint8_t x = 2; x != 6; ++x)
        port_byte_out(disk_port + x, 0);
    port_byte_out(status_port, 0xEC);
    if (port_byte_in(status_port) == 0)
        return kprintf("\033[4f]Error:\033[_] status register is 0, disk does not exist\n");
    while (port_byte_in(status_port) & (1 << 7));
    if (port_byte_in(0x1f4) | port_byte_in(0x1f5))
        return kprintf("\033[4f]Error:\033[_] drive is not ATA, whatever that means...\n");
    while (!(port_byte_in(status_port) & 0b1001));
    if (port_byte_in(status_port) & 1)
        return kprintf("\033[4f]Error:\033[_] disk error... Infinite sadness :-(\n");
    for (uint16_t x = 0; x != 256; ++x) {
        info[x] = port_word_in(disk_port);
    }
    drive_size =  (((uint32_t)info[61]) << 16) + info[60];
    kprintf("Disk detected: %d sectors (%d o)\n", drive_size, drive_size*256*2);
    //d_write(1);
    //d_write(2);
    //kprintf("Written\n");
    //d_read(1);
    //io_wait();
    //kprintf("\n");
    //d_read(2);
}

char wait_for_poll() {
    unsigned char status = 0;
    do {
        status = port_byte_in(status_port);
        io_wait();
        //kprintf("%b\n", port_byte_in(status_port));
        if (status & 0b100001) {
            return 1; // error
        }
    } while ((status & 0x80) | (!((status >> 3) & 1))); // while BSY
    return 0;
}

void prepare_IO(uint32_t addr, uint8_t seccount) {
    /*if (wait_for_poll())
        return kprintf("\033[4f]Error:\033[_] when preparing disk IO\n");
    uint32_t LBA = addr & 0x1ffffff;
    port_byte_out(0x1F6, 0xE0 | ((LBA >> 24) & 0x0F));
    port_byte_out(0x1F1, 0x00);
    port_byte_out(0x1F2, seccount); // sectorcount
    port_byte_out(0x1F3, (unsigned char) LBA);
    port_byte_out(0x1F4, (unsigned char)(LBA >> 8));
    port_byte_out(0x1F5, (unsigned char)(LBA >> 16));
    port_byte_out(0x1f6, 0xA0);
    io_wait();*/
    uint32_t LBA = addr & 0x1ffffff; // only use 24 first bits
    //kprintf("Sector %d\n", LBA);
    port_byte_out(0x1F6, 0x40);
    port_byte_out(0x1F2, seccount >> 8); // sectorcount    
    port_byte_out(0x1F3, ((LBA >> 24) & 0xFF));
    port_byte_out(0x1F4, 0x00);
    port_byte_out(0x1F5, 0x00);
    port_byte_out(0x1F2, seccount & 0xff);
    port_byte_out(0x1F3, (unsigned char) (LBA & 0xff));
    port_byte_out(0x1F4, (unsigned char)((LBA >> 8) & 0xff));
    port_byte_out(0x1F5, (unsigned char)((LBA >> 16) & 0xff));
}

void d_read(uint32_t addr) {
    prepare_IO(addr, 0);
    port_byte_out(status_port, disk_read);
    if (wait_for_poll())
        return kprintf("\033[4f]Error:\033[_] when reading from disk\n");
    for (uint16_t x = 0; x != 256; ++x) {
        ((uint16_t*)disk_buffer)[x] = port_word_in(disk_port);
        //io_wait();
    }
    /*for (uint16_t x = 0; x != 256; ++x) {
        kprintf("%d ", ((uint16_t*)disk_buffer)[x]);
    }*/
    if (wait_for_poll())
        return kprintf("\033[4f]Error:\033[_] after reading from disk\n");
}

void d_write(uint32_t addr) {
    prepare_IO(addr, 0);
    port_byte_out(status_port, disk_write);
    if (wait_for_poll())
        return kprintf("\033[4f]Error:\033[_] when writing to disk\n");
    //kprintf("%b\n", port_byte_in(status_port));
    for (uint16_t x = 0; x != 256; ++x) {
        port_word_out(disk_port, 3); //(((uint16_t)disk_buffer[2*x]) << 8) | disk_buffer[2*x+1]);
        io_wait();
        //kprintf("%d ", out_buffer[x]);
    }
    if (wait_for_poll())
        return kprintf("\033[4f]Error:\033[_] after writing to disk\n");
    port_byte_out(status_port, disk_flush);
    if (wait_for_poll())
        return kprintf("\033[4f]Error:\033[_] after flushing to disk\n");
}

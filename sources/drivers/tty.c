//voir doc dans tty.h
#include "tty.h"

#include "../cpu/ports.h"
#include "../libc/buffer.h"

#include <stdint.h>
#include <stdarg.h>

static char INSERT_MODE = 0; // 1 for insert, 0 for replace

inline static size_t min(size_t a, size_t b) { return a < b ? a : b; }
inline static size_t max(size_t a, size_t b) { return a > b ? a : b; }

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
 
// formate caractere+couleur pour l'impression
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

uint8_t row_length[VGA_HEIGHT];

uint16_t scroll_up_table[BUFFER_SIZE];
uint16_t scroll_down_table[BUFFER_SIZE];
buffer_t scroll_up_buffer;
buffer_t scroll_down_buffer;

static char terminal_awaits_ansi;
static size_t ansi_value;

void update_cursor() {
    unsigned short location=(terminal_row*80) + terminal_column;/* Short is a 16bit type , the formula is used here*/

    //Cursor Low port
    port_byte_out(0x3D4, 0x0F);//Sending the cursor low byte to the VGA Controller
    port_byte_out(0x3D5, (unsigned char)(location & 0xFF));

    //Cursor High port
    port_byte_out(0x3D4, 0x0E);//Sending the cursor high byte to the VGA Controller
    port_byte_out(0x3D5, (unsigned char)((location >> 8) & 0xFF)); //Char is a 8bit type
}

void terminal_clearbuffers(void) {
    scroll_up_buffer = buffer_new(scroll_up_table, BUFFER_SIZE);
    scroll_down_buffer = buffer_new(scroll_down_table, BUFFER_SIZE);
}
void terminal_clearscreen(void) {
	for (size_t y = 0; y < VGA_HEIGHT; ++y) {
		for (size_t x = 0; x < VGA_WIDTH; ++x) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
        row_length[y] = 0;
    }
	terminal_row = 0;
	terminal_column = 0;
}
void terminal_clearline(void) {
    size_t t = terminal_row * VGA_WIDTH;
    for (size_t x = t; x < t + VGA_WIDTH; ++x) {
		terminal_buffer[x] = vga_entry(' ', terminal_color);
	}
    row_length[terminal_row] = 0;
    terminal_column = 0;
    update_cursor();
}

void terminal_setcolor(enum vga_color fg, enum vga_color bg) {
    terminal_color = vga_entry_color(fg, bg);
}
void terminal_resetcolor(void) {
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void terminal_initialize(void) {
    terminal_resetcolor();
    terminal_buffer = (uint16_t*) 0xB8000;
    terminal_awaits_ansi = 0;
    ansi_value = 0;
    terminal_clearscreen();
    terminal_clearbuffers();
    update_cursor();
}

void scroll_up(const size_t bottom_row, const uint8_t n) { 
    // scrolls up by one row for all rows >= bottom_row
    // clears bottom_row
    size_t row = 0;
    size_t next_row = VGA_WIDTH*n;
    size_t limit = max((bottom_row +1) * VGA_WIDTH, next_row);
    while (next_row != limit)
        terminal_buffer[row++] = terminal_buffer[next_row++];
    for (size_t s = 0; s != bottom_row - n + 1; ++s) // update row lengths
        row_length[s] = row_length[s + n];
    for (size_t s = bottom_row - n + 1; s != bottom_row + 1; ++s)
        row_length[s] = 0;
    while (row != next_row) // clear last line
        terminal_buffer[row++] = vga_entry(' ', terminal_color);
}

void scroll_down(const size_t top_row, const uint8_t n) { 
    // scrolls up by one row for all rows >= bottom_row
    // clears bottom_row
    size_t row = VGA_HEIGHT * VGA_WIDTH - 1;
    size_t next_row = row - VGA_WIDTH*n;
    size_t limit = min(top_row * VGA_WIDTH - 1, next_row);
    while (next_row != limit)
        terminal_buffer[row--] = terminal_buffer[next_row--];
    for (int s = top_row; s < VGA_HEIGHT - n; ++s) // update row lengths
        row_length[s + n] = row_length[s];
    for (size_t s = top_row; s != top_row + n; ++s)
        row_length[s] = 0;
    while (row != next_row) // clear last lines
        terminal_buffer[row--] = vga_entry(' ', terminal_color);
}

void terminal_newline(void) {
    if (INSERT_MODE == 0) { //overwrite, don't use buffers
        if (++terminal_row == VGA_HEIGHT) {
            --terminal_row; // last line
            // move text up one line
            scroll_up(terminal_row, 1);
        }
    }
    else if (INSERT_MODE == 1) { // insertion
        if (++terminal_row == VGA_HEIGHT) {
            --terminal_row; // last line
            // move text up one line
            buffer_add(scroll_up_buffer, terminal_buffer, row_length[0]);
            scroll_up(terminal_row, 1);
        }            
        else if ((!buffer_isempty(scroll_down_buffer)) | row_length[VGA_HEIGHT - 1]) {
            buffer_add(scroll_down_buffer, terminal_buffer + (VGA_HEIGHT - 1) * VGA_WIDTH, row_length[VGA_HEIGHT - 1]);
            scroll_down(terminal_row, 1);
        }
        else {
            scroll_down(terminal_row, 1);
        }
    }
    terminal_column = 0;
    update_cursor();
}

uint16_t terminal_getcursor() {
    return (terminal_row << 8) | terminal_column;
}

void terminal_movecursor(size_t row, size_t column) {
    terminal_row = row;
    terminal_column = column;
    update_cursor();
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
    if (row_length[y] <= x)
        row_length[y] = x;
    else if (INSERT_MODE == 1) {
        if (++row_length[y] == VGA_WIDTH) {
            --row_length[y];
            char c = terminal_row + 1 == VGA_HEIGHT;
            terminal_newline();
            if (c) {
                terminal_column = x;
                y--;
            }
            terminal_buffer[terminal_row * VGA_WIDTH] = terminal_buffer[terminal_row * VGA_WIDTH - 1];
            terminal_row--;
        }
        for (size_t t = y * VGA_WIDTH + row_length[y] - 1; t != index - 1; --t)
            terminal_buffer[t + 1] = terminal_buffer[t];
    }
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(const char c) {
    if (terminal_awaits_ansi) {
        if ('0' <= c && c <= '9') {// decimal
            ansi_value = ansi_value * 10 + (c - '0');
            return;
        }
        switch (c) {
        case '[': return;
        case ']': terminal_awaits_ansi = 0; return;
        case ':': terminal_clearscreen(); return;
        case '.': terminal_clearline(); return;
        case '_': terminal_resetcolor(); return;
        case 'f':
            terminal_color = (terminal_color & 0b11110000) | ansi_value % NB_COLORS;
            ansi_value = 0;
            return;
        case 'b':
            terminal_color = (terminal_color & 0b00001111) | (ansi_value % NB_COLORS) << 4;
            ansi_value = 0;
            return;
        case 'y':
            terminal_row = ansi_value % VGA_HEIGHT;
            ansi_value = 0;
            update_cursor();
            return;
        case 'x':
            terminal_column = ansi_value % VGA_WIDTH;
            ansi_value = 0;
            update_cursor();
            return;
        case 'u':
            terminal_row = max(0, terminal_row - ansi_value);
            ansi_value = 0;
            update_cursor();
            return;
        case 'd':
            terminal_row = min(VGA_HEIGHT - 1, terminal_row + ansi_value);
            ansi_value = 0;
            update_cursor();
            return;
        case 'l':
            terminal_column = max(0, terminal_column - ansi_value);
            ansi_value = 0;
            update_cursor();
            return;
        case 'r':
            terminal_column = min(VGA_WIDTH - 1, terminal_column + ansi_value);
            ansi_value = 0;
            update_cursor();
            return;
        default:
            //unrecognised character, exits ansi mode
            terminal_awaits_ansi = 0;
        }
    }
    switch (c) {
    case '\n':
    case '\r':
        terminal_newline();
        return;
    case '\t':
        {
        size_t index = terminal_row * VGA_WIDTH + terminal_column;
        uint16_t blank = vga_entry(' ', terminal_color);
        do {
            terminal_buffer[index++] = blank;
            if (++terminal_column == VGA_WIDTH)
                terminal_newline();
        } while (terminal_column % 4 != 0);
        }
        break;
    case '\033':
        terminal_awaits_ansi = 1;
        ansi_value = 0;
        break;
    default:
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH)
            terminal_newline();
        break;
    }
    update_cursor();
}

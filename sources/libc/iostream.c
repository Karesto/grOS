/*
Module printer, gère l'impression sur le terminal.
Voir documentation dans printer.h
*/

#include <stdint.h>
#include <stdarg.h>

#include "iostream.h"
#include "../drivers/keyboard.h"
#include "../drivers/tty.h"

static const uint16_t IN_BUFFER_SIZE = 800;

static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void print_unsigned(unsigned long long int c, unsigned char base, unsigned char padding, unsigned char pad_with_zeros) {
    char pad_char = pad_with_zeros == 1 ? '0' : ' ';
    if (c == 0) {
        if (padding != 0)
            for (unsigned char x = 0; x < padding - 1; ++x)
                terminal_putchar(pad_char);
        terminal_putchar('0');
    }
    else {
        unsigned char nb = 1;
        unsigned long long int x = 1;
        unsigned long long int y = base;
        while (y <= c && y > x) {
            ++nb;
            x = y;
            y *= base;
        }
        while (nb <= padding - 1) {
            terminal_putchar(pad_char);
            ++nb;
        }
        while (x) {
            terminal_putchar(digits[c / x]);
            c %= x;
            x /= base;
        }
    }
}

void print_int(long long int c, unsigned char base, unsigned char padding, unsigned char pad_with_zeros) {
    if (c < 0) {
        terminal_putchar('-');
        c *= -1;
        if (padding != 0)
            --padding;
    }
    print_unsigned((unsigned long long int)c, base, padding, pad_with_zeros);
}

unsigned char get_base(char c) {
    switch (c) {
        case 'd': return 10;
        case 'x': return 16;
        case 'o': return 8;
        default:
            break;
    }
    return 2;
}

void print_string(const char * chaine) {
    size_t indice = 0;
    while(chaine[indice]){
        terminal_putchar(chaine[indice++]);
    }
}

void kprintf(const char * chaine, ...)
{
    va_list args;
    va_start(args, chaine);
    size_t indice = 0;
    while (chaine[indice])
    {
        if (chaine[indice] == '%')
        {
            ++indice;
            unsigned char flags = 0;
            unsigned char padding = 0;
            unsigned char leading_zero = 0;
            if (chaine[indice] == '0') {
                leading_zero = 1;
                ++indice;
            }
            while (chaine[indice] >= '0' && chaine[indice] <= '9') {
                padding *= 10;
                padding += chaine[indice++] - '0';
            }
            // bit 0 -> long ?
            // bit 1 -> unsigned ?
            if (chaine[indice] == 'l') {
                flags = 1;
                if (chaine[++indice] == 'u') {
                    flags = 3;
                    ++indice;
                }
            }
            if (chaine[indice] == 'u') {
                flags = 2;
                if (chaine[++indice] == 'l') {
                    flags = 3;
                    ++indice;
                }
            }
            switch (chaine[indice])
            {
            case 0:
                terminal_putchar('%');
                va_end(args);
                return;
            case 'd':
            case 'x':
            case 'o':
            case 'b':
                {
                    unsigned char base = get_base(chaine[indice]);
                    long long int c;
                    if (flags & 1)
                        c = va_arg(args, long long int);
                    else
                        c = (long long int)(va_arg(args, int));
                    if (flags & 2)
                        print_unsigned((unsigned long long int)c, base, padding, leading_zero);
                    else
                        print_int(c, base, padding, leading_zero);
                }
                break;
            case 'c':
                if (padding == 0)
                    terminal_putchar((char)va_arg(args, int));
                else {
                    char c = (char)va_arg(args, int);
                    for (unsigned char x = 0; x != padding; ++x)
                        terminal_putchar(c);
                }
                break;
            case 's':
                if (padding == 0)
                    print_string(va_arg(args, char *));
                else {
                    char* c = va_arg(args, char *);
                    for (unsigned char x = 0; x != padding; ++x)
                        print_string(c);
                }
                break;
                break;
            case '%':
                terminal_putchar('%');
                break;
            default:
                terminal_putchar('%');
                terminal_putchar(chaine[indice]);
            }
            
        }
        else
            terminal_putchar(chaine[indice]);
        ++indice;
    }
    va_end(args);
    return;
}

char in_buffer[800];
uint16_t in_cursor;
uint16_t in_buffer_end;
uint8_t initial_x, initial_y;
char volatile is_reading;

char * readline() {
    initial_y = terminal_getcursor() >> 8;
    initial_x = terminal_getcursor() & 0xff;
    is_reading = 1;
    in_cursor = 0;
    in_buffer_end = 0;
    while (is_reading);
    terminal_newline();
    return in_buffer;
}

void print_inbuffer() {
    terminal_movecursor(initial_y, initial_x);
    uint8_t x = initial_x;
    uint8_t y = initial_y;
    uint8_t cx = initial_x;
    uint8_t cy = initial_y;
    uint8_t c = 0;
    while (c != in_buffer_end) {
        char t = in_buffer[c];
        terminal_putchar(t);
        ++c;
        do {
            if (++x == 80) {
                x = 0;
                if (++y == 25) {
                    --initial_y;
                    --y;
                    --cy;
                }
            }
        } while ((t == '\t') && ((x % 4) != 0));
        if (c == in_cursor) {
            cx = x;
            cy = y;
        }
    }
    terminal_movecursor(cy, cx);
}

void in_buffer_delete() {
    for (uint16_t x = in_cursor; x < in_buffer_end - 1; ++x)
        in_buffer[x] = in_buffer[x + 1];
    if (in_buffer_end > in_cursor)
        in_buffer[in_buffer_end - 1] = 0;
    print_inbuffer(); // printing 0 to erase last char
    if (in_buffer_end > in_cursor)
        --in_buffer_end;
}

void in_buffer_add(char a) {
    if (++in_buffer_end == IN_BUFFER_SIZE-1) {
        --in_buffer_end;
        return;
    }
    for (uint16_t x = in_buffer_end + 1; x > in_cursor; --x) {
        in_buffer[x - 1] = in_buffer[x - 2];
    }
    in_buffer[in_cursor++] = a;
    print_inbuffer();
}

void key_pressed(uint32_t keycode) {
    if (is_reading) {
      if (keycode_ispressed(keycode)) {
		if (keycode_isascii(keycode)) {
			in_buffer_add(keycode & 0xff);		
		}
		else {
		  switch ((char)keycode) {
			case 1: {// F1
					char mode = 1 ^ keyboard_get();
					/*if (mode)
						kprintf("Passage en mode AZERTY\n");
					else 
						kprintf("Passage en mode QWERTY\n");*/
					return keyboard_set(mode);					
				}
				break;
			case 30: // entree
                in_buffer[in_cursor] = 0;
                is_reading = 0;
                return;
            case 32: // espace
				return in_buffer_add(' ');
			case 29: // Tab
				return in_buffer_add('\t');
			//case 40: // Flch H
			//  return kprintf("\033[1u]");
			case 41: // Flch G
                if (in_cursor != 0)
                    --in_cursor;
                return print_inbuffer();
            case 42: // Flch D
                if (in_cursor != in_buffer_end)
                    ++in_cursor;
                return print_inbuffer();
			//case 43: // Flch B
			//  return kprintf("\033[1d]");
			case 46: // Debut
                in_cursor = 0;
                return print_inbuffer();
            case 47: // Fin
                in_cursor = in_buffer_end;
                return print_inbuffer();
            case 26: // Backspace
                if (in_cursor > 0) {
                    --in_cursor;
                    in_buffer_delete();
                } 
                return;
            case 27: // Suppr
                return in_buffer_delete();
            default:
                return;
          }
		}
      }
    }
    else {
	  if (keycode_ispressed(keycode)) {
		if (keycode_isascii(keycode))
		{
			keycode_print(keycode);		
		}
		else {
		  switch ((char)keycode) {
			case 1: {// F1
					char mode = 1 ^ keyboard_get();
					if (mode)
						kprintf("Passage en mode AZERTY\n");
					else 
						kprintf("Passage en mode QWERTY\n");
					return keyboard_set(mode);					
				}
				break;
			case 30: // entree
				return kprintf("\n");
			case 32: // espace
				return kprintf(" ");
			case 29: // Tab
				return kprintf("\t");
			case 40: // Flch H
			  return kprintf("\033[1u]");
			case 41: // Flch G
			  return kprintf("\033[1l]");
			case 42: // Flch D
			  return kprintf("\033[1r]");
			case 43: // Flch B
			  return kprintf("\033[1d]");
			case 44: // Pg Up
			  return kprintf("\033[0y]");
			case 45: // Pg Dw
                return kprintf("\033[%dy]", VGA_HEIGHT - 1);
			case 46: // Home
			  return kprintf("\033[0x]");
			case 47: // End
                return kprintf("\033[%dy]", VGA_WIDTH - 1);
            case 26: // Backspace
                return kprintf("\033[1l] \033[1l]");
            case 27: // Suppr
                return kprintf(" \033[1l]");
            default:
                return;
                //keycode_print(keycode);
          }
		}
	  }
	}
}


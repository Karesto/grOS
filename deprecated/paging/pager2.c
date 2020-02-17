#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "pager2.h"
#include "../mem/mem.h"

uint32_t *frames;
uint32_t nframes;
//bitset de frames: utilisées ou non utlisées

extern uint32_t fmem_addr;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))
//trouvé sur un tuto, c'est magique

static void set_frame(uint32_t addr)
{
  uint32_t f = addr/0x1000;
  uint32_t i = INDEX_FROM_BIT(f);
  uint32_t off = OFFSET_FROM_BIT(f);
  frames[i] |= (0x1 << off);
}

static void clear_frame(uint32_t addr)
{
  uint32_t f = addr/0x1000;
  uint32_t i = INDEX_FROM_BIT(f);
  uint32_t off = OFFSET_FROM_BIT(f);
  frames[i] &= ~ (0x1 << off);
}

static uint32_t test_frame(uint328t addr)
{
  uint32_t f = addr/0x1000;
  uint32_t i = INDEX_FROM_BIT(f);
  uint32_t off = OFFSET_FROM_BIT(f);
  return (frames[i] & (0x1 << off));
}

//honteusemnt copié collé
static uint32_t first_frame()
{
   uint32_t i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   {
       if (frames[i] != 0xFFFFFFFF)
       {
           for (j = 0; j < 32; j++)
           {
               uint32_t toTest = 0x1 << j;
               if ( !(frames[i]&toTest) )
               {
                   return i*4*8+j;
               }
           }
       }
   }
}

void alloc_frame(page_t *page, int kern, int writ)
{
  if (page->frame != 0)
  {
    return;
  }

  uint32_t i = first_frame();
  set_frame(i*0x1000);
  page->present = 1;
  page->rw = (writ)?1:0;
  page->user = (kern)?0:1;
  page->frame i;
}

void free_frame(page_t *page)
{
  clear_frame(page->frame);
  page->present = 0;
  page->frame = 0x0;
}

void init_paging()
{
  uint32_t end_page = 0x1000000; //On admet que la mémoire fait 16MB

  nframes = end_page / 0x1000;
  frames = (uint32_t*) kmalloc(INDEX_FROM_BIT(nframes));
  set(frames, 0, INDEX_FROM_BIT(nframes));

  //création du directory
  kdir = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
  set(kdir, 0, sizeof(page_directory_t));
  cdir = kdir;

  //magique, j'ai pas encore compris
  int i = 0;
  while (i < fmem_addr)
  {
    alloc_frame(get_page(i, 1, kdir), 0, 0);
    i += 0x1000;
  }

  switch_page_directory(kdir);
}

void switch_page_directory(page_directory_t *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(&dir->tables_phys));
   u32int cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000;
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t * get_page(uint32_t add, int make, page_directory_t *dir)
{
  add /= 0x1000;
  uint32_t t_id = add / 1024;

  if (dir->tables[t_id]) //si la table est bien assignée
  {
    return &dir->tables[table_id]->pages[add%1024];
  }
  else if (make)
  {
    uint32_t tmp;
    dir->tables[t_id] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
    set(dir->tables[t_id], 0, 0x1000);
    dir->tables_phys[t_id] = tmp | 0x7;
    return &dir->tables[t_id]->pages[add%1024];
  }
  else
  {
    return 0;
  }
}
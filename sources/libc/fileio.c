
#include "fileio.h"

#include "../drivers/disk.h"
#include "../kernel/filesystem.h"
#include "../kernel/shell.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#include "../libc/linked_list.h"

char PATH_SEP     = '/';
char *PATH_SELF   = ".";
char *PATH_PARENT = "..";

void free_obj0(file_t f) {
    free(f.path);
    free(f.name);
}
void free_obj1(file_t * f) {
    free_obj0(*f);
    free(f);
}
void free_obj2(directory_t d) {
    free(d.path);
    for (uint16_t i = 0; i != d.size; ++i) {
        free_obj0(d.contents[i]);
    }
    free(d.contents);
}
void free_obj3(directory_t* d) {
    free_obj2(*d);
    free(d);
}

file_t copy_file_t(file_t rep, file_t other) {
    rep.name = str_copy(other.name);
    rep.path = str_copy(other.path);
    rep.addr_start = other.addr_start;
    rep.addr_end = other.addr_end;
    rep.is_file = other.is_file;
    rep.owner = other.owner;
    rep.size = other.size;
    return rep;
}

directory_t dir_read(file_t * f) {
    // read the size directly in the directory
    uint16_t * contents = (uint16_t*)file_read(f->addr_start, 8);
    uint64_t bit_size = ((uint64_t *)contents)[0];
    free(contents);
    contents = (uint16_t*)(&file_read(f->addr_start, bit_size)[8]);
    directory_t d;
    d.size = contents[0] + 1;
    unsigned int i = str_length(f->path);
    unsigned int j = str_length(f->name);
    d.path = malloc(sizeof(char) * (i + 2 + j));
    memcpy(f->path, d.path, i);
    d.path[i] = PATH_SEP;
    memcpy(f->name, d.path + (i + 1), j);
    d.path[i + j + 1] = 0;
    d.addr = f->addr_start;
    d.contents = malloc(sizeof(file_t) * d.size);
    copy_file_t(d.contents[0], *f);
    d.contents[0].name = PATH_SELF;
    uint64_t pos = 1;
    for (uint16_t x = 1; x != d.size; ++x) {
        d.contents[x].is_file     = contents[pos] & 1;
        d.contents[x].permissions = (contents[pos] >> 1) & 0b111111;
        d.contents[x].owner       = (contents[pos] >> 7);
        d.contents[x].addr_start  = contents[++pos];
        d.contents[x].addr_end    = contents[++pos];
        d.contents[x].name = str_copy((char*)&contents[++pos]);
        uint32_t s = str_length(d.contents[x].name) + 1; //includes last 0
        pos += s / 2 + (s & 1); 
        d.contents[x].path = str_copy(d.path);
    }
    free(contents);
    return d;
}
uint64_t dir_write(directory_t d) {
    uint64_t bit_size = 8 + 1; // <taille:64> + <nb fichier:16>
    uint16_t x;
    for (x = 1; x != d.size; ++x) { //ignore first dir (self)
        bit_size += 6;
        bit_size += str_length(d.contents[x].name) + 1;
    }
    uint16_t* contents = malloc(bit_size * sizeof(char));
    ((uint64_t*)contents)[0] = bit_size;
    uint64_t pos = 8;
    contents[pos++] = d.size - 1;
    for (x = 1; x != d.size; ++x) {
        contents[pos++] = d.contents[x].is_file | (d.contents[x].permissions << 1) | (d.contents[x].owner << 7);
        contents[pos++] = d.contents[x].addr_start;
        contents[pos++] = d.contents[x].addr_end;
        uint16_t i = str_length(d.contents[x].name) + 1;
        memcpy(d.contents[x].name, (char *)(contents + pos), i);
        pos += (i / 2) + (i & 1);
    }
    file_write(d.addr, bit_size, (char*)contents);
    free(contents);
    return bit_size;
}
void dir_update(char * dirpath, file_t * file) {
    file_t *a = info(dirpath);
    directory_t d = dir_read(a);
    for (uint16_t x = 0; x != d.size; ++x) {
        if (str_equal(file->name, d.contents[x].name)) {
            copy_file_t(d.contents[x], *file);
            break;
        }
    }
    dir_write(d);
    free_obj2(d);
    free_obj1(a);
}
void dir_remove(directory_t d, char * name) {
    char cp = 0;
    --d.size;
    for (uint16_t x = 0; x != d.size; ++x) {
        if (str_equal(name, d.contents[x].name))
            cp = 1;
        if (cp)
            d.contents[x] = d.contents[x + 1];
    }
    dir_write(d);
}
void dir_add(directory_t dir, file_t file) {
    ++dir.size;
    file_t *old = dir.contents;
    dir.contents = malloc(dir.size * sizeof(file_t));
    for (uint16_t x = 0; x != dir.size-1; ++x) {
        copy_file_t(dir.contents[x], old[x]);
    }
    copy_file_t(dir.contents[dir.size - 1], file);
    dir_write(dir);
    free(old);
}


file_t* get_child(char * name, directory_t container) {
    for (uint16_t i = 0; i != container.size; ++i) {
        if (str_equal(name, container.contents[i].name))
            return &container.contents[i];
    }
    return 0;
}

file_t * info(char * path) {
    file_t *t = 0;
    if (path[0] == 0) {
        t = malloc(sizeof(file_t));
        copy_file_t(*t, CWD.contents[0]);
        return t;
    }
    directory_t s = CWD;
    struct linked_list *runner = ll_split(path, PATH_SEP);
    if (runner->hd[0] == 0) {
        s = ROOT;
        runner = runner->tl;
    }
    do {
        file_t *t = get_child(runner->hd, s);
        if (!t)
            return 0; //path does not exist
        if (runner->tl) {
            if (t->is_file) {
                return 0; // no next directory
            }
            s = dir_read(t);
            free_obj1(t);
            runner = runner->tl;
        }
        else {
            ll_free(runner);
            return t;
        }
    } while (1);
}

directory_t ls(char * path) {
    file_t *f = info(path);
    directory_t d = dir_read(f);
    free_obj1(f);
    return d;
}
void cd(char * path) {
    file_t *nwd = info(path);
    if (nwd->is_file) {
        free_obj1(nwd);
        return; // error
    }
    free_obj2(CWD);
    CWD = dir_read(nwd);
    free_obj1(nwd);
}
char *getcwd() {
    return CWD.path;
}

char exists(char * path) {
    file_t *f = info(path);
    if (f) {
        free_obj1(f);
        return 1;
    }
    return 0;
}

char is_file(char * path) { // 1 fichier, 0 dossier inexistant
    file_t *a = info(path);
    char t = a->is_file;
    free_obj1(a);
    return t;
}

char rm(char * path) {
    file_t * f = info(path);
    if (f) {
        if (!f->is_file) {
            directory_t d = dir_read(f);
            if (d.size > 2) { // . and ..
                free_obj1(f);
                return 3; //non empty dir
            }
        }
        file_delete(f->addr_start);
        file_t* parent = info(f->path);
        directory_t p = dir_read(parent);
        free_obj1(parent);
        dir_remove(p, f->name);
        free_obj1(f);
        free_obj2(p);
        return 0; // valid
    }
    return 1; // file not found
}
char cp(char * source, char * dest) {
    file_contents_t c = readfile(source);
    if (c.contents == 0)
        return 1;
    char t = write(dest, c.contents, c.size);
    free(c.contents);
    return t == 0 ? 0 : 2;
}
char mv(char * source, char * dest) {
    file_t *f = info(source);
    if (f) {
        uint8_t i = 0;
        int last_sep = -1;
        while (source[i]) {
            if (source[i] == PATH_SEP)
                last_sep = i;
            ++i;
        }
        source[last_sep] = 0;
        file_t *g = info(source);
        directory_t d = dir_read(g);
        dir_remove(d, f->name);
        free_obj2(d);
        free_obj1(g);
        source[last_sep] = PATH_SEP;
        i = 0;
        last_sep = -1;
        while (dest[i]) {
            if (dest[i] == PATH_SEP)
                last_sep = i;
            ++i;
        }
        dest[last_sep] = 0;
        g = info(dest);
        f->path = dest;
        f->name = dest + last_sep + 1;
        d = dir_read(g);
        dir_add(d, *f);
        free_obj1(g);
        free_obj1(f);
        free_obj2(d);
        dest[last_sep] = PATH_SEP;
        return 0;
    }
    return 1;
}

char mkdir(char * path) { // cree dernier dossier
    uint8_t i = 0;
    int last_sep = -1;
    while (path[i]) {
        if (path[i] == PATH_SEP)
            last_sep = i;
        ++i;
    }
    file_t new;
    new.name = last_sep == -1 ? path : path + last_sep;
    if (last_sep != -1) {
        path[last_sep] = 0; // end path on separator
    }
    file_t *s = info(path);
    if (s) {
        directory_t parent = dir_read(s);
        directory_t self;
        self.size = 2;
        self.path = malloc(sizeof(char) * i);
        memcpy(path, self.path, i);
        self.contents = malloc(2 * sizeof(file_t));
        copy_file_t(self.contents[1], *s); // copies parent into ..
        free(self.contents[1].name);
        self.contents[1].name = PATH_PARENT;
        dir_write(self);
        uint32_t addr = file_new(s->addr_start, 0, 0, 0);
        new.addr_start = addr >> 16;
        new.addr_end = (uint16_t)(addr & 0xffff);
        new.size = 1;
        new.permissions = s->permissions; // copies permission from parent dir
        new.owner = s->owner;
        new.is_file = 0;
        new.path = s->path;
        dir_add(parent, new); // adds as child of parent
        free(s);
        return 0;
    }
    return 1; // invalid path
}
char mkfile(char * path) {
    uint8_t i = 0;
    int last_sep = -1;
    while (path[i]) {
        if (path[i] == PATH_SEP)
            last_sep = i;
        ++i;
    }
    file_t new;
    new.name = last_sep == -1 ? path : path + last_sep;
    if (last_sep != -1) {
        path[last_sep] = 0; // end path on separator
    }
    file_t *s = info(path);
    if (s) {
        directory_t parent = dir_read(s);
        uint32_t addr = file_new(s->addr_start, 0, 0, 0);
        new.addr_start = addr >> 16;
        new.addr_end = (uint16_t)(addr & 0xffff);
        new.size = 0;
        new.permissions = s->permissions; // copies permission from parent dir
        new.owner = s->owner;
        new.is_file = 1;
        new.path = s->path;
        dir_add(parent, new);
        free_obj0(new);
        free_obj1(s);
        free_obj2(parent);
        return 0;
    }
    return 1; // invalid path
}

file_contents_t readfile(char * path) {
    file_contents_t rep;
    rep.contents = 0;
    rep.size = 0;
    file_t *file = info(path);
    if (file) {
        if (file->is_file) {
            rep.contents = file_read(file->addr_start, file->size);
            rep.size = file->size;
            free(file);
            return rep;
        }
    }
    return rep; // File not found
}
char append(char *path, char *contents, uint64_t size) {
    file_t *file = info(path);
    if (file) {
        if (file->is_file) {
            file->addr_end = file_append(file->addr_end, size, contents);
            dir_update(file->path, file);
            free_obj1(file);
            return 0;
        }
        return 3;
    }
    return 1; // file does not exist
}
char write(char *path, char *contents, uint64_t size) {
    file_t *file = info(path);
    if (file) {
        if (file->is_file) {
            file->addr_end = file_write(file->addr_end, size, contents);
            dir_update(file->path, file);
            free_obj1(file);
            return 0;
        }
        return 3;
    }
    return 1; // file does not exist
}
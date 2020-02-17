
#include "shell.h"

#include "../libc/mem.h"
#include "../libc/string.h"
#include "../libc/fileio.h"
#include "../libc/linked_list.h"
#include "../libc/iostream.h"

uint32_t int_from_char(char c)
{
	return c - '0';
}

uint64_t int_from_string(char *s)
{
	uint64_t res = 0;
	int i;
	for(i=0; s[i]; i++)
	{
		res = res*10 + int_from_char(s[i]);
	}
	return res;
}

void shell_echo(struct linked_list *l)
{
	ll_print(l, ' ');
	kprintf("\n"); //Pas sûr si ça devrait être là :shrug:
}

void shell_ls(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	directory_t dir = (p) ? ls(p->hd) : ls(empty_str);
	int i;
	for(i = 0; i < dir.size ; i++)
	{
		kprintf("%s\n", dir.contents[i].name);
	}
}

void shell_cd(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	(p) ? cd(p->hd) : cd(empty_str);
}

void shell_rm(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	(p) ? rm(p->hd) : rm(empty_str);
}

void shell_cp(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	if(p)
	{
		struct linked_list *p2 = ll_purge(p->tl);
		(p2)? cp(p->hd, p2->hd) : cp(p->hd, empty_str);
	}
	else kprintf("Not enough args for cp\n");
}

void shell_mv(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	if(p)
	{
		struct linked_list *p2 = ll_purge(p->tl);
		(p2)? mv(p->hd, p2->hd) : mv(p->hd, empty_str);
	}
	else kprintf("Not enough args for mv\n");
}

void shell_mkdir(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	if(p) {
		(exists(p->hd)) ? kprintf("%s already exists\n", p->hd) : mkdir(p->hd);
	}
	else kprintf("No name given to make directory\n");
}

void shell_mkfile(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);

	if(p) {
		(exists(p->hd)) ? kprintf("%s already exists\n", p->hd) : mkfile(p->hd);
	}
	else kprintf("No name given to make file\n");
}

void shell_readfile(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	if(p){
		char *cont = readfile(p->hd).contents;
		(cont[0])? kprintf("%s\n", cont) : kprintf("Error when reading file\n");
	}
	else kprintf("No file given to read\n");
}

void shell_append(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	if(p) {
		struct linked_list *p2 = ll_purge(p->tl);
		if(p2) {
			if(p2->tl) {
				switch ((append(p->hd, ll_stitch(p2->tl, ' '), int_from_string(p2->hd))))
				{
				case 1:
					return kprintf("File not found : %s\n", p->hd);
				case 2:
					return kprintf("Permission denied, cannot write %s\n", p->hd);
				case 3:
					return kprintf("Can't write to directory %s\n", p->hd);
				default:
					break;
				}
			}
			else kprintf("Not enough args. Usage: append <path> <nb_bytes> bytes\n");
		}
		else kprintf("Not enough args. Usage: append <path> <nb_bytes> bytes\n");
	}
	else kprintf("Not enough args. Usage: append <path> <nb_bytes> bytes\n");
}

void shell_write(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	if (p) {
		struct linked_list *p2 = ll_purge(p->tl);
		if (p2) {
			if(p2->tl) {
				switch ((write(p->hd, ll_stitch(p2->tl, ' '), int_from_string(p2->hd))))
				{
				case 1:
					return kprintf("File not found : %s\n", p->hd);
				case 2:
					return kprintf("Permission denied, cannot write %s\n", p->hd);
				case 3:
					return kprintf("Can't write to directory %s\n", p->hd);
				default:
					break;
				}
			}
			else kprintf("Not enough args. Usage: write <path> <nb_bytes> bytes\n");
		}
		else kprintf("Not enough args. Usage: write <path> <nb_bytes> bytes\n");
	}
	else kprintf("Not enough args. Usage: write <path> <nb_bytes> bytes\n");
}


void help(struct linked_list *l)
{
	struct linked_list *p = ll_purge(l);
	if(p){
		if (str_equal(p->hd, "echo")) kprintf("\"echo a\" prints a\n");
		else if(str_equal(p->hd, "ls")) kprintf("\"ls dir\" prints the contents of the directory dir\n");
		else if(str_equal(p->hd, "cd")) kprintf("\"cd dir\" changes the current working directory to dir\n");
		else if(str_equal(p->hd, "cwd")) kprintf("\"cwd\" prints the current working directory\n");
		else if(str_equal(p->hd, "rm")) kprintf("\"rm x\" removes file or directory x\n");
		else if(str_equal(p->hd, "cp")) kprintf("\"cp x y\" copies x to y\n");
		else if(str_equal(p->hd, "mv")) kprintf("\"mv x y\" moves x to y\n");
		else if(str_equal(p->hd, "mkdir")) kprintf("\"mkdir dir\" creates directory dir\n");
		else if(str_equal(p->hd, "mkfile")) kprintf("\"mkfile f\" creates file f\n");
		else if(str_equal(p->hd, "readfile")) kprintf("\"readfile f\" reads file f and prints its contents\n");
		else if(str_equal(p->hd, "append")) kprintf("\"append f n c\" appends n bytes of c to file f\n");
		else if(str_equal(p->hd, "write")) kprintf("\"write f n c\" writes n bytes of c on file f\n");
		else if(str_equal(p->hd, "help")) kprintf("The help\n");
		else kprintf("%s is not a recognised command, try help for a list of supported commands\n", p->hd);
	}
	else
	{
		kprintf("The supported commands are:\n");
		kprintf("echo\nls\ncd\ncwd\nrm\ncp\nmv\nmkdir\nmkfile\nreadfile\nappend\nwrite\nhelp\n");
		kprintf("type \"help command\" to get more details on command\n");
	}
}

void interp(char* line)
{
	struct linked_list *ll = ll_split(line, ' ');
	if (ll == 0) return ll_free(ll);
	if (str_equal(ll->hd, "echo")) shell_echo(ll->tl);
	else if (str_equal(ll->hd, "ls")) shell_ls(ll->tl);
	else if(str_equal(ll->hd, "cd")) shell_cd(ll->tl);
	else if(str_equal(ll->hd, "cwd")) kprintf("%s\n", getcwd());
	else if(str_equal(ll->hd, "rm")) shell_rm(ll->tl);
	else if(str_equal(ll->hd, "cp")) shell_cp(ll->tl);
	else if(str_equal(ll->hd, "mv")) shell_mv(ll->tl);
	else if(str_equal(ll->hd, "mkdir")) shell_mkdir(ll->tl);
	else if(str_equal(ll->hd, "mkfile")) shell_mkfile(ll->tl);
	else if(str_equal(ll->hd, "readfile")) shell_readfile(ll->tl);
	else if(str_equal(ll->hd, "append")) shell_append(ll->tl);
	else if(str_equal(ll->hd, "write")) shell_write(ll->tl);
	else if(str_equal(ll->hd, "help")) help(ll->tl);
	else kprintf("%s is not a recognised command, try help for a list of supported commands\n", ll->hd);
	ll_free(ll);
}

void shell()
{
	kprintf("\033[9f]%s\033[_]$ ", getcwd());
	interp(readline());
}
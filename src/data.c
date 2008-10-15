/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "main.h"
#include "code.h"
#include "data.h"
#include "undo.h"
#include "flags.h"
#include "arch/csr/dis.h"
#include "arch/arm/disarm.h"
#include "arch/ppc/ppc_disasm.h"
#include "arch/m68k/m68k_disasm.h"
#include "arch/x86/udis86/types.h"
#include "arch/x86/udis86/extern.h"
#include "list.h"

struct reflines_t *reflines = NULL;

static struct list_head data;
static struct list_head comments;
static struct list_head xrefs;

int data_set_len(u64 off, u64 len)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (off>= d->from && off<= d->to) {
			d->to = d->from+len;
			d->size = d->to-d->from+1;
			return 0;
		}
	}
	return -1;
}

void data_info()
{
	struct list_head *pos;
	int n_functions = 0;
	int n_xrefs = 0;
	int n_dxrefs = 0;

	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->type == DATA_FUN)
			n_functions++;
	}

	list_for_each(pos, &xrefs) {
		struct xrefs_t *x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->type == 0)
			n_dxrefs++;
		else n_xrefs++;
	}
	
	cons_printf("functions: %d\n", n_functions);
	cons_printf("data_xrefs: %d\n", n_dxrefs);
	cons_printf("code_xrefs: %d\n", n_xrefs);
}

int data_set(u64 off, int type)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (off>= d->from && off<= d->to) {
			d->type = type;
			return 0;
		}
	}
	return -1;
}

struct data_t *data_add_arg(u64 off, int type, const char *arg)
{
	struct data_t *d = data_add(off, type);
	strncpy(d->arg , arg, 128);
	return d;
}

struct data_t *data_add(u64 off, int type)
{
	u64 tmp;
	struct data_t *d;
	struct list_head *pos;

	__reloop:
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d && (off>= d->from && off< d->to) ) {//&& d->type != type ) {
			list_del((&d->list)); //->list));
			goto __reloop;
		}
	}

	if (type == DATA_CODE)
		return d;

	d = (struct data_t *)malloc(sizeof(struct data_t));
	d->arg[0]='\0';
	d->from = off;
	d->to = d->from + config.block_size;  // 1 byte if no cursor // on strings should autodetect

	if (config.cursor_mode) {
		d->to = d->from + 1;
		d->from+=config.cursor;
		if (config.ocursor!=-1)
			d->to = config.seek+config.ocursor;
		if (d->to < d->from) {
			tmp = d->to;
			d->to  = d->from;
			d->from = tmp;
		}
	}
	d->type = type;
	if (d->to > d->from) {
	//	d->to++;
		d->size = d->to - d->from+1;
	} else d->size = d->from - d->to+1;
	if (d->size<1)
		d->size = 1;

	list_add(&(d->list), &data);

	return d;
}

u64 data_seek_to(u64 offset, int type, int idx)
{
	u64 ret = 0ULL;
	struct list_head *pos;
	int i = 0;
	idx--;

	list_for_each(pos, &xrefs) {
		struct xrefs_t *d = (struct xrefs_t *)list_entry(pos, struct xrefs_t , list);
		if (d->type == type || type == -1) {
			if (d->addr == offset && idx == i) {
				ret = d->from;
				break;
			}
			i++;
		}
	}
	return ret;
}

struct data_t *data_get(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset >= d->from && offset < d->to)
			return d;
	}
	return NULL;
}

struct data_t *data_get_range(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset >= d->from && offset < d->to)
			return d;
	}
	return NULL;
}

/* TODO: OPTIMIZE: perform cache here */
struct data_t *data_get_between(u64 from, u64 to)
{
	int hex = 0;
	int str = 0;
	int fun = 0;
	int code = 0;
	struct list_head *pos;
	struct data_t *d = NULL;

	list_for_each(pos, &data) {
		d = (struct data_t *)list_entry(pos, struct data_t, list);
		//if (from >= d->from && to <= d->to) {
		if (d->from >= from && d->to < to) {
			switch(d->type) {
			case DATA_HEX: hex++; break;
			case DATA_STR: str++; break;
			case DATA_CODE: code++; break;
			case DATA_FUN: fun++; break;
			}
		}
	}

	if (d == NULL)
		return NULL;

	if (hex>=str && hex>=code && hex>=fun) {
		d->type = DATA_HEX;
		d->times = hex;
	} else
	if (str>=hex && str>=code && str>=fun) {
		d->type = DATA_STR;
		d->times = str;
	} else
	if (fun>=hex && fun>=str && fun>=code) {
		d->type = DATA_FUN;
		d->times = fun;
	} else
	if (code>=hex && code>=str && code>=fun) {
		d->type = DATA_CODE;
		d->times = code;
	}
//printf("0x%llx-0x%llx: %d %d %d = %d\n", from, to, hex, str, code, d->type);

	return d;
}

int data_type_range(u64 offset)
{
	struct data_t *d = data_get_range(offset);
	if (d != NULL)
		return d->type;
	return -1;
}

int data_type(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from)
			return d->type;
	}
	return -1;
}

int data_end(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from+d->size) // XXX: must be d->to..but is buggy ?
			return d->type;
	}
	return -1;
}

int data_size(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from)
			return d->size;
	}
	return 0;
}

int data_list()
{
	char label[1024];
	int verbose=config.verbose;
	struct data_t *d;
	struct list_head *pos;
	list_for_each(pos, &data) {
		d = (struct data_t *)list_entry(pos, struct data_t, list);
		label[0]='\0';
		string_flag_offset(label, d->from);
		switch(d->type) {
		case DATA_FOLD_O: cons_strcat("Cu "); break;
		case DATA_FOLD_C: cons_strcat("Cf "); break;
		case DATA_FUN:    cons_strcat("CF "); break;
		case DATA_HEX:    cons_strcat("Cd "); break;
		case DATA_STR:    cons_strcat("Cs "); break;
		default:          cons_strcat("Cc "); break; }
		cons_printf("%lld @ 0x%08llx ; %s", d->to-d->from, d->from, label);
#if 0
		if (verbose)
		if (d->type == DATA_STR) {
			cons_printf("  (");
			sprintf(label, "pz@0x%08llx", d->from);
			radare_cmd(label, 0);
		}else
#endif
		cons_newline();
	}
	return 0;
}
/* -- metadata -- */
int data_xrefs_print(u64 addr, int type)
{
	char str[1024];
	int n = 0;
	struct xrefs_t *x;
	struct list_head *pos;
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr) {
			str[0]='\0';
			string_flag_offset(str, x->from);
			switch(type) {
			case 0: if (x->type == type) { cons_printf("; 0x%08llx CODE xref 0x%08llx (%s)\n", addr, x->from, str); n++; } break;
			case 1: if (x->type == type) { cons_printf("; 0x%08llx DATA xref 0x%08llx (%s)\n", addr, x->from), str; n++; } break;
			default: { cons_printf("; 0x%08llx %s xref from 0x%08llx (%s)\n", addr, (x->type==1)?"DATA":(x->type==0)?"CODE":"UNKNOWN",x->from, str); n++; };
			}
		}
	}

	return n;
}

int data_xrefs_add(u64 addr, u64 from, int type)
{
	struct xrefs_t *x;
	struct list_head *pos;

	/* avoid dup */
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr && x->from == from)
			return 0;
	}

	x = (struct xrefs_t *)malloc(sizeof(struct xrefs_t));

	x->addr = addr;
	x->from = from;
	x->type = type;

	list_add(&(x->list), &xrefs);

	return 1;
}

int data_xrefs_at(u64 addr)
{
	int ctr = 0;
	struct xrefs_t *x;
	struct list_head *pos;

	/* avoid dup */
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr)
			ctr++;
	}
	return ctr;

}

void data_xrefs_del(u64 addr, u64 from, int data /* data or code */)
{
	struct xrefs_t *x;
	struct list_head *pos;
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr && x->from == from) {
			list_del(&(x->list));
			break;
		}
	}
}

void data_comment_del(u64 offset, const char *str)
{
	struct comment_t *cmt;
	struct list_head *pos;
	//u64 off = get_math(str);

	list_for_each(pos, &comments) {
		cmt = list_entry(pos, struct comment_t, list);
#if 0
		if (!pos)
			return;
#endif

#if 0
		if (off) {
			if ((off == cmt->offset)) {
				free(cmt->comment);
				list_del(&(pos));
				free(cmt);
				if (str[0]=='*')
					data_comment_del(offset, str);
				pos = comments.next; // list_init
				return;
			}
		} else {
#endif
		    if (offset == cmt->offset) {
			    if (str[0]=='*') {
				    free(cmt->comment);
				    list_del(&(pos));
				    free(cmt);
				    pos = comments.next; // list_init
				    //data_comment_del(offset, str);
			    } else
			    if (!strcmp(cmt->comment, str)) {
				    list_del(&(pos));
				    return;
			    }
		    }
#if 0
		}
#endif
	}
}

void data_comment_add(u64 offset, const char *str)
{
	struct comment_t *cmt;
	char *ptr;

	/* no null comments */
	if (strnull(str))
		return;

	/* avoid dupped comments */
	data_comment_del(offset, str);

	cmt = (struct comment_t *) malloc(sizeof(struct comment_t));
	cmt->offset = offset;
	ptr = strdup(str);
	if (ptr[strlen(ptr)-1]=='\n')
		ptr[strlen(ptr)-1]='\0';
	cmt->comment = ptr;
	list_add_tail(&(cmt->list), &(comments));
}

void data_comment_list()
{
	struct list_head *pos;
	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		cons_printf("CC %s @ 0x%llx\n", cmt->comment, cmt->offset);
	}
}

void data_xrefs_here(u64 addr)
{
	int count = 0;
	char label[1024];
	struct xrefs_t *x;
	struct list_head *pos;

	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (addr = x->addr) {
			label[0]='\0';
			string_flag_offset(label, x->from);
			cons_printf("%d %s xref 0x%08llx @ 0x%08llx ; %s\n",
				count+1, x->type?"data":"code", x->from, x->addr, label);
			count++;
		}
	}
	if (count == 0) {
		eprintf("No xrefs found\n");
	}
}

void data_xrefs_list()
{
	char label[1024];
	struct xrefs_t *x;
	struct list_head *pos;

	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		label[0]='\0';
		string_flag_offset(label, x->from);
		cons_printf("C%c 0x%08llx @ 0x%08llx ; %s\n", x->type?'d':'x', x->from, x->addr, label);
	}
}

char *data_comment_get(u64 offset, int lines)
{
	struct list_head *pos;
	char *str = NULL;
	int cmtmargin = (int)config_get_i("asm.cmtmargin");
	int cmtlines = config_get_i("asm.cmtlines");
	char null[128];

	memset(null,' ',126);
	null[126]='\0';
	if (cmtmargin<0) cmtmargin=0; else
		// TODO: use screen width here
		if (cmtmargin>80) cmtmargin=80;
	null[cmtmargin] = '\0';
	if (cmtlines<0)
		cmtlines=0;

	if (cmtlines) {
		int i = 0;
		list_for_each(pos, &comments) {
			struct comment_t *cmt = list_entry(pos, struct comment_t, list);
			if (cmt->offset == offset)
				i++;
		}
		if (i>cmtlines)
			cmtlines = i-cmtlines;
	}

	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		if (cmt->offset == offset) {
			if (cmtlines) {
				cmtlines--;
				continue; // skip comment lines
			}
			if (str == NULL) {
				str = malloc(1024);
				str[0]='\0';
			} else {
				str = realloc(str, cmtmargin+strlen(str)+strlen(cmt->comment)+128);
			}
			strcat(str, null);
			strcat(str, "; ");
			strcat(str, cmt->comment);
			strcat(str, "\n");
			if (--lines == 0)
				break;
		}
	}
	return str;
}

void data_comment_init(int new)
{
	INIT_LIST_HEAD(&(xrefs));
	INIT_LIST_HEAD(&(comments));
	INIT_LIST_HEAD(&(data));
}

void data_reflines_init()
{
	int show_lines    = (int) config_get_i("asm.lines");
	reflines = NULL;
	if (show_lines)
		reflines = code_lines_init();
}

int data_printd(int delta)
{
	int show_lines = (int)config_get("asm.lines");
	int show_flagsline = (int)config_get("asm.flagsline");
	u64 offset = (u64)config.seek + (u64)delta;// - config.baddr;
	int lines = 0;
	const char *ptr;
	int i = 0;

	D {} else return 0;
	if (config_get("asm.flags") && show_flagsline) {
		ptr = flag_name_by_offset( offset );
		if (ptr == NULL && config.baddr)
			ptr = flag_name_by_offset( config.seek + delta);
		if (ptr && ptr[0]) {
			if (show_lines&&reflines)
				code_lines_print(reflines, offset, 1);
			C
				cons_printf(C_RESET C_BWHITE""OFF_FMT" %s:"C_RESET"\n", offset, ptr);
			else
				cons_printf(OFF_FMTs" %s:\n", offset, ptr);
			lines++;
		}
	}

	ptr = data_comment_get(offset, config.height-cons_lines);
	if (ptr && ptr[0]) {
		int i;
		for(i=0;ptr[i];i++)
			if (ptr[i]=='\n') lines++;
		C 	cons_printf(C_MAGENTA"%s"C_RESET, ptr);
		else 	cons_printf("%s", ptr);
		free(ptr);
	}

	lines += data_xrefs_print(offset, -1);
	return lines;
}
/* radare - LGPL - Copyright 2008-2009 pancake<nopcode.org> */

#include <r_range.h>

// TODO: redesign this api

#if 0
// TODO: add tags to ranges
#endif

//void (*ranges_new_callback)(struct range_t *r) = NULL;

int r_range_init(struct r_range_t *r)
{
	r->count = 0;
	r->changed = 0;
	INIT_LIST_HEAD(&r->ranges);
	return 0;
}

struct r_range_t *r_range_new()
{
	struct r_range_t *r = MALLOC_STRUCT(struct r_range_t);
	if (r == NULL)
		return NULL;
	r_range_init(r);
	return r;
}

struct r_range_t *r_range_free(struct r_range_t *r)
{
	struct list_head *pos;
	list_for_each(pos, &r->ranges) {
		struct r_range_item_t *h = list_entry(pos, struct r_range_item_t, list);
		free(h);
	}
	free(r);
	return NULL;
}

// TODO: optimize by just returning the pointers to the internal foo?
int r_range_get_data(struct r_range_t *rgs, u64 addr, u8 *buf, int len)
{
	struct r_range_item_t *r = r_range_item_get(rgs, addr);
	if (r == NULL)
		return 0;
	if (r->datalen < len)
		len = r->datalen;
	memcpy(buf, r->data, len);
	return len;
}

int r_range_set_data(struct r_range_t *rgs, u64 addr, const u8 *buf, int len)
{
	struct r_range_item_t *r = r_range_item_get(rgs, addr);
	if (r == NULL)
		return 0;
	r->data = (u8*)malloc(len);
	r->datalen = len;
	memcpy(r->data, buf, len);
	return 1;
}

struct r_range_item_t *r_range_item_get(struct r_range_t *rgs, u64 addr)
{
	struct r_range_item_t *r;
	struct list_head *pos;
	list_for_each(pos, &rgs->ranges) {
		r = list_entry(pos, struct r_range_item_t, list);
		if (addr >= r->from && addr < r->to)
			return r;
	}
	return NULL;
}

/* returns the sum of all the ranges contained */
// XXX: can be catched while adding/removing elements
u64 r_range_size(struct r_range_t *rgs)
{
	struct list_head *pos;
	struct r_range_item_t *r;
	u64 sum = 0;

	list_for_each(pos, &rgs->ranges) {
		r = list_entry(pos, struct r_range_item_t, list);
		sum += r->to - r->from;
	}

	return sum;
}

struct r_range_t *r_range_new_from_string(const char *string)
{
	struct r_range_t *rgs = r_range_new();
	r_range_add_from_string(rgs, string);
	return rgs;
}

int r_range_add_from_string(struct r_range_t *rgs, const char *string)
{
	u64 addr, addr2;
	int i, len = strlen(string)+1;
	char *str = alloca(len);
	char *p = str;
	char *p2 = NULL;
	struct r_range_item_t *r;

	memcpy(str, string, len);

	for(i=0;i<len;i++) {
		switch(str[i]) {
		case '-':
			str[i]='\0';
			p2 = p;
			p = str+i+1;
			break;
		case ',':
			str[i]='\0';
			if (p2) {
				addr = r_num_get(NULL, p);
				addr2 = r_num_get(NULL, p2);
				r = r_range_add(rgs, addr, addr2, 1);
				p2 = NULL;
			} else {
				addr = r_num_get(NULL, p);
				r = r_range_add(rgs, addr, addr+1, 1);
			}
			p = str+i+1;
			str[i]=',';
			break;
		}
	}
	if (p2) {
		addr = r_num_get(NULL, p);
		addr2 = r_num_get(NULL, p2);
		r = r_range_add(rgs, addr, addr2, 1);
	} else 
	if (p) {
		addr = r_num_get(NULL, p);
		r = r_range_add(rgs, addr, addr+1, 1);
	}

	return rgs->changed;
}

#if 0
    update to      new one     update from   update from/to  ignore

   |______|        |___|           |_____|      |____|      |_______|  range_t
+     |______|   +      |__|   + |___|      + |_________|  +  |__|     from/to
  ------------   -----------   -----------  -------------  -----------
=  |_________|   = |___||__|   = |_______|  = |_________|   |_______|  result
#endif

struct r_range_item_t *r_range_add(struct r_range_t *rgs, u64 from, u64 to, int rw)
{
	struct list_head *pos;
	struct r_range_item_t *r;
	struct r_range_item_t *ret = NULL;
	int add = 1;

	r_num_minmax_swap(&from, &to);

	list_for_each(pos, &rgs->ranges) {
		r = list_entry(pos, struct r_range_item_t, list);
		if (r->from == from && r->to==to) {
			add = 0;
		} else
		if (r->from<=from && r->from <= to && r->to>=from && r->to <= to) {
			r->to = to;
			ret = r;
			add = 0;
		} else
		if (r->from>=from && r->from<=to && r->to>=from && r->to >= to) {
			r->from = from;
			ret = r;
			add = 0;
		} else
		if (r->from<=from && r->from<=to && r->to>=from && r->to >= to) {
			/* ignore */
			add = 0;
		} else
		if (r->from>=from && r->from<=to && r->to>=from && r->to <= to) {
			r->from = from;
			r->to = to;
			ret = r;
			add = 0;
		}
	}

	if (rw && add) {
		ret = MALLOC_STRUCT(struct r_range_item_t);
		ret->from = from;
		ret->to = to;
		ret->datalen = 0;
		ret->data = NULL;
		list_add_tail(&(ret->list), &rgs->ranges);
		rgs->changed = 1;
	}

	return ret;
}

#if 0
    update to      ignore      update from      delete        split

   |______|        |___|           |_____|      |____|       |________|  range_t
-     |______|   -      |__|   - |___|      - |_________|  -    |__|     from/to
  ------------   -----------   -----------  -------------  ------------
=  |__|          =             =     |___|  =                |__|  |__|   result
#endif

int r_range_sub(struct r_range_t *rgs, u64 from, u64 to)
{
	struct r_range_item_t *r;
	struct list_head *pos;

	r_num_minmax_swap(&from, &to);

	__reloop:
	list_for_each(pos, &rgs->ranges) {
		r = list_entry(pos, struct r_range_item_t, list);
		/* update to */
		if (r->from<from && r->from < to && r->to>from && r->to < to) {
			r->to = from;
		} else
		/* update from */
		if (r->from>from && r->from<to && r->to>from && r->to>to) {
			r->from = to;
		}
		/* delete */
		if (r->from>from && r->from<to && r->to>from && r->to < to) {
			/* delete */
			list_del(&(r->list));
			rgs->changed = 1;
			goto __reloop;
		}
		/* split */
		if (r->from<from && r->from<to && r->to>from && r->to > to) {
			r->to = from;
			r_range_add(rgs, to, r->to, 1);
			//ranges_add(rang, to, r->to, 1);
			goto __reloop;
		}
	}
	return 0;
}

/* TODO: should remove some of them right? */
//int r_range_merge(struct r_range_t *r)
int r_range_merge(struct r_range_t *rgs, struct r_range_t *r)
{
	struct list_head *pos;

	list_for_each(pos, &r->ranges) {
		struct r_range_item_t *r = list_entry(pos, struct r_range_item_t, list);
		r_range_add(rgs, r->from, r->to, 0);
	}
	return 0;
}

//int ranges_is_used(u64 addr)
int r_range_contains(struct r_range_t *rgs, u64 addr)
{
	struct list_head *pos;
	list_for_each(pos, &rgs->ranges) {
		struct r_range_item_t *r = list_entry(pos, struct r_range_item_t, list);
		if (addr >= r->from && addr <= r->to)
			return 1;
	}
	return 0;
}

int r_range_sort(struct r_range_t *rgs)
{
	struct list_head *pos, *pos2;
	struct list_head *n, *n2;

	if (rgs->changed==0)
		return 0;

	rgs->changed=0;
	list_for_each_safe(pos, n, &rgs->ranges) {
		struct r_range_item_t *r = list_entry(pos, struct r_range_item_t, list);
		list_for_each_safe(pos2, n2, &rgs->ranges) {
			struct r_range_item_t *r2 = list_entry(pos2, struct r_range_item_t, list);
			if ((r != r2) && (r->from > r2->from)) {
				list_move(pos, pos2);
				rgs->changed = 1;
			}
		}
	}
	return rgs->changed;
}

int r_range_percent(struct r_range_t *rgs)
{
	struct list_head *pos;
	int w, i;
	u64 seek, step;
	u64 dif, from = -1, to = -1;

	list_for_each(pos, &rgs->ranges) {
		struct r_range_item_t *r = list_entry(pos, struct r_range_item_t, list);
		if (from == -1) {
			/* init */
			from = r->from;
			to = r->to;
		} else {
			if (from>r->from)
				from = r->from;
			if (to<r->to)
				to = r->to;
		}
	}
	w = 65 ; // columns
	if (from == -1) {
		step = from = to = 0;
	} else {
		dif = to-from;
		if (dif<w) step = 1; // XXX
		else step = dif/w;
		seek = 0;
	}
	seek = 0;
	// XXX do not use printf here!
	printf("0x%08llx [", from);
	for(i=0;i<w;i++) {
		if (r_range_contains(rgs, seek))
			printf("#");
		else printf(".");
		seek += step;
	}
	printf("] 0x%08llx\n", to);
	return 0;
}

// TODO: total can be cached in rgs!!
int r_range_list(struct r_range_t *rgs, int rad)
{
	u64 total = 0;
	struct list_head *pos;
	r_range_sort(rgs);
	list_for_each(pos, &rgs->ranges) {
		struct r_range_item_t *r = list_entry(pos, struct r_range_item_t, list);
		if (rad) printf("ar+ 0x%08llx 0x%08llx\n", r->from, r->to);
		else printf("0x%08llx 0x%08llx ; %lld\n", r->from, r->to, r->to-r->from);
		total += (r->to-r->from);
	}
	eprintf("Total bytes: %lld\n", total);
	return 0;
}

int r_range_get_n(struct r_range_t *rgs, int n, u64 *from, u64 *to)
{
	int count = 0;
	struct list_head *pos;
	r_range_sort(rgs);
	list_for_each(pos, &rgs->ranges) {
		struct r_range_item_t *r = list_entry(pos, struct r_range_item_t, list);
		if (count == n) {
			*from = r->from;
			*to = r->to;
			return 1;
		}
		count++;
	}
	return 0;
}

#if 0
     .....|______________________|...
      |_____|  |____|  |_______|
    ---------------------------------
            |__|    |__|       |_|      
#endif
struct r_range_t *r_range_inverse(struct r_range_t *rgs, u64 from, u64 to, int flags)
{
	u64 total = 0;
	//u64 min = to;
	//u64 max = from;
	struct list_head *pos;
	struct r_range_item_t *r = NULL;
	struct r_range_t *newrgs = r_range_new();

	r_range_sort(rgs);

	list_for_each(pos, &rgs->ranges) {
		r = list_entry(pos, struct r_range_item_t, list);
		if (r->from > from && r->from < to) {
			r_range_add(newrgs, from, r->from, 1);
			//printf("0x%08llx .. 0x%08llx\n", from, r->from);
			total += (r->from - from);
			from = r->to;
		}
	}
	if (from < to) {
		//printf("0x%08llx .. 0x%08llx\n", from, to);
		r_range_add(newrgs, from, to, 1);
		total += (to-from);
	}
//	printf("Total bytes: %lld\n", total);

	return newrgs;
}

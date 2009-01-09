/* radare - LGPL - Copyright 2006-2009 pancake<nopcode.org> */

#include "r_config.h"
#include "r_util.h" // strhash, strclean, ...

struct r_config_node_t* r_config_node_new(const char *name, const char *value)
{
	struct r_config_node_t *node = 
		(struct r_config_node_t *)
			malloc(sizeof(struct r_config_node_t));
	INIT_LIST_HEAD(&(node->list));
	node->name = strdup(name);
	node->hash = strhash(name);
	node->value = value?strdup(value):strdup("");
	node->flags = CN_RW | CN_STR;
	node->i_value = 0;
	node->callback = NULL;
	return node;
}

void r_config_list(struct r_config_t *cfg, const char *str)
{
	struct list_head *i;
	int len = 0;

	if (!strnull(str)) {
		str = strclean(str);
		len = strlen(str);
	}

	list_for_each(i, &(cfg->nodes)) {
		struct r_config_node_t *bt = list_entry(i, struct r_config_node_t, list);
		if (str) {
			if (strncmp(str, bt->name,len) == 0)
				cfg->printf("%s = %s\n", bt->name, bt->value);
		} else cfg->printf("%s = %s\n", bt->name, bt->value);
	}
}

struct r_config_node_t *r_config_node_get(struct r_config_t *cfg, const char *name)
{
	struct list_head *i;
	int hash;
	if (strnull(name))
		return NULL;
	hash = strhash(name);
	list_for_each_prev(i, &(cfg->nodes)) {
		struct r_config_node_t *bt = list_entry(i, struct r_config_node_t, list);
		if (bt->hash == hash)
			return bt;
	}
	return NULL;
}

const char *r_config_get(struct r_config_t *cfg, const char *name)
{
	struct r_config_node_t *node =
		r_config_node_get(cfg, name);
	if (node) {
		cfg->last_notfound = 0;
		if (node->flags & CN_BOOL)
			return (const char *)
				(((!strcmp("true", node->value))
				  || (!strcmp("1", node->value)))?(const char *)1:NULL);
		return node->value;
	}
	cfg->last_notfound = 1;
	return NULL;
}

u64 r_config_get_i(struct r_config_t *cfg, const char *name)
{
	struct r_config_node_t *node =
		r_config_node_get(cfg, name);
	if (node) {
		if (node->i_value != 0)
			return node->i_value;
		return (u64)get_math(node->value);
	}
	return (u64)0LL;
}

struct r_config_node_t *r_config_set(struct r_config_t *cfg, const char *name, const char *value)
{
	struct r_config_node_t *node;

	if (name[0] == '\0')
		return NULL;

	node = r_config_node_get(cfg, name);

	// TODO: store old value somewhere..
	if (node) {
		if (node->flags & CN_RO) {
			eprintf("(read only)\n");
			return node;
		}
		free(node->value);
		if (node->flags & CN_BOOL) {
			int b = (!strcmp(value,"true")||!strcmp(value,"1"));
			node->i_value = (u64)(b==0)?0:1;
			node->value = strdup(b?"true":"false");
		} else {
			if (value == NULL) {
				node->value = strdup("");
				node->i_value = 0;
			} else {
				node->value = strdup(value);
				if (strchr(value, '/'))
					node->i_value = get_offset(value);
				else  node->i_value = get_math(value);
				node->flags |= CN_INT;
			}
		}
	} else {
		if (cfg->lock) {
			eprintf("(config-locked: '%s' no new keys can be created)\n", name);
		} else {
			node = r_config_node_new(name, value);
			if (value && (!strcmp(value,"true")||!strcmp(value,"false"))) {
				node->flags|=CN_BOOL;
				node->i_value = (!strcmp(value,"true"))?1:0;
			}
			list_add_tail(&(node->list), &(cfg->nodes));
		}
	}

	if (node && node->callback)
		node->callback(node);

	return node;
}

int r_config_rm(struct r_config_t *cfg, const char *name)
{
	struct r_config_node_t *node =
		r_config_node_get(cfg, name);
	if (node) {
		list_del(&(node->list));
		return 1;
	}
	return 0;
}

struct r_config_node_t *r_config_set_i(struct r_config_t *cfg, const char *name, const u64 i)
{
	char buf[128];
	struct r_config_node_t *node =
		r_config_node_get(cfg, name);

	if (node) {
		if (node->flags & CN_RO)
			return NULL;
		free(node->value);
		if (node->flags & CN_BOOL) {
			node->value = strdup(i?"true":"false");
		} else {
			sprintf(buf, "%lld", i); //0x%08lx", i);
			node->value = strdup(buf);
		}
		node->flags = CN_RW | CN_INT;
		node->i_value = i;
	} else {
		if (cfg->lock) {
			eprintf("(locked: no new keys can be created)");
		} else {
			sprintf(buf, "%d", (unsigned int)i);//OFF_FMTd, (u64) i);
			node = r_config_node_new(name, buf);
			node->flags = CN_RW | CN_OFFT;
			node->i_value = i;
			list_add_tail(&(node->list), &(cfg->nodes));
		}
	}

	if (node && node->callback)
		node->callback(node);

	return node;
}

int r_config_eval(struct r_config_t *cfg, const char *str)
{
	char *ptr,*a,*b;
	char *name;
	int len;

	if (str == NULL)
		return 0;

	len = strlen(str)+1;
	name = alloca(len);
	memcpy(name, str, len);
	str = strclean(name);

	if (str == NULL)
		return 0;

	if (str[0]=='\0'||!strcmp(str, "help")) {
		r_config_list(cfg, NULL);
		return 0;
	}

	if (str[0]=='-') {
		r_config_rm(cfg, str+1);
		return 0;
	}

	ptr = strchr(str, '=');
	if (ptr) {
		/* set */
		ptr[0]='\0';
		a = strclean(name);
		b = strclean(ptr+1);
		r_config_set(cfg, a, b);
	} else {
		char *foo = strclean(name);
		if (foo[strlen(foo)-1]=='.') {
			/* list */
			r_config_list(cfg, name);
			return 0;
		} else {
			/* get */
			const char * str = r_config_get(cfg, foo);
			if (cfg->last_notfound)
				r_config_list(cfg, name);
			else cfg->printf("%s\n", (((int)str)==1)?"true":(str==0)?"false":str);
		}
	}
	return 1;
}

void r_config_lock(struct r_config_t *cfg, int l)
{
	cfg->lock = l;
}

int r_config_init(struct r_config_t *cfg)
{
	cfg->n_nodes = 0;
	cfg->lock = 0;
	cfg->printf = &printf;
	INIT_LIST_HEAD(&(cfg->nodes));
	return 0;
}

struct r_config_t *r_config_new()
{
	struct r_config_t *cfg = (struct r_config_t *)
		malloc(sizeof(struct r_config_t));
	r_config_init(cfg);
	return cfg;
}

int r_config_free(struct r_config_t *cfg)
{
	// TODO: Free node list
	free(cfg);
	return 0;
}

void r_config_visual_hit_i(struct r_config_t *cfg, const char *name, int delta)
{
	struct r_config_node_t *node =
		r_config_node_get(cfg, name);
	if (node && (node->flags & CN_INT || node->flags & CN_OFFT))
		r_config_set_i(cfg, name, r_config_get_i(cfg, name)+delta);
}

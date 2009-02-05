#ifndef _INCLUDE_R_CORE_H_
#define _INCLUDE_R_CORE_H_

#include "r_types.h"
#include "r_io.h"
#include "r_lib.h"
#include "r_lang.h"
#include "r_cmd.h"
#include "r_cons.h"
#include "r_line.h"
#include "r_print.h"
#include "r_macro.h"
#include "r_debug.h"
#include "r_flags.h"
#include "r_config.h"

#define R_CORE_BLOCKSIZE 512
#define R_CORE_BLOCKSIZE_MAX 0x40000 /* 4 MB */

struct r_core_file_t {
	char *uri;
	char *filename;
	u64 seek;
	u64 size;
	int rwx;
	int fd;
	struct list_head list;
};

struct r_core_t {
	u64 seek;
	u32 blocksize;
	u8 *block;
	u8 *oobi;
	int oobi_len;
	/* files */
	struct r_io_t io;
	struct r_core_file_t *file;
	struct list_head files;
	struct r_num_t num;
	struct r_lib_t lib;
	struct r_cmd_t cmd;
	struct r_lang_t lang;
	struct r_debug_t dbg;
	struct r_flag_t flags;
	struct r_config_t config;
	struct r_macro_t macro;
};

int r_core_init(struct r_core_t *core);
struct r_core_t *r_core_new();
int r_core_config_init(struct r_core_t *core);
int r_core_prompt(struct r_core_t *r);
int r_core_cmd(struct r_core_t *r, const char *cmd, int log);
int r_core_cmd0(void *user, const char *cmd);
int r_core_cmd_init(struct r_core_t *core);
char *r_core_cmd_str(struct r_core_t *core, const char *cmd);
int r_core_cmd_file(struct r_core_t *core, const char *file);
int r_core_seek(struct r_core_t *core, u64 addr);
int r_core_block_read(struct r_core_t *core, int next);
int r_core_block_size(struct r_core_t *core, u32 bsize);
int r_core_cmd_init(struct r_core_t *core);
int r_core_visual(struct r_core_t *core);

struct r_core_file_t *r_core_file_open(struct r_core_t *r, const char *file, int mode);

int r_core_write_at(struct r_core_t *core, u64 addr, const u8 *buf, int size);

#endif

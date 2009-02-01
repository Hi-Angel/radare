#include <r_types.h>
#include "list.h"

#define r_cmd_callback(x) int (*x)(void *data, const char *input)
#define r_cmd_nullcallback(x) int (*x)(void *data);

struct r_cmd_item_t {
	char cmd[64];
	char desc[128];
	r_cmd_callback(callback);
};

struct r_cmd_long_item_t {
	char cmd[64]; /* long command */
	int cmd_len;
	char cmd_short[32]; /* short command */
	char desc[128];
	struct list_head list;
};

struct r_cmd_t {
	void *data;
	r_cmd_nullcallback(nullcallback);
	struct list_head lcmds;
	struct r_cmd_item_t *cmds[255];
};

int r_cmd_init(struct r_cmd_t *cmd);
int r_cmd_set_data(struct r_cmd_t *cmd, void *data);
int r_cmd_add(struct r_cmd_t *cmd, const char *command, const char *desc, r_cmd_callback(callback));
int r_cmd_del(struct r_cmd_t *cmd, const char *command);
int r_cmd_call(struct r_cmd_t *cmd, const char *command);
char **r_cmd_args(struct r_cmd_t *cmd, int *argc);

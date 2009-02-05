/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_core.h"
#include "r_flags.h"
#include "r_hash.h"
#include "r_asm.h"

int r_core_write_op(struct r_core_t *core, const char *arg, char op);

static int cmd_print(void *data, const char *input);

static int cmd_iopipe(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '\0':
		r_lib_list(&core->lib);
		r_io_handle_list(&core->io);
		break;
	default:
		r_io_system(&core->io, core->file->fd, input+1);
		break;
	}
	return R_TRUE;
}

static int cmd_quit(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '?':
		fprintf(stderr,
		"Usage: q[!] [retvalue]\n"
		" q     ; quit program\n"
		" q!    ; force quit (no questions)\n"
		" q 1   ; quit with return value 1\n"
		" q a-b ; quit with return value a-b\n");
		break;
	case '\0':
	case ' ':
	case '!':
	default:
		exit(r_num_math(&core->num, input+1));
		break;
	}
	return 0;
}

static int cmd_interpret(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case ' ':
		/* interpret file */
		r_core_cmd_file(core, input+1);
		break;
	case '!':
		/* from command */
		fprintf(stderr, "TODO\n");
		break;
	case '(':
		fprintf(stderr, "macro call (%s)\n", input+1);
		r_macro_call(&core->macro, input+1);
		break;
	case '?':
		fprintf(stderr,
		"Usage: . [file] | [!command] | [(macro)]\n"
		" . foo.rs          ; interpret r script\n"
		" .!rabin -ri $FILE ; interpret output of command\n"
		" .(foo 1 2 3)      ; run macro 'foo' with args 1, 2, 3\n");
		break;
	}
	return 0;
}

static int cmd_seek(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;

	if (input[0]=='\0') {
		r_cons_printf("0x%llx\n", core->seek);
	} else {
		u64 off = r_num_math(&core->num, input+1);
		switch(input[0]) {
		case ' ':
			r_core_seek(core, off);
			break;
		case '+':
			r_core_seek(core, core->seek + off);
			break;
		case '-':
			r_core_seek(core, core->seek - off);
			break;
		case '?':
			fprintf(stderr,
			"Usage: s[+-] [addr]\n"
			" s 0x320   ; seek to this address\n"
			" s+ 512    ; seek 512 bytes forward\n"
			" s- 512    ; seek 512 bytes backward\n");
			break;
		}
	}
	return 0;
}

static int cmd_help(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	u64 n;

	switch(input[0]) {
	case ' ':
		n = r_num_math(&core->num, input+1);
		r_cons_printf("%lld 0x%llx\n", n,n);
		break;
	case '=':
		r_num_math(&core->num, input+1);
		break;
	case '+':
		if (input[1]) {
			if (core->num.value & U64_GT0)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '-':
		if (input[1]) {
			if (core->num.value & U64_LT0)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '!': // ??
		if (input[1]) {
			if (core->num.value != U64_MIN)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '?': // ??
		if (input[1]=='?') {
			fprintf(stderr,
			"Usage: ?[?[?]] expression\n"
			" ? eip-0x804800  ; calculate result for this math expr\n"
			" ?= eip-0x804800  ; same as above without user feedback\n"
			" ?? [cmd]    ; ? == 0  run command when math matches\n"
			" ?! [cmd]    ; ? != 0\n"
			" ?+ [cmd]    ; ? > 0\n"
			" ?- [cmd]    ; ? < 0\n"
			" ???             ; show this help\n");
		} else
		if (input[1]) {
			if (core->num.value == U64_MIN)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '\0':
	default:
		fprintf(stderr,
		"Usage:\n"
		" b [bsz]          ; get or change block size\n"
		" e [a[=b]]        ; list/get/set config evaluable vars\n"
		" f [name][sz][at] ; set flag at current address\n"
		" s [addr]         ; seek to address\n"
		" i [file]         ; get info about opened file\n"
		" p?[len]          ; print current block with format and length\n"
		" x [len]          ; alias for 'px' (print hexadecimal\n"
		" w[mode] [arg]    ; multiple write operations\n"
		" V                ; enter visual mode\n"
		" ? [expr]         ; evaluate math expression\n"
		" |[cmd]           ; run this command thru the io pipe (no args=list)\n"
		" #[algo] [len]    ; calculate hash checksum of current block\n"
		" q [ret]          ; quit r\n"
		"Append '?' to every char command to get detailed help\n"
		"");
		break;
	}
	return 0;
}

static int cmd_bsize(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '\0':
		r_cons_printf("0x%x\n", core->blocksize);
		break;
	default:
		//input = r_str_clean(input);
		r_core_block_size(core, r_num_math(NULL, input));
		break;
	}
	return 0;
}

static int cmd_hexdump(void *data, const char *input)
{
	return cmd_print(data, input-1);
}

static int cmd_info(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '?':
		break;
	case '*':
		break;
	default:
		r_cons_printf("URI: %s\n", core->file->uri);
		r_cons_printf("blocksize: 0x%x\n", core->blocksize);
	}
	return 0;
}

static int cmd_print(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	int l, len = core->blocksize;
	u32 tbs = core->blocksize;

	if (input[0] && input[1]) {
		l = (int) r_num_get(&core->num, input+2);
		if (l>0) len = l;
		if (l>tbs) {
			r_core_block_size(core, l);
			len = l;
		}
	}
	
	switch(input[0]) {
	case 'd':
		{
			/* XXX hardcoded */
			int ret, idx; 
			u8 *buf = core->block;
			struct r_asm_t a;// TODO: move to core.h
			r_asm_init(&a);
			r_asm_set_pc(&a, core->seek);
			
			for(idx=ret=0; idx < len; idx+=ret) {
				r_asm_set_pc(&a, a.pc + ret);
				ret = r_asm_disasm(&a, buf+idx, len-idx);
				r_cons_printf("0x%08llx  %14s  %s\n", core->seek+idx, a.buf_hex, a.buf_asm);
			}
		}
		break;
	case 'r':
		r_print_raw(core->block, len);
		break;
	case 'x':
        	r_print_hexdump(core->seek, core->block, len, 1, 78, !(input[1]=='-'));
		break;
	case '8':
		r_print_bytes(core->block, len, "%02x");
		break;
	default:
		fprintf(stderr, "Usage: p[8] [len]"
		" p8 [len]    8bit hexpair list of bytes\n"
		" px [len]    hexdump of N bytes\n"
		" pd [len]    disassemble N bytes\n"
		" pr [len]    print N raw bytes\n");
		break;
	}
	if (tbs != core->blocksize)
		r_core_block_size(core, tbs);
	return 0;
}

static int cmd_flag(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	int len = strlen(input)+1;
	char *ptr;
	char *str = alloca(len);
	//u64 seek = core->seek;
	//u64 size = core->blocksize;
	memcpy(str, input, len);
	ptr = strchr(str+1, ' ');
	if (ptr) {
		*ptr='\0';
		ptr = strchr(ptr+1, ' ');
		if (ptr) {
			*ptr='\0';
		}
		// TODO redefine seek and size here
	}
	switch(input[0]) {
	case '+':
		r_flag_set(&core->flags, input+1, core->seek, core->blocksize, 1);
		break;
	case ' ':
		r_flag_set(&core->flags, input+1, core->seek, core->blocksize, 0);
		break;
	case '-':
		r_flag_unset(&core->flags, input+1);
		break;
	case 's':
		/* spaces */
		break;
	case '*':
		r_flag_list(&core->flags, 1);
		break;
	case '\0':
		r_flag_list(&core->flags, 0);
		break;
	case '?':
		fprintf(stderr, "Usage: f[ ] [flagname]\n"
		" f name 12 @ 33   ; set flag 'name' with size 12 at 33\n"
		" f name 12 33     ; same as above\n"
		" f+name 12 @ 33   ; like above but creates new one if doesnt exist\n"
		" f-name           ; remove flag 'name'\n"
		" f                ; list flags\n"
		" f*               ; list flags in r commands\n");
		break;
	}
	return 0;
}

/* TODO: simplify using r_write */
static int cmd_write(void *data, const char *input)
{
	int i, len = strlen(input);
	char *tmp, *str = alloca(len)+1;
	struct r_core_t *core = (struct r_core_t *)data;
	memcpy(str, input+1, len);
	switch(input[0]) {
	case ' ':
		// write strifng
		len = r_str_escape(str);
		r_io_lseek(&core->io, core->file->fd, core->seek, R_IO_SEEK_SET);
		r_io_write(&core->io, core->file->fd, str, len);
		r_core_block_read(core, 0);
		break;
	case 't':
	case 'f':
		fprintf(stderr, "TODO: wf, wt\n");
		break;
	case 'w':
		str = str+1;
		len = len-1;
		len *= 2;
		tmp = alloca(len);
		for(i=0;i<len;i++) {
			if (i%2) tmp[i] = 0;
			else tmp[i] = str[i>>1];
		}
		str = tmp;

		// write strifng
		r_io_lseek(&core->io, core->file->fd, core->seek, R_IO_SEEK_SET);
		r_io_write(&core->io, core->file->fd, str, len);
		r_core_block_read(core, 0);
		break;
	case 'x':
		{
		int len = strlen(input);
		char *buf = alloca(len);
		len = r_hex_str2bin(input+1, buf);
		r_io_lseek(&core->io, core->file->fd, core->seek, R_IO_SEEK_SET);
		r_io_write(&core->io, core->file->fd, buf, len);
		r_core_block_read(core, 0);
		}
		// write hexpairs
		break;
	case 'b':
		{
		int len = strlen(input);
		char *buf = alloca(len);
		len = r_hex_str2bin(input+1, buf);
		r_mem_copyloop(core->block, buf, core->blocksize, len);
		r_io_lseek(&core->io, core->file->fd, core->seek, R_IO_SEEK_SET);
		r_io_write(&core->io, core->file->fd, core->block, core->blocksize);
		r_core_block_read(core, 0);
		}
		break;
	case 'm':
		{
		int len = strlen(input+1);
		len = r_hex_str2bin(input+1, str);
		switch(input[1]) {
		case '\0':
			fprintf(stderr, "Current write mask: TODO\n");
			// TODO
			break;
		case '?':
			break;
		case '-':
			r_io_set_write_mask(&core->io, -1, 0, 0);
			fprintf(stderr, "Write mask disabled\n");
			break;
		case ' ':
			if (len == 0) {
				fprintf(stderr, "Invalid string\n");
			} else {
				r_io_set_write_mask(&core->io, core->file->fd, str, len);
				fprintf(stderr, "Write mask set to '");
				for (i=0;i<len;i++)
					fprintf(stderr, "%02x", str[i]);
				fprintf(stderr, "'\n");
			}
			break;
		}
		}
		break;
	case 'v':
		{
		u64 off = r_num_math(&core->num, input+1);
		r_io_lseek(&core->io, core->file->fd, core->seek, R_IO_SEEK_SET);
		if (off&U64_32U) {
			/* 8 byte addr */
			u64 addr8;
			memcpy((u8*)&addr8, (u8*)&off, 8); // XXX needs endian here
		//	endian_memcpy((u8*)&addr8, (u8*)&off, 8);
			r_io_write(&core->io, core->file->fd, (const u8 *)&addr8, 8);
		} else {
			/* 4 byte addr */
			u32 addr4, addr4_ = (u32)off;
			//drop_endian((u8*)&addr4_, (u8*)&addr4, 4); /* addr4_ = addr4 */
			//endian_memcpy((u8*)&addr4, (u8*)&addr4_, 4); /* addr4 = addr4_ */
			memcpy((u8*)&addr4, (u8*)&addr4_, 4); // XXX needs endian here too
			r_io_write(&core->io, core->file->fd, (const u8 *)&addr4, 4);
		}
		r_core_block_read(core, 0);
		}
		break;
	case 'o':
                switch(input[1]) {
                case 'a':
                case 's':
                case 'A':
                case 'x':
                case 'r':
                case 'l':
                case 'm':
                case 'd':
                case 'o':
                        if (input[2]!=' ') {
                                fprintf(stderr, "Usage: 'wo%c 00 11 22'\n", input[1]);
                                return 0;
                        }
                case '2':
                case '4':
                        r_core_write_op(core, input+3, input[1]);
                        break;
                case '\0':
                case '?':
                default:
                        fprintf(stderr, 
                        "Usage: wo[xrlasmd] [hexpairs]\n"
                        "Example: wox 90    ; xor cur block with 90\n"
                        "Example: woa 02 03 ; add 2, 3 to all bytes of cur block\n"
                        "Supported operations:\n"
                        "  woa  addition            +=\n"
                        "  wos  substraction        -=\n"
                        "  wom  multiply            *=\n"
                        "  wod  divide              /=\n"
                        "  wox  xor                 ^=\n"
                        "  woo  or                  |=\n"
                        "  woA  and                 &=\n"
                        "  wor  shift right         >>=\n"
                        "  wol  shift left          <<=\n"
                        "  wo2  2 byte endian swap  2=\n"
                        "  wo4  4 byte endian swap  4=\n"
                        );
                        break;
                }
                break;
	default:
	case '?':
		if (core->oobi) {
			fprintf(stderr, "Writing oobi buffer!\n");
			r_io_write(&core->io, core->file->fd, core->oobi, core->oobi_len);
		} else
			r_cons_printf("Usage: w[x] [str] [<file] [<<EOF] [@addr]\n"
			" w foobar   ; write string 'foobar'\n"
			" ww foobar  ; write wide string 'f\\x00o\\x00o\\x00b\\x00a\\x00r\\x00'\n"
			" wb 010203  ; fill current block with cyclic hexpairs\n"
			" wx 9090    ; write two intel nops\n"
			" wv eip+34  ; write 32-64 bit value\n"
			" wo[] hex   ; write in block with operation. 'wo?' fmi\n"
			" wm f0ff    ; cyclic write mask\n");
		break;
	}
	return 0;
}

static int cmd_search(void *data, const char *input)
{
	//struct r_core_t *core = (struct r_core_t *)data;
	/* TODO */
	return R_TRUE;
}

static int cmd_eval(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '\0':
		r_config_list(&core->config, NULL, 0);
		break;
	case '-':
		r_config_init(&core->config);
		break;
	case '*':
		r_config_list(&core->config, NULL, 1);
		break;
	case '?':
		r_cons_printf(
		"Usage: e[?] [var[=value]]\n"
		"  e     ; list config vars\n"
		"  e-    ; reset config vars\n"
		"  e*    ; dump config vars in r commands\n"
		"  e a   ; get value of var 'a'\n"
		"  e a=b ; set var 'a' the 'b' value\n");
		//r_cmd_help(&core->cmd, "e");
		break;
	default:
		r_config_eval(&core->config, input);
	}
	return 0;
}

static int cmd_hash(void *data, const char *input)
{
	char algo[32];
	struct r_core_t *core = (struct r_core_t *)data;
	u32 len = core->blocksize;
	const char *ptr;

	if (input[0]=='!') {
#if 0
		#!lua < file
		#!lua <<EOF
		#!lua
		#!lua foo bar
#endif
		r_lang_run(&core->lang, core->oobi, core->oobi_len);
		r_cons_printf("TODO\n");
		return R_TRUE;
	}

	ptr = strchr(input, ' ');
	sscanf(input, "%31s", algo);
	if (ptr != NULL)
		len = r_num_math(&core->num, ptr+1);

	if (!r_str_ccmp(input, "crc32", ' ')) {
		r_cons_printf("%04x\n", r_hash_crc32(core->block, len));
	} else
	if (!r_str_ccmp(input, "crc16", ' ')) {
		r_cons_printf("%02x\n", r_hash_crc16(0, core->block, len));
	} else {
		r_cons_printf(
		"Usage: #algo <size> @ addr\n"
		" #crc32               ; calculate crc32 of current block\n"
		" #crc32 < /etc/fstab  ; calculate crc32 of this file\n"
		" #md5 128K @ edi      ; calculate md5 of 128K from 'edi'\n"
		"Usage #!interpreter [<args>] [<file] [<<eof]\n"
		" #!                   ; list all available interpreters\n"
		" #!python             ; run python commandline\n"
		" #!python < foo.py    ; run foo.py python script\n"
		" #!python <<EOF       ; get python code until 'EOF' mark\n"
		" #!python arg0 a1 <<q ; set arg0 and arg1 and read until 'q'\n");
	}

	return 0;
}

static int cmd_visual(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	r_core_visual(core);
	return 0;
}

static int cmd_system(void *data, const char *input)
{
	//struct r_core_t *core = (struct r_core_t *)data;
	// slurped from teh old radare_system
#if __FreeBSD__
	/* freebsd system() is broken */
	int fds[2];
	int st,pid;
	char *argv[] ={ "/bin/sh", "-c", input, NULL};
	pipe(fds);
	/* not working ?? */
	//pid = rfork(RFPROC|RFCFDG);
	pid = vfork();
	if (pid == 0) {
		dup2(1, fds[1]);
		execv(argv[0], argv);
		_exit(127); /* error */
	} else {
		dup2(1, fds[0]);
		waitpid(pid, &st, 0);
	}
	return WEXITSTATUS(st);
#else
	return system(input);
#endif
}

static int cmd_io_system(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	return r_io_system(&core->io, core->file->fd, input);
}

static int cmd_macro(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case ')':
		r_macro_break(&core->macro, input+1);
		break;
	case '-':
		r_macro_rm(&core->macro, input+1);
		break;
	case '\0':
		r_macro_list(&core->macro);
		break;
	case '?':
		eprintf(
		"Usage: (foo\\n..cmds..\\n)\n"
		" Record macros grouping commands\n"
		" (foo args\\n ..)  ; define a macro\n"
		" (-foo)            ; remove a macro\n"
		" .(foo)            ; to call it\n"
		" ()                ; break inside macro\n"
		"Argument support:\n"
		" (foo x y\\n$1 @ $2) ; define fun with args\n"
		" .(foo 128 0x804800) ; call it with args\n"
		"Iterations:\n"
		" .(foo\\n() $@)      ; define iterator returning iter index\n"
		" x @@ .(foo)         ; iterate over them\n"
		);
		break;
	default:
		r_macro_add(&core->macro, input);
		break;
	}
	return 0;
}

static int r_core_cmd_subst(struct r_core_t *core, char *cmd, int *rs, int *rfd, int *times)
{
	char *ptr, *ptr2, *str;
	int i, len = strlen(cmd);

	len = atoi(cmd);
	if (len>0) {
		for(i=0;cmd[i]>='0'&&cmd[i]<='9'; i++);
		if (i>0) strcpy(cmd, cmd+i);
		*times = len;
	}
	if (cmd[0]=='\0')
		return;

	/* quoted / raw command */
	if (cmd[0]=='"') {
		if (cmd[len]!='"') {
			fprintf(stderr, "parse: Missing ending '\"': '%s'\n", cmd);
			return 0;
		}
		cmd[len]='\0';
		strcpy(cmd, cmd+1);
		return 0;
	}
	ptr = strchr(cmd, ';');
	if (ptr)
		ptr[0]='\0';
	ptr = strchr(cmd+1, '|');
	if (ptr) {
		ptr[0] = '\0';
		fprintf(stderr, "System pipes not yet supported.\n");
	}

	/* Out Of Band Input */
	free(core->oobi);
	core->oobi = NULL;
	ptr = strchr(cmd, '<');
	if (ptr) {
		ptr[0] = '\0';
		if (ptr[1]=='<') {
			/* this is a bit mess */
			char *oprompt = r_line_prompt;
			oprompt = ">";
			for(str=ptr+2;str[0]== ' ';str=str+1);
			fprintf(stderr, "==> Reading from stdin until '%s'\n", str);
			free(core->oobi);
			core->oobi = malloc(1);
			core->oobi[0] = '\0';
			core->oobi_len = 0;
			for (;;) {
				char buf[1024];
				int ret;
				printf("> "); fflush(stdout);
				fgets(buf, 1023, stdin);
				if (feof(stdin))
					break;
				buf[strlen(buf)-1]='\0';
				ret = strlen(buf);
				core->oobi_len+=ret;
				core->oobi = realloc(core->oobi, core->oobi_len+1);
				if (!strcmp(buf, str))
					break;
				strcat(core->oobi, buf);
			}
			r_line_prompt = oprompt;
		} else {
			for(str=ptr+1;str[0]== ' ';str=str+1);
			printf("SLURPING FILE '%s'\n", str);
			core->oobi = r_file_slurp(str, &core->oobi_len);
			if (core->oobi == NULL) {
				printf("Cannot open file\n");
			}
		}
	}
	/* Pipe console to file */
	ptr = strchr(cmd, '>');
	if (ptr) {
		ptr[0] = '\0';
		for(str=ptr+1;str[0]== ' ';str=str+1);
		*rfd = r_cons_pipe_open(str, ptr[1]=='>');
	}
	ptr = strchr(cmd, '@');
	if (ptr) {
		ptr[0]='\0';
		*rs = 1;
		if (ptr[1]=='@') {
			fprintf(stderr, "TODO: foreach @ loop\n");
		}
		r_core_seek(core, r_num_math(&core->num, ptr+1));
	}
	ptr = strchr(cmd, '~');
	if (ptr) {
		ptr[0]='\0';
		r_cons_grep(ptr+1);
	} else r_cons_grep(NULL);
	while(((ptr = strchr(cmd, '`')))) {
		ptr2 = strchr(ptr+1, '`');
		if (ptr2==NULL) {
			fprintf(stderr, "parse: Missing '`' in expression (%s).\n", ptr+1);
			return 0;
		}
		ptr2[0]='\0';
		str = r_core_cmd_str(core, ptr+1);
		for(i=0;str[i];i++) if (str[i]=='\n') str[i]=' ';
		r_str_inject(ptr, ptr2+1, str, 1024);
		free(str);
	}
	return 0;
}

int r_core_cmd(struct r_core_t *core, const char *command, int log)
{
	int i, len;
	char *cmd;
	int ret = -1;
	int times = 1;
	int newfd = 1;
	
	u64 tmpseek = core->seek;
	int restoreseek = 0;

	if (command == NULL )
		return 0;
	while(command[0]==' ') // TODO: handle tabs to with iswhitespace()
		command = command+1;

	len = strlen(command)+1;
	cmd = alloca(len)+1024;
	memcpy(cmd, command, len);

	ret = r_core_cmd_subst(core, cmd, &restoreseek, &newfd, &times);
	if (ret == -1) {
		fprintf(stderr, "Invalid conversion: '%s'\n", command);
		ret = -1;
	} else {
		for(i=0;i<times;i++) {
			ret = r_cmd_call(&core->cmd, cmd);
			if (ret == -1) // stop on error?
				break;
		}
		if (ret == -1){
			if (cmd[0])
				fprintf(stderr, "Invalid command: '%s'\n", command);
			ret = 1;
		} else
		if (log) r_line_hist_add(command);
		if (restoreseek)
			r_core_seek(core, tmpseek);
	}

	if (newfd != 1) {
		r_cons_flush();
		r_cons_pipe_close(newfd);
	}

	free (core->oobi);
	core->oobi = NULL;
	core->oobi_len = 0;

	return ret;
}

int r_core_cmd_file(struct r_core_t *core, const char *file)
{
	char buf[1024];
	FILE *fd = fopen(file, "r");
	if (fd == NULL)
		return -1;
	while (!feof(fd)) {
		if (fgets(buf, 1023, fd) != NULL) {
			buf[strlen(buf)-1]='\0';
			if (r_core_cmd(core, buf, 0) == -1) {
				fprintf(stderr, "Error running command '%s'\n", buf);
				break;
			}
		}
	}
	fclose(fd);
	return 0;
}

static int cmd_debug(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case 's':
		fprintf(stderr, "step\n");
		break;
	case 'b':
		fprintf(stderr, "breakpoint\n");
		break;
	case 'c':
		fprintf(stderr, "continue\n");
		r_debug_continue(&core->dbg);
		break;
	case 'r':
		fprintf(stderr, "show registers\n");
		break;
	case 'h':
		r_debug_handle_set(&core->dbg, input+1);
		break;
	default:
		r_cons_printf("Usage: d[sbc] [arg]\n"
		" dh [handler] ; list or set debugger handler\n"
		" ds           ; perform one step\n"
		" ds 3         ; perform 3 steps\n"
		" do 3         ; perform 3 steps overs\n"
		" dc           ; continue execution\n"
		" dr           ; show registers\n"
		" dr*          ; show registers in radare commands\n"
		" dr eax       ; show value of eax register\n"
		" dr eax = 33  ; set register value. eax = 33\n"
		" db           ; list breakpoints\n"
		" db sym.main  ; set breakpoint\n"
		" db -sym.main ; drop breakpoint\n"
		" dm           ; show memory maps\n"
		" dm 4096      ; allocate 4KB in child process\n"
		" dm rw- esp 9K; set 9KB of the stack as read+write (no exec)\n");
		break;
	}
	return 0;
}

int r_core_cmd0(void *user, const char *cmd)
{
	return r_core_cmd((struct r_core_t *)user, cmd, 0);
}

char *r_core_cmd_str(struct r_core_t *core, const char *cmd)
{
	char *retstr;
	r_cons_reset();
	if (r_core_cmd(core, cmd, 0) == -1) {
		fprintf(stderr, "Invalid command: %s\n", cmd);
		retstr = strdup("");
	} else {
		const char *static_str = r_cons_get_buffer();
		if (retstr==NULL)
			retstr = strdup("");
		else retstr = strdup(static_str);
		r_cons_reset();
	}
	return retstr;
}

int r_core_cmd_init(struct r_core_t *core)
{
	r_cmd_init(&core->cmd);
	r_cmd_set_data(&core->cmd, core);
	r_cmd_add(&core->cmd, "x",     "alias for px", &cmd_hexdump);
	r_cmd_add(&core->cmd, "flag",  "get/set flags", &cmd_flag);
	r_cmd_add(&core->cmd, "debug", "debugger operations", &cmd_debug);
	r_cmd_add(&core->cmd, "info",  "get file info", &cmd_info);
	r_cmd_add(&core->cmd, "seek",  "seek to an offset", &cmd_seek);
	r_cmd_add(&core->cmd, "bsize", "change block size", &cmd_bsize);
	r_cmd_add(&core->cmd, "eval",  "evaluate configuration variable", &cmd_eval);
	r_cmd_add(&core->cmd, "print", "print current block", &cmd_print);
	r_cmd_add(&core->cmd, "write", "write bytes", &cmd_write);
	r_cmd_add(&core->cmd, "Visual","enter visual mode", &cmd_visual);
	r_cmd_add(&core->cmd, "!",     "run system command", &cmd_system);
	r_cmd_add(&core->cmd, "|",     "run io system command", &cmd_io_system);
	r_cmd_add(&core->cmd, "#",     "calculate hash", &cmd_hash);
	r_cmd_add(&core->cmd, "?",     "help message", &cmd_help);
	r_cmd_add(&core->cmd, ".",     "interpret", &cmd_interpret);
	r_cmd_add(&core->cmd, "/",     "search kw, pattern aes", &cmd_search);
	r_cmd_add(&core->cmd, "(",     "macro", &cmd_macro);
	r_cmd_add(&core->cmd, "|",     "io pipe", &cmd_iopipe);
	r_cmd_add(&core->cmd, "quit",  "exit program session", &cmd_quit);

	return 0;
}

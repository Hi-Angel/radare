/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * libps2fd is part of the radare project
 *
 * libps2fd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libps2fd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libps2fd; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "../../config.h"
#include "../../main.h"
#include "../../code.h"
#include "../libps2fd.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int pids_cmdline(int pid, char *cmdline)
{
	int fd;
	sprintf(cmdline, "/proc/%d/cmdline", pid);
	fd = open(cmdline, O_RDONLY);
	cmdline[0] = '\0';
	if (fd != -1) {
		read(fd, cmdline, 1024);
		cmdline[1024] = '\0';
		close(fd);
	}

	return 0;
}

/* ptrace attaches to all sons of a pid */
int pids_ptrace_all(int pid)
{
// XXX lot of c&p shit here lol!1
	int p;
	int n = 0;
	int mola;
	char buf[128];
	int tmp;
	char tmp2[1024];
	char tmp3[8];
	struct dirent *file;
	FILE *fd;
	DIR *dh = opendir("/proc/");

	if (dh == NULL)
		return 0;

	while((file=(struct dirent *)readdir(dh)) ) {
		p = atoi(file->d_name);
		if (p) {
			sprintf(buf,"/proc/%s/stat", file->d_name);
			fd = fopen(buf, "r");
			if (fd) {
				mola = 0;
				fscanf(fd,"%d %s %s %d",
					&tmp, tmp2, tmp3, &mola);
				if (mola == pid) {
					//pids_cmdline(p, tmp2);
					//for(i=0; i<recursive*2;i++)
					//	printf(" ");
					//printf(" `- %d : %s\n", p, tmp2);
					/* do not change -attach- stuff here, only make the kernel more conscient about us */
					ptrace(PTRACE_ATTACH, p, 0, 0);
					n++;
				}
			}
			fclose(fd);
		}
	}
	return n;
}

int pids_sons_of_r(int pid, int recursive, int limit, int rad)
{
	int p;
	int n = 0;
	int mola;
	char buf[128];
	int tmp;
	char tmp2[1024];
	char tmp3[8];
	struct dirent *file;
	FILE *fd;
	DIR *dh = opendir("/proc/");

	if (pid == 0 || dh == NULL)
		return 0;

	while((file=(struct dirent *)readdir(dh)) ) {
		p = atoi(file->d_name);
		if (p) {
			sprintf(buf,"/proc/%s/stat", file->d_name);
			fd = fopen(buf, "r");
			if (fd) {
				mola = 0;
				fscanf(fd,"%d %s %s %d",
					&tmp, tmp2, tmp3, &mola);
				if (mola == pid) {
					pids_cmdline(p, tmp2);
					//for(i=0; i<recursive*2;i++)
					//	printf(" ");
					if (rad)
						cons_printf("%d ", p);
					else cons_printf(" `- %d : %s (%s)\n",
						p, tmp2,
						(tmp3[0]=='S')?"sleeping":
						(tmp3[0]=='T')?"stopped":"running");
					n++;
					if (recursive<limit)
						n+=pids_sons_of_r(p, recursive+1, limit, rad);
				}
			}
			fclose(fd);
		}
	}
	return n;
}

int pids_list()
{
	int i,n;
	char cmdline[1025];

	// TODO: use ptrace to get cmdline from esp like tuxi does
	for(n=0,i=2;i<999999;i++) {
		switch( kill(i, 0) ) {
		case 0:
			pids_cmdline(i, cmdline);
			printf("%d : %s\n", i, cmdline);
			n++;
			break;
//		case -1:
//			if (errno == EPERM)
//				printf("%d [not owned]\n", i);
//			break;
		}
	}
	return n;
}

static const char *debug_unix_pid_running  = "running";
static const char *debug_unix_pid_stopped  = "stopped";
static const char *debug_unix_pid_sleeping = "sleeping";
static const char *debug_unix_pid_unknown  = "unknown";
static const char *debug_unix_pid_zombie   = "zombie";

const char *debug_unix_pid_status(int pid)
{
	const char *str = debug_unix_pid_unknown;
	char buf[129];
	FILE *fd;

	sprintf(buf, "/proc/%d/status", pid);
	fd = fopen(buf, "r");
	if (fd) {
		while(1) {
			buf[0]='\0';
			fgets(buf, 128, fd);
			if (buf[0]=='\0')
				break;
			if (strstr(buf, "State:")) {
				if (strstr(buf, "sleep"))
					str = debug_unix_pid_sleeping;
				else
				if (strstr(buf, "running"))
					str = debug_unix_pid_running;
				else
				if (strstr(buf, "stop"))
					str = debug_unix_pid_stopped;
				else
				if (strstr(buf, "zombie")) // XXX ?
					str = debug_unix_pid_zombie;
			}
		}
		fclose(fd);
	}
		
	return str;
}

int debug_pstree(const char *input)
{
	int foo, tid = 0;
	int rad = 0;
	if (input) { 
		tid = atoi(input);
		switch(input[-1]) {
		case '*':
			rad = 1;
			break;
		case '?':
			eprintf("Usage: !pid[*] [pid]\n");
			eprintf(" !pid    : list all processes in a human way\n");
			eprintf(" !pid*   : list all processes as a pid list oneliner\n");
			eprintf(" !pid 29 : select the process 29 (change thread)\n");
			break;
		}
	}

	if (tid != 0) {
		if (strstr(input, "stop")) {
			kill(tid, SIGSTOP);
			eprintf("stop for %d\n", tid);
		} else
		if (strstr(input, "cont")) {
			kill(tid, SIGCONT);
			eprintf("running for %d\n", tid);
		} else
		if (strstr(input, "segv")) {
			kill(tid, SIGSEGV);
			eprintf("segv for %d\n", tid);
		} else
		if (strstr(input, "kill")) {
			kill(tid, SIGKILL);
			eprintf("Current selected thread id (pid): %d\n", ps.tid);
		} else
		if (input && strstr(input, "?")) {
			cons_printf("Usage: !pid [pid] [stop|cont|segv|kill]\n");
		} else {
			ps.tid = tid;
			eprintf("Current selected thread id (pid): %d\n", ps.tid);
		}
		// XXX check if exists or so
		return 0;
	}

	foo = ps.pid;
	if (rad) {
		cons_printf ("%d ", ps.tid);
		pids_sons_of_r (foo, 0, 0, 1);
		if (ps.pid != ps.tid) // sure?
			cons_printf ("%d", ps.pid);
		cons_newline ();
	} else {
		eprintf(" tid : %d\n", ps.tid);
		eprintf(" pid : %d 0x%08llx (%s)\n", foo, arch_pc(foo), debug_unix_pid_status(foo));
		pids_sons_of_r(foo, 0, 0, 0);
		if (ps.pid != ps.tid) {
			foo = ps.tid;
			eprintf(" pid : %d 0x%08llx (%s)\n", foo, arch_pc(foo), debug_unix_pid_status(foo));
			pids_sons_of_r(foo, 0, 0, 0);
		}
	}

	return 0;
}

